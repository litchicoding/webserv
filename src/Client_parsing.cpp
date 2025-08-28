#include "../inc/Client.hpp"

bool	Client::isRequestCompleted()
{
	if (_chunked == true && _method == "POST") {
		if (_body.size() > _content_length)
			return (handleError(400), false);
		if (_body.size() < _content_length)
			return (false);
	}
	return (true);
}

int	Client::isRequestChunked()
{
	map<string, string>::iterator transfer;
	map<string, string>::iterator content_len;

	transfer = _headersMap.find("Transfer-Encoding");
	content_len = _headersMap.find("Content-Length");
	if (transfer != _headersMap.end() && transfer->second != "chunked") // can be gzip etc CHECK RFC
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
		_content_length = atoi(content_len->second.c_str());
		if (_content_length < 0)
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
