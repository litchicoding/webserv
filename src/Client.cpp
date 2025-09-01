#include "webserv.hpp"

/*************************************************************************************************/
/* Constructor and Deconstructor *****************************************************************/

Client::Client(int listen_fd, int epoll_fd)
: state(READ_HEADERS), _listen_fd(listen_fd), _server(NULL), _config(NULL), _keep_alive(false)
{
	cout << GREEN << "***   Client Connection   ***" << RESET << endl;
	socklen_t	client_addr_len = sizeof(_client_addr);
	_client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&_client_addr), &client_addr_len);
	if (_client_fd == INVALID) {
		perror("accept");
		return ;
	}
	add_fd_to_epoll(epoll_fd, _client_fd);
}

Client::~Client()
{
	cout << GREEN << "***   Client Deconstruction   ***" << RESET << endl;
	if (_client_fd >= 0)
		close(_client_fd);
}

/*************************************************************************************************/
/* Request Parsing *******************************************************************************/

void	Client::sendResponse()
{
	write(_client_fd, _request.response.c_str(), _request.response.size());
	cout << CYAN << "   - RESPONSE TO REQUEST [socket:" << _client_fd << "] : " << RESET;
	size_t pos = _request.response.find("\n");
	cout << _request.response.substr(0, pos) << endl << endl;
}

int	Client::readData(int epoll_fd)
{
	char	buffer[8192];
	ssize_t	bytes_read;

	memset(buffer, 0, sizeof(buffer));
	bytes_read = recv(_client_fd, buffer, sizeof(buffer), 0);
	if (bytes_read < 0) {
		cout << RED "Error: readData(): while reading request." RESET << endl;
		return (ERROR);
	}
	if (bytes_read == 0)
		return (OK);
	_buffer.append(buffer, bytes_read);
	return (processBuffer());
}

int	Client::processBuffer()
{
	if (state == READ_HEADERS) {
		size_t	pos = _buffer.find("\r\n\r\n");
		if (pos != string::npos) {
			string	headers = _buffer.substr(0, pos);
			_request.parsingHeaders(headers);
			_buffer = _buffer.substr(pos + 4);
			state = READ_BODY;
		}
		else
			return (ERROR); // error 400 ?
	}
	if (state == READ_BODY) {
		// IMPLEMENT CHUNKED BODY
		if (!_buffer.empty()) {
			size_t	remaining_len = _request.getExpectedBodyLen() - _request.getBodyLen();
			size_t	to_copy = min(_buffer.length(), remaining_len);
			_request.appendBodyData(_buffer.c_str(), to_copy);
			_buffer = _buffer.substr(to_copy);
		}
		if (_request.isBodyEnded() == true)
			state = READ_END;
	}
	return (OK);
}

int	Client::processRequest()
{
	// cout << "[ DEBUG ] :\n" << _request;
	string	method = _request.getMethod();
	cout << BLUE << "📨 - REQUEST RECEIVED [socket:" << _client_fd << "]";
	cout << endl << "     Method:[\e[0m" << method << "\e[34m] URI:[\e[0m";
	cout << _request.getURI() << "\e[34m] Version:[\e[0m" << _request.getVersion();
	if (_config)
		cout << "\e[34m] FullPath:[\e[0m" << _config->full_path << "\e[34m]\e[0m" << endl;
	else
		cout << "\e[34m]\e[0m" << endl;
	if (isRequestWellFormedOptimized() == OK) {
		if (method == "GET")
			handleGet();
		else if (method == "POST")
			handlePost();
		else if (method == "DELETE")
			handleDelete();
		else
			_request.code = 501;
	}
	buildResponse(_request.code);
	return (ERROR);
}

