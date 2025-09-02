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

int	Client::readData()
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

int Client::processBuffer()
{
    // Traitement des headers (une seule fois)
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
            // Pas encore tous les headers, on attend plus de données
            return (OK);
        }
    }
    // Traitement du body (peut être appelé plusieurs fois)
    if (state == READ_BODY) {
        if (_request.isChunked() == true) {
            // Pour les chunks, on traite autant qu'on peut à chaque fois
            if (!_buffer.empty()) {
                if (parseChunked(_buffer) == static_cast<size_t>(ERROR)) {
                    return ERROR;
                }
            }
            // Si parseChunked a tout traité et mis state=READ_END, c'est fini
            // Sinon on attend plus de données dans le prochain readData()
        }
        else if (!_buffer.empty()) {
            // Body classique avec Content-Length
            size_t remaining_len = _request.getExpectedBodyLen() - _request.getBodyLen();
            size_t to_copy = min(_buffer.length(), remaining_len);
            _request.appendBodyData(_buffer.c_str(), to_copy);
            _buffer = _buffer.substr(to_copy);
            
            // Vérifier si on a tout reçu
            if (_request.getBodyLen() >= _request.getExpectedBodyLen()) {
                state = READ_END;
            }
        }
		else
			state = READ_END;
    }
    return (OK);
}

int	Client::processRequest()
{
	// cout << "[ DEBUG ] :\n" << _request;
	string	method = _request.getMethod();
	if (isRequestWellFormedOptimized() == OK) {
		cout << BLUE << "📨 - REQUEST RECEIVED [socket:" << _client_fd << "]";
		cout << endl << "     Method:[\e[0m" << method << "\e[34m] URI:[\e[0m";
		cout << _request.getURI() << "\e[34m] Version:[\e[0m" << _request.getVersion();
		if (_config)
			cout << "\e[34m] FullPath:[\e[0m" << _config->full_path << "\e[34m]\e[0m" << endl;
		else
			cout << "\e[34m]\e[0m" << endl;
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
	return (OK);
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

size_t Client::parseChunked(std::string &buffer)
{
    size_t total_processed = 0;
    
    for (size_t i = 0; i < std::min(buffer.size(), size_t(30)); ++i) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    std::cout << std::endl;
    
    while (true)
    {
        size_t pos = total_processed;
        
        // 1. Chercher la fin de la ligne de taille (CRLF)
        size_t endline = buffer.find("\r\n", pos);
        if (endline == std::string::npos)
        {
            break;
        }

        // 2. Extraire et convertir la taille hexadécimale
        std::string hexsize = buffer.substr(pos, endline - pos);
        
        // Nettoyer la ligne de taille (supprimer espaces)
        hexsize.erase(0, hexsize.find_first_not_of(" \t"));
        hexsize.erase(hexsize.find_last_not_of(" \t") + 1);
        
        
        std::istringstream iss(hexsize);
        size_t chunk_size = 0;
        iss >> std::hex >> chunk_size;

        if (iss.fail()) {
            _request.code = 400;
            return ERROR;
        }
        

        pos = endline + 2; // Position après le CRLF de la taille

        // 3. Vérifier si on a reçu tout le chunk + son CRLF final
        size_t needed_data = pos + chunk_size + 2; // chunk + CRLF final
        if (buffer.size() < needed_data)
        {
            break;
        }

        // 4. Traiter le chunk
        if (chunk_size > 0) {
            _request.appendBodyData(buffer.c_str() + pos, chunk_size);
        }
        
        pos += chunk_size;

        // 5. Vérifier et skip le CRLF après le chunk
        if (pos + 2 <= buffer.size() && buffer.substr(pos, 2) != "\r\n") {
            if (pos + 2 <= buffer.size()) {
                for (int i = 0; i < 2; i++) {
                    printf("%02X ", (unsigned char)buffer[pos + i]);
                }
            }
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
		string redirectUrl = _request.getRedirectURI();
		sendRedirect(redirectUrl);
	}
	else
		return ;
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

static void	getErrorMessage(int code, string &message)
{
	switch (code) {
		case 400:
			message = "400 Bad Request";
			break;
		case 403:
  			message = "403 Forbidden";
			break;
		case 404:
			message = "404 Not Found";
			break;
		case 405:
			message = "405 Method Not Allowed";
			break;
		case 409:
			message = "409 Conflict";
			break;		
		case 413:
			message = "413 Playload Too Large";
			break;
		case 415:
			message = "415 Unsupported Media Type";
			break;
		case 500:
			message = "500 Internal Server Error";
			break;
		case 501:
			message = "501 Not Implemented";
			break;
		case 505:
			message = "505 HTTP Version Not Supported";
			break;
	}
}

void	Client::handleError(int code)
{
	string			message;
	ostringstream	response, body;
	ifstream		error_file;

	getErrorMessage(code, message);
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