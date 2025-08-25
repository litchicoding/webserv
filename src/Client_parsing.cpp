#include "../inc/Client.hpp"

int	Client::request_well_formed_optimized() {

	string clean_URI = stripQueryString(_URI);
	// VALIDATION DE L'URI
	if (_URI.empty() || _URI[0] != '/')
		return(handleError(400), ERROR);
	if (_URI.find("..") != std::string::npos)
		return(handleError(403), ERROR);
	if (URI_Not_Printable(clean_URI))
		return(handleError(400), ERROR);
	if (_URI.size() > 2048)
		return(handleError(414), ERROR);

	// VALIDATION DE LA VERSION
	if (_version != "HTTP/1.1")
		return(handleError(505), ERROR);

	// VALIDATION ROOT LOCATION
	setConfig();
	if (_config == NULL)
		return (handleError(500), ERROR);
	if (!(_config->redirection.empty()))
	{
		string redirectUrl = _config->redirection.begin()->second;
		return (sendRedirect(redirectUrl), ERROR);
	}

	if (static_cast<size_t>(_config->client_max_body_size) < _body.size())
		return (handleError(413), ERROR);
	if (std::find(_config->methods.begin(), _config->methods.end(), _method) == _config->methods.end())
		return (handleError(405), ERROR);
	// if (location_have_redirection(_config) != OK) // voir si redirection précisée dans la location
	// 	return (handleError(301));

	// if (max_body_size(_config) != OK) // Voir si le body_size est respectée dans le fichier conf.
	// 	return (handleError(413));

	// if (is_method_allowed(_config) != OK) // Voir si method = GET POST DELETE ou en fonction de ce qui est autorisée dans fichier conf.
	// 	return (handleError(405));
	




	// VALIDATION DES HEADERS - Ici tout mettre dans une fonction et vérifier les différents HEADERS obligatoire.
	std::map<std::string, std::string>::iterator transferEncodingIt = _headersMap.find("Transfer-Encoding");
	// std::map<std::string, std::string>::iterator contentLengthIt = _headersMap.find("Content-Length");
		
	if (transferEncodingIt != _headersMap.end() && transferEncodingIt->second != "chunked")
		return(handleError(501), ERROR);
	// else if (transferEncodingIt == _headersMap.end() && contentLengthIt == _headersMap.end() && _method != "POST")
	// {
	// 	std::cout << RED "transferencodingit" RESET << std::endl;
	// 	return (handleError(400));
	// }
	// Transfer encoding + content length impossible
	// Si content-length vérifier que ce soit un nombre valide
	// HOST obligatoire en HTTP/1.1

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