int	Client::isRequestWellFormedOptimized() {

	string	URI, clean_URI;
	
	URI = _request.getURI();
	clean_URI = stripQueryString(URI);
	// VALIDATION DE L'URI
	if (URI.empty() || URI[0] != '/' || URI_Not_Printable(clean_URI)) {
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
	setConfig(URI);
	if (_config == NULL)  {
		_request.code = 500;
		return (ERROR);
	}
	if (!_config->redirection.empty()) {
		_request.code = 301;
		return (ERROR);
	}
	if (_config->client_max_body_size < _request.getBodyLen()) {
		_request.code = 413;
		return (ERROR);
	}
	if (find(_config->methods.begin(), _config->methods.end(), _request.getMethod()) == _config->methods.end()) {
		_request.code = 405;
		return (ERROR);
	}
	if (isRequestWellChunked(_request.getHeaders()) != OK)
		return (ERROR);
	// HOST obligatoire en HTTP/1.1
	return (OK);
}

int	Client::isRequestWellChunked(const map<string, string> &headers)
{
	map<string, string>::const_iterator transfer;
	map<string, string>::const_iterator content_len;
	int									code = 0;

	transfer = headers.find("Transfer-Encoding");
	content_len = headers.find("Content-Length");
	if (transfer != headers.end() && transfer->second != "chunked") // can be gzip etc CHECK RFC
		code = 501;
	else if (transfer != headers.end() && content_len != headers.end()) // both present
		code = 400;	
	else if (transfer != headers.end())  // just transfer-encoding
		return (OK);
	if (content_len == headers.end() && _request.getMethod() == "POST")
		code = 400;
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

void	Client::resetRequest()
{
	state = READ_HEADERS;
	map<string, string>::const_iterator header = _request.getHeaders().find("Connection");
	if (header != _request.getHeaders().end() && header->second.find("keep-alive") != string::npos)
		_keep_alive = true;
	else
		_keep_alive = false;
	_config = NULL;
	_request.resetRequest();
	_buffer.clear();
}

/*************************************************************************************************/
/* Response Function *****************************************************************************/

void	Client::buildResponse(int code)
{
	if (code == 0)
		return ;
	else if (code >= 400 && code <= 600)
		handleError(code);
	else if (code == 301) {
		string redirectUrl = _config->redirection.begin()->second;
		sendRedirect(redirectUrl);
	}
	// else {

	// }
}


void	Client::sendRedirect(const string &URI)
{
	ostringstream	response;

	response << "HTTP/1.1 301 Moved Permanently" << "\r\n";
	response << "Location: " << URI << "\r\n";
	map<string, string>::const_iterator header = _request.getHeaders().find("Connection");
	if (header != _request.getHeaders().end() && header->second.find("keep-alive") != string::npos)
		response << "Connection: keep-alive\r\n";
	else
		response << "Connection: close\r\n";
	response << "Content-Length: 0\r\n";
	response << "\r\n";
	_request.response = response.str();
}

static void	getErrorData(int code, string &message, string &error_path)
{
	switch (code) {
		case 400:
			message = "400 Bad Request";
			error_path = "www/error_pages/400.html";
			break;
		case 403:
  			message = "403 Forbidden";
			error_path = "www/error_pages/403.html";
			break;
		case 404:
			message = "404 Not Found";
			error_path = "www/error_pages/404.html";			
			break;
		case 405:
			message = "405 Method Not Allowed";
			error_path = "www/error_pages/405.html";			
			break;
		case 409:
			message = "409 Conflict";
			error_path = "www/error_pages/409.html";			
			break;		
		case 413:
			message = "413 Playload Too Large";
			error_path = "www/error_pages/413.html";			
			break;
		case 415:
			message = "415 Unsupported Media Type";
			error_path = "www/error_pages/415.html";			
			break;
		case 500:
			message = "500 Internal Server Error";
			error_path = "www/error_pages/500.html";			
			break;
		case 501:
			message = "501 Not Implemented";
			error_path = "www/error_pages/501.html";			
			break;
		case 505:
			message = "505 HTTP Version Not Supported";
			error_path = "www/error_pages/505.html";			
			break;
	}
}

void	Client::handleError(int code)
{
	string			message, error_path;
	ostringstream	response, body;
	ifstream		error_file;

	getErrorData(code, message, error_path);
	error_file.open(error_path.c_str());
	if (!error_file.is_open())
		body << "<html><body><h1>" << message << "</h1></body></html>" << endl;
	else {
		body << error_file.rdbuf();
		error_file.close();
	}
	response << "HTTP/1.1 " << message << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body.str().size() << "\r\n";
	map<string, string>::const_iterator header = _request.getHeaders().find("Connection");
	if (header != _request.getHeaders().end() && header->second.find("keep-alive") != string::npos)
		response << "Connection: keep-alive\r\n";
	else
		response << "Connection: close\r\n";
	response << "\r\n";
	response << body.str();
	_request.response = response.str();
}

/*************************************************************************************************/
/* Setters ***************************************************************************************/

void	Client::setConfig(const string &URI)
{
	if (_server == NULL)
		return ;
	_config = _server->searchLocationMatch(URI);
	if (_config == NULL) {
		cout << RED << "Error: setLocationMatch(): no match found with URI(" << URI;
		cout << ")" << RESET << endl;
		return ;
	}
}