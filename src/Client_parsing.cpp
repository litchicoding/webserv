#include "../inc/Client.hpp"

int	Client::getCompleteRequest(int epoll_fd)
{
	char	buffer[4064];
	int		bytes_read;
	
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = read(_client_fd, buffer, sizeof(buffer) - 1);
		if (bytes_read <= 0) {
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _client_fd, NULL);
			// delete this;
			close(_client_fd);
			if (bytes_read < 0) {
				cout << RED "Error: handleClientRequest(): while reading client request." RESET << endl;
				return ERROR;
			}
			break ;
		}
		_body += buffer;
	}
	if (_body.size() < (size_t)_content_len_target || _body.size() > (size_t)_content_len_target)
		return (handleError(400), ERROR);
	if (_config->client_max_body_size < _body.size())
		return (handleError(413), ERROR);
	return (OK);
}

int	Client::getChunkedRequest(int epoll_fd)
{
	char	buffer[4064];
	int		bytes_read;
	
	cout << BLUE "Chunked Detected !" RESET << endl;

    cout << BLUE "Current _raw_body content: [" << _raw_body << "]" RESET << endl;
    
    // IMPORTANT: _raw_body contient probablement encore les en-têtes HTTP
    // Il faut les séparer d'abord
    
    // Chercher où finissent les en-têtes HTTP (\r\n\r\n)
    size_t headers_end = _raw_body.find("\r\n\r\n");
    if (headers_end != string::npos) {
        // Les en-têtes se terminent, le body chunked commence après
        string temp_body = _raw_body.substr(headers_end + 4);
        _raw_body = temp_body; // Garder seulement la partie chunked
        cout << CYAN "Separated chunked body: [" << _raw_body << "]" RESET << endl;
    }

	while (true)
    {
        cout << BLUE " Reading more chunks..." << RESET << endl;
        
        // Vérifier si on a déjà la fin des chunks
        if (_raw_body.find("0\r\n\r\n") != string::npos) {
            cout << GREEN "Found end of chunks!" RESET << endl;
            break;
        }
        
        memset(buffer, 0, sizeof(buffer));
        bytes_read = read(_client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
                return INCOMPLETE;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _client_fd, NULL);
            close(_client_fd);
            cout << RED "Error: getChunked - read failed." << RESET << endl;
            return (handleError(400), ERROR);
        }
        else if (bytes_read == 0)
        {
            cout << YELLOW "Connection closed by client" << RESET << endl;
            // Connexion fermée, vérifier si on a tout reçu
            if (_raw_body.find("0\r\n\r\n") == string::npos) {
                return (handleError(400), ERROR);
            }
            break;
        }

        _raw_body.append(buffer, bytes_read);
        cout << YELLOW << "Updated _raw_body: [" << _raw_body << "]" RESET << endl;
        
        // Protection contre les requêtes trop longues
        if (_raw_body.size() > 1048576) {
            return (handleError(413), ERROR);
        }
    }
    
    // Maintenant parser les chunks dans _raw_body
    cout << BLUE "Starting to parse chunks from: [" << _raw_body << "]" << RESET << endl;
    
    // Retirer la terminaison finale si présente
    size_t end_pos = _raw_body.find("0\r\n\r\n");
    if (end_pos != string::npos) {
        _raw_body = _raw_body.substr(0, end_pos);
        cout << BLUE "Cleaned _raw_body: [" << _raw_body << "]" << RESET << endl;
    }

    // Parser les chunks
    size_t pos = 0;
    _body.clear(); // S'assurer que _body est vide
    
    while (pos < _raw_body.size())
    {
        cout << CYAN "Parsing at position: " << pos << RESET << endl;
        
        // Chercher la fin de la taille du chunk
        size_t end_size = _raw_body.find("\r\n", pos);
        if (end_size == string::npos) {
            cout << RED "No \\r\\n found for chunk size" << RESET << endl;
            return (handleError(400), ERROR);
        }
    
        // Extraire la taille hexadécimale
        string hex_size = _raw_body.substr(pos, end_size - pos);
        cout << CYAN "Hex size: [" << hex_size << "]" << RESET << endl;
        
        // Validation de la chaîne hexadécimale
        if (hex_size.empty()) {
            return (handleError(400), ERROR);
        }
        
        // Convertir en entier
        char* endptr;
        errno = 0;
        long chunk_size = strtol(hex_size.c_str(), &endptr, 16);
        
        if (errno != 0 || *endptr != '\0' || chunk_size < 0) {
            cout << RED "Invalid hex conversion" << RESET << endl;
            return (handleError(400), ERROR);
        }
        
        cout << CYAN "Chunk size: " << chunk_size << RESET << endl;
        pos = end_size + 2; // Passer le \r\n

        // Si chunk de taille 0, c'est la fin
        if (chunk_size == 0) {
            break;
        }

        // Vérifier qu'on a assez de données
        if (pos + chunk_size > _raw_body.size()) {
            cout << RED "Not enough data for chunk" << RESET << endl;
            return (handleError(400), ERROR);
        }

        // Extraire les données du chunk
        string chunk_data = _raw_body.substr(pos, chunk_size);
        cout << CYAN "Chunk data: [" << chunk_data << "]" << RESET << endl;
        _body += chunk_data;
        pos += chunk_size + 2; // chunk_size + \r\n à la fin du chunk
    }

    // Vérification finale de la taille
    if (_config && _config->client_max_body_size > 0 && 
        _body.size() > _config->client_max_body_size) {
        return (handleError(413), ERROR);
    }
    
    cout << GREEN "Final parsed body: [" << _body << "]" << RESET << endl;
    cout << GREEN "Body length: " << _body.size() << RESET << endl;
    return OK;
}

