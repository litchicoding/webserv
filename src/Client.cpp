#include "webserv.hpp"

/*************************************************************************************************/
/* Constructor and Deconstructor *****************************************************************/

Client::Client(int listen_fd, int epoll_fd, Listen* listen)
: last_activity(0), state(READ_HEADERS), _listen_fd(listen_fd), _server(NULL), _config(NULL), _keep_alive(true)
{
	_listen = listen;
	cout << GREEN << "***  Client Connection  ***" << RESET << endl;
	socklen_t	client_addr_len = sizeof(_client_addr);
	_client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&_client_addr), &client_addr_len);
	if (_client_fd == INVALID) {
		perror("accept");
		return ;
	}
	add_fd_to_epoll(epoll_fd, _client_fd);
	_epoll_fd = epoll_fd;
	_cgi.is_running = false;
}

Client::~Client()
{
	cout << GREEN << "***  Client Deconstruction  ***" << RESET << endl << endl;
	if (_client_fd >= 0)
		close(_client_fd);
}

/*************************************************************************************************/
/* Request Parsing *******************************************************************************/

void	Client::sendResponse()
{

	_writeBuffer = _request.response.res;
	ssize_t bytes_sent = send(_client_fd, _writeBuffer.c_str(), _writeBuffer.size(), 0);
	if (bytes_sent < 0)
		return; // erreur à gérer si nécessaire
	_writeBuffer = _writeBuffer.substr(bytes_sent);

	size_t pos = _request.response.res.find("\n");
	cout << CYAN << "   - RESPONSE TO REQUEST [socket:" << _client_fd << "] : " << RESET;
	cout << _request.response.res.substr(0, pos) << endl;
	
	if (!_writeBuffer.empty())
	{
		// on active EPOLLOUT uniquement si tout n'a pas été envoyé
		epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT; // IN toujours actif
		ev.data.fd = _client_fd;
		epoll_ctl(g_global_instance->getEpollFd(), EPOLL_CTL_MOD, _client_fd, &ev);
    }
}

int	Client::readData()
{
	char	buffer[8192];
	ssize_t	bytes_read;

	memset(buffer, 0, sizeof(buffer));
	bytes_read = recv(_client_fd, buffer, sizeof(buffer), 0);
	if (bytes_read < 0) {
		// cout << RED "Error: readData(): while reading request." RESET << endl;
		return (ERROR);
	}
	if (bytes_read == 0)
		return (OK);
	time(&last_activity);
	_buffer.append(buffer, bytes_read);
	return (processBuffer());
}

int	Client::processRequest()
{
	string	method = _request.getMethod();
	setConfig(_request.getURI());
	if (_request.code == 0 && _config == NULL)
		_request.code = 500;
	isRedirectionNeeded();
	if (_request.code <= 0 && isRequestWellFormedOptimized() == OK) {
		cout << endl << "     Method:[\e[0m" << method << "\e[34m] URI:[\e[0m";
		cout << _request.getURI() << "\e[34m]";
		if (_config)
			cout << "FullPath:[\e[0m" << _config->full_path << "\e[34m]\e[0m" << endl;
		else
			cout << RESET << endl;
		if (method == "GET")
			_request.code = handleGet();
		else if (method == "POST")
			_request.code = handlePost();
		else if (method == "DELETE")
			_request.code = handleDelete();
		else
			_request.code = 501;
	}	
	if (isCgi() && _cgi.is_running) // ne pas appeler buildresponse() maintenant, cgi pas termine
		return OK;
	buildResponse(_request.code);
	return (OK);
}

void	Client::resetRequest()
{
	state = READ_HEADERS;
	_config = NULL;
	_request.resetRequest();
	_buffer.clear();
}

