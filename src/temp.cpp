#include "../inc/Client.hpp"

void	Client::request_well_formed_optimized() {

	// VALIDATION DE L'URI
	if (_URI.empty() || _URI[0] != '/')
		return(handleError(400));
	if (_URI.find("..") != std::string::npos)
		return(handleError(403));
	if (URI_Not_Printable(_URI))
		return(handleError(400));
	if (_URI.size() > 2048)
		return(handleError(414));
			
	// VALIDATION DE LA VERSION
	if (_version != "HTTP/1.1")
		return(handleError(505));

	// VALIDATION ROOT LOCATION
	setConfig();
	if (_config == NULL)
		return (handleError(404));
	if (!(_config->redirection.empty()))
		return (handleError(301));
	if (_config->client_max_body_size < _body.size())
		return (handleError(413));
	if (std::find(_config->methods.begin(), _config->methods.end(), _method) == _config->methods.end())
		return (handleError(405));

	// if (location_have_redirection(_config) != OK) // voir si redirection précisée dans la location
	// 	return (handleError(301));

	// if (max_body_size(_config) != OK) // Voir si le body_size est respectée dans le fichier conf.
	// 	return (handleError(413));

	// if (is_method_allowed(_config) != OK) // Voir si method = GET POST DELETE ou en fonction de ce qui est autorisée dans fichier conf.
	// 	return (handleError(405));
	
	// VALIDATION DES HEADERS - Ici tout mettre dans une fonction et vérifier les différents HEADERS obligatoire.
	std::map<std::string, std::string>::iterator transferEncodingIt = _headersMap.find("Transfer-Encoding");
	std::map<std::string, std::string>::iterator contentLengthIt = _headersMap.find("Content-Length");
		
	if (transferEncodingIt != _headersMap.end() && transferEncodingIt->second != "chunked")
		return(handleError(501));
	else if (transferEncodingIt == _headersMap.end() && contentLengthIt == _headersMap.end() && _method != "POST")
		return (handleError(400));
	// Transfer encoding + content length impossible
	// Si content-length vérifier que ce soit un nombre valide
	// HOST obligatoire en HTTP/1.1
}

void	Client::start()
{
	if (_method == "GET")
		handleGet();
	if (_method == "POST")
		handlePost();
	if (_method == "DELETE")
		handleDelete();
}