int	Client::isRequestChunked()
{
	map<string, string>::iterator transfer;
	map<string, string>::iterator content_len;

	transfer = _headersMap.find("Transfer-Encoding");
	content_len = _headersMap.find("Content-Length");
	if (transfer != _headersMap.end() && trim(transfer->second) != "chunked") // can be gzip etc CHECK RFC
		return(handleError(501), ERROR);
	else if (transfer != _headersMap.end() && content_len != _headersMap.end()) // both present
		return(handleError(400), ERROR);
	else if (transfer != _headersMap.end()) {  // just transfer-encoding
		_chunked = true;
		return (OK);
	}
	if (content_len == _headersMap.end() && _method == "POST")
		return (handleError(400), ERROR);
	else if (content_len != _headersMap.end()) {
		if (content_len->second.empty() || content_len->second.size() > 19)
			return (handleError(400), ERROR);
		for (size_t i = 0; i < content_len->second.size(); i++) {
			if (!isdigit(content_len->second[i]))
				return (handleError(400), ERROR);
		}
		_chunked = true;
		_content_len_target = atoi(content_len->second.c_str());
		if (_content_len_target < 0)
			return (handleError(400), ERROR);
	}

	return (OK);
}

int	Client::request_well_formed_optimized() {
	string clean_URI;
	
	clean_URI = stripQueryString(_URI);
	// VALIDATION DE L'URI
	if (_URI.empty() || _URI[0] != '/')
		return(handleError(400), ERROR);
	if (_URI.find("..") != std::string::npos)
		return(handleError(403), ERROR);
	if (URI_Not_Printable(clean_URI))
		return (handleError(400), ERROR);
	if (_URI.size() > 2048)
		return(handleError(414), ERROR);
	// VALIDATION DE LA VERSION
	if (_version != "HTTP/1.1")
		return(handleError(505), ERROR);
	// VALIDATION ROOT LOCATION AND DIRECTIVES
	setConfig();
	if (_config == NULL)
		return (handleError(500), ERROR);
	if (!_config->redirection.empty()) {
		string redirectUrl = _config->redirection.begin()->second;
		return (sendRedirect(redirectUrl), ERROR);
	}
	if (_config->client_max_body_size < _body.size())
		return (handleError(413), ERROR);
	if (find(_config->methods.begin(), _config->methods.end(), _method) == _config->methods.end())
		return (handleError(405), ERROR);
	// VALIDATION DES HEADERS - Ici tout mettre dans une fonction et vérifier les différents HEADERS obligatoire.
	if (isRequestChunked() == ERROR)
		return (ERROR);
	// HOST obligatoire en HTTP/1.1
	cout << BLUE << "📨 - REQUEST RECEIVED [socket:" << _client_fd << "]";
	cout << endl << "     Method:[\e[0m" << _method << "\e[34m] URI:[\e[0m";
	cout << _URI << "\e[34m] Version:[\e[0m" << _version;
	cout << "\e[34m] FullPath:[\e[0m" << _config->full_path << "\e[34m]" << endl;
	return OK;
}

int	Client::handleMethodLine(std::string& line)
{
	std::istringstream  iss(line);
	if (!(iss >> this->_method >> this->_URI >> this->_version))
		return(handleError(400), ERROR);
	return OK;
}

int	Client::handleHeaders(std::string& line)
{
	size_t delimiterPos = line.find(':');
	if (delimiterPos == std::string::npos)
		return(handleError(400), ERROR);
	std::string key = line.substr(0, delimiterPos);
	std::string value = line.substr(delimiterPos + 1);
	if (key.empty())
		return(handleError(400), ERROR);
	this->_headersMap[key] = value;
	return OK;
}


void    Client::handleBody(std::string& line)
{
	this->_body.append(line + "\n");
}