int Client::processBuffer()
{
	if (state == READ_HEADERS) {
		size_t pos = _buffer.find("\r\n\r\n");
		if (pos != string::npos) {
			string headers = _buffer.substr(0, pos);
			if (_request.parsingHeaders(headers) != OK)
				return (ERROR);
			_buffer = _buffer.substr(pos + 4);
			state = READ_BODY;
		}
		else {
			state = READ_END;
			_request.code = 400;
			return (OK);
		}
	}
	if (state == READ_BODY) {
		if (_request.isChunked() == true) {
			if (!_buffer.empty()) {
				if (parseChunked(_buffer) == static_cast<size_t>(ERROR)) {
					state = READ_END;
					return (ERROR);
				}
			}
		}
		else if (!_buffer.empty()) {
			size_t remaining_len = _request.getExpectedBodyLen() - _request.getBodyLen();
			size_t to_copy = min(_buffer.length(), remaining_len);
			_request.appendBodyData(_buffer.c_str(), to_copy);
			_buffer = _buffer.substr(to_copy);
			if (_request.getBodyLen() >= _request.getExpectedBodyLen())
				state = READ_END;
		}
		else
			state = READ_END;
	}
	return (OK);
}

int	Client::isRequestWellFormedOptimized() {

	string	URI, clean_URI, method;
	
	URI = _request.getURI();
	clean_URI = stripQueryString(URI);

	// VALIDATION DE L'URI
	if (URI.empty() || URI[0] != '/' || URI_Not_Printable(clean_URI) || URI.find("%00") != string::npos) {
		_request.code = 400;
		return (ERROR);
	}
	if (URI.find("..") != string::npos) {
		_request.code = 403;
		return (ERROR);
	}
	if (URI.size() > 2048) {
		_request.code = 414;
		return (ERROR);
	}
	// VALIDATION DE LA VERSION
	if (_request.getVersion() != "HTTP/1.1") {
		_request.code = 505;
		return (ERROR);
	}
	// VALIDATION ROOT LOCATION AND DIRECTIVES
	if (!_config->redirection.empty()) {
		_request.code = 301;
		return (ERROR);
	}
	if (_config->client_max_body_size < _request.getBodyLen()) {
		_request.code = 413;
		return (ERROR);
	}
	method = _request.getMethod();
	if (method != "POST" && method != "DELETE" && method != "GET") {
		_request.code = 501;
		return (ERROR);
	}
	if (find(_config->methods.begin(), _config->methods.end(), method) == _config->methods.end()) {
		_request.code = 405;
		return (ERROR);
	}
	if (isRequestWellChunked(_request.getHeaders()) != OK)
		return (ERROR);
	return (OK);
}

int	Client::isRequestWellChunked(const map<string, string> &headers)
{
	map<string, string>::const_iterator transfer;
	map<string, string>::const_iterator content_len;
	map<string, string>::const_iterator hostIt;
	int									code = 0;

	transfer = headers.find("Transfer-Encoding");
	content_len = headers.find("Content-Length");
	hostIt = headers.find("Host");
	if (hostIt == headers.end())
		code = 400;
	else if (transfer != headers.end() && transfer->second != "chunked") // can be gzip etc CHECK RFC
		code = 501;
	else if (transfer != headers.end() && content_len != headers.end()) // both present
		code = 400;	
	else if (transfer != headers.end())  // just transfer-encoding
		return (OK);
	if (content_len == headers.end() && _request.getMethod() == "POST")
		code = 411;
	else if (content_len != headers.end()) {
		if (content_len->second.empty() || content_len->second.length() > 19)
			code = 400;
		for (size_t i = 0; i < content_len->second.length(); i++)
		{
			if (content_len->second[i] < '0' && content_len->second[i] > '9')
				code = 400;
		}
	}
	if (code != 0) {
		_request.code = code;
		return (ERROR);
	}
	return (OK);
}

void	Client::isRedirectionNeeded()
{
	if (!_config || _config->redirection.empty())
		return ;
	map<int, string>::iterator redir = _config->redirection.begin();
	int		code = redir->first;
	string	data = redir->second;
	_request.code = code;
	if (code >= 300 && code <= 308)
		_request.setRedirectURI(data);
	else
		_request.response.body = data;
}

