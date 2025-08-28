#include "../inc/Client.hpp"

int	Client::request_well_formed_optimized() {

	string clean_URI = stripQueryString(_URI);
	// VALIDATION DE L'URI
	if (_URI.empty() || _URI[0] != '/')
		return(handleError(400), ERROR);
	if (_URI.find("..") != std::string::npos)
		return(handleError(403), ERROR);
	if (URI_Not_Printable(clean_URI))
	{
		cout << "clean_URI : " << YELLOW << clean_URI << RESET << endl;
		return (handleError(400), ERROR);
	}
	if (_URI.size() > 2048)
		return(handleError(414), ERROR);

	// VALIDATION DE LA VERSION
	if (_version != "HTTP/1.1")
		return(handleError(505), ERROR);

	// VALIDATION ROOT LOCATION
	setConfig();
	if (_config == NULL)
		return (handleError(500), ERROR);
	// Est ce que la location dispose d'une redirection ?
	if (!(_config->redirection.empty()))
	{
		string redirectUrl = _config->redirection.begin()->second;
		return (sendRedirect(redirectUrl), ERROR);
	}
	// BodySize respectée ?
	if (_config->client_max_body_size < _body.size())
		return (handleError(413), ERROR);
	// Method autorisée au sein de la location 
	if (std::find(_config->methods.begin(), _config->methods.end(), _method) == _config->methods.end())
		return (handleError(405), ERROR);



	// VALIDATION DES HEADERS - Ici tout mettre dans une fonction et vérifier les différents HEADERS obligatoire.
	std::map<std::string, std::string>::iterator transferEncodingIt = _headersMap.find("Transfer-Encoding");
	std::map<std::string, std::string>::iterator contentLengthIt = _headersMap.find("Content-Length");
	
	// TE Existe t-il ? Si oui son contenu est il Chunked ?
	if (transferEncodingIt != _headersMap.end() && transferEncodingIt->second != "chunked")
	{
		// ContentLength existe t-il également ?
		if (contentLengthIt != _headersMap.end())
			return (handleError(400), ERROR);
		return(handleError(501), ERROR);
	}
	// Content Lenght existe t-il ?
	if (contentLengthIt != _headersMap.end())
	{
		// Regarder char/char si c'est bien du digitale !
		for (size_t i = 0; i < contentLengthIt->second.size(); i++)
		{
			if (!isdigit(contentLengthIt->second[i]))
				return (handleError(400), ERROR);
		}
		// Regarder si taille non négatif !
		if (atoll(contentLengthIt->second.c_str()) < 0)
			return (handleError(400), ERROR);
	}
	else
	{
		if (_method == "POST")
			return (handleError(400), ERROR);
	}

	
	

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