string Client::getMIME(string &URI)
{
	string contentType;
	if (URI.find(".html") != string::npos)
		contentType = "text/html";
	else if (URI.find(".jpg") != string::npos || URI.find(".jpeg") != string::npos)
		contentType = "image/jpeg";
	else if (URI.find(".png") != string::npos)
		contentType = "image/png";
	else if (URI.find(".ttf") != string::npos)
		contentType = "font/ttf";
	else if (URI.find(".js") != string::npos)
		contentType = "text/javascript";
	else if (URI.find(".gif") != string::npos)
		contentType = "image/gif";
	else if (URI.find(".otf") != string::npos)
		contentType = "font/otf";
	else if (URI.find(".svg") != string::npos)
		contentType = "image/svg+xml";
	else if (URI.find(".css") != string::npos)
		contentType = "text/css";
	else if (URI.find(".ico") != string::npos)
		contentType = "image/x-icon";
	else
		contentType = "text/plain";
	return contentType;
}

bool	Client::URI_Not_Printable(string& URI)
{
	for (size_t i = 0; i < URI.length(); i++)
	{
		char c = URI[i];
		if (!(c == 95 ||
			(c >= 45 && c <= 57) ||
			(c >= 64 && c <= 90) ||
			(c >= 97 && c <= 122)))
		{
			if (c == '%')
			{
				if (i+2 >= URI.size() || !isxdigit(URI[i+1]) || !isxdigit(URI[i+2]))
					return true;
				i += 2;
				continue;
			}
			return true;
		}
	}
	return false;
}

string Client::urlDecode(const string &str)
{
	string result;
	size_t i = 0;

	while (i < str.length()) {
		if (str[i] == '%' && i + 2 < str.length() &&
			isxdigit(str[i+1]) && isxdigit(str[i+2])) {
			string hex = str.substr(i + 1, 2);
			char decodedChar = static_cast<char>(strtol(hex.c_str(), 0, 16));
			result += decodedChar;
			i += 3;
		} else if (str[i] == '+') {
			result += ' ';
			i++;
		} else {
			result += str[i++];
		}
	}
	return result;
}

size_t Client::parseChunked(std::string &buffer)
{
    size_t total_processed = 0;

    while (true)
    {
        size_t pos = total_processed;
        // Chercher la fin du chunk
        size_t endline = buffer.find("\r\n", pos);
        if (endline == std::string::npos)
            break;
        // Converti la taille hexadécimale
        std::string hexsize = buffer.substr(pos, endline - pos);
        hexsize.erase(0, hexsize.find_first_not_of(" \t"));
        hexsize.erase(hexsize.find_last_not_of(" \t") + 1);
        std::istringstream iss(hexsize);
        size_t chunk_size = 0;
        iss >> std::hex >> chunk_size;
        if (iss.fail()) {
            _request.code = 400;
            return ERROR;
        }
        pos = endline + 2;
        // 3. Vérifier si on a reçu tout le chunk + son CRLF final
        size_t needed_data = pos + chunk_size + 2; // chunk + CRLF final
        if (buffer.size() < needed_data)
            break;
        // 4. Traiter le chunk
        if (chunk_size > 0)
            _request.appendBodyData(buffer.c_str() + pos, chunk_size);
        pos += chunk_size;
        // 5. Vérifier et skip le CRLF après le chunk
        if (pos + 2 <= buffer.size() && buffer.substr(pos, 2) != "\r\n") {
            _request.code = 400;
            return ERROR;
        }
        pos += 2;
        total_processed = pos;
        // 6. Fin du dernier chunk
        if (chunk_size == 0)
        {
            state = READ_END;
            break;
        }
    }
    // 7. Supprimer les données traitées
    if (total_processed > 0) {
        buffer = buffer.substr(total_processed);
    }
    return OK;
}

string	Client::findHeaderConnection()
{
	map<string, string>::const_iterator header = _request.getHeaders().find("Connection");
	if ((header != _request.getHeaders().end() && header->second.find("close") != string::npos) 
		|| _request.getHeaders().find("User-Agent") == _request.getHeaders().end()) {
			_keep_alive = false;
			return ("close");
	}
	_keep_alive = true;
	return ("keep-alive");
}

/*************************************************************************************************/
/* Response Function *****************************************************************************/

string	Client::getCodeMessage(int code)
{
	switch (code) {
		case 200: return ("200 OK");
		case 201: return ("201 Created");
		case 202: return ("202 Accepted");
		case 204: return ("204 No Content");
		case 205: return ("205 Reset Content");
		case 206: return ("206 Partial Content");
		case 300: return ("300 Multiple Choices");
		case 301: return ("301 Moved Permanently");
		case 302: return ("302 Found");
		case 303: return ("303 See Other");
		case 308: return ("308 Permanent Redirect");
		case 400: return ("400 Bad Request");
		case 403: return ("403 Forbidden");
		case 404: return ("404 Not Found");
		case 405: return ("405 Method Not Allowed");
		case 409: return ("409 Conflict");
		case 411: return ("411 Length Required");
		case 413: return ("413 Playload Too Large");
		case 415: return ("415 Unsupported Media Type");
		case 500: return ("500 Internal Server Error");
		case 501: return ("501 Not Implemented");
		case 505: return ("505 HTTP Version Not Supported");
	}
	ostringstream	tmp;
	tmp << code;
	return (tmp.str());
}

void	Client::buildResponse(int code)
{
	map<string, string>::const_iterator header;
	ostringstream	response;

	if (code == 0)
		return ;
	else if (code >= 400 && code < 600)
		handleError(code);
	else if (code >= 301 && code <= 308)
		_request.response.location = _request.getRedirectURI();
	response << "HTTP/1.1 " << getCodeMessage(code) << "\r\n";
	response << "Date: " << obtainDateHeader() << "\r\n";
	if (!_request.response.body.empty())
		response << "Content-Type: " << _request.response.content_type << "\r\n";
	response << "Content-Length: " << _request.response.body.size() << "\r\n";
	if ((code >= 301 && code <= 308) || _request.getMethod() == "POST") {
		header = _request.getHeaders().find("Host");
		if (header != _request.getHeaders().end() && !header->second.empty() && _request.response.location.find("http://") == string::npos)
			response << "Location: http://" << header->second << _request.response.location << "\r\n";
		else
		response << "Location: " << _request.response.location << "\r\n";
	}
	// header = _request.getHeaders().find("Connection");
	// if ((header != _request.getHeaders().end() && header->second.find("close") != string::npos) || _request.getHeaders().find("User-Agent") == _request.getHeaders().end())
	// 	response << "Connection: close\r\n";
	// else
	// 	response << "Connection: keep-alive\r\n";
	response << "Connection: " << findHeaderConnection() << "\r\n";
	response << "\r\n";
	if (!_request.response.body.empty())
		response << _request.response.body;
	_request.response.res = response.str();
}

void	Client::handleError(int code)
{
	string			message;
	ostringstream	response, body;
	ifstream		error_file;

	message = getCodeMessage(code);
	if (_config && !_config->error_page.empty())
	{
		map<int, string>::iterator it = _config->error_page.find(code);
		if (it == _config->error_page.end() || it->second.empty())
			body << "<html><body><h1>" << message << "</h1></body></html>" << endl;
		else {
			error_file.open(it->second.c_str());
			if (!error_file.is_open())
				body << "<html><body><h1>" << message << "</h1></body></html>" << endl;
			else {
				body << error_file.rdbuf();
				error_file.close();
			}
		}
	}
	else
		body << "<html><body><h1>" << message << "</h1></body></html>" << endl;
	_request.response.content_type = "text/html";
	_request.response.body = body.str();
}

string	Client::obtainDateHeader()
{
	char buf[128];
	struct tm *timeinfo = localtime(&last_activity);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return string(buf);
}


/*************************************************************************************************/
/* Setters ***************************************************************************************/

void	Client::setConfig(const string &URI)
{
	if (_server == NULL)
		return ;
	_config = _server->searchLocationMatch(URI);
	if (_config == NULL && _request.code == 0) {
		cout << RED << "Error: setLocationMatch(): no match found with URI(" << URI;
		cout << ")" << RESET << endl;
		return ;
	}
}
