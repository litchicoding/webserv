#include "../inc/Client.hpp"

void	Client::handleGet() {
	struct stat st;
	string root = _config->full_path;
	cout << GREEN << "handleGet() - root(full_path) : " << RESET << root << endl;

	// ./WEBSITE/QSD/
	if (access(root.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(root.c_str(), &st) != 0)
		return(handleError(500));
	if (S_ISREG(st.st_mode))
	{
		// if (handleFileRequest() != OK)
		// 	return ERROR;
		// return OK;
		return (handleFileRequest());
	}
	else if (S_ISDIR(st.st_mode))
		return (handleDirectoryRequest());
	else
		return (handleError(403));
}

void	Client::handleFileRequest()
{
	const char	*uri = _config->full_path.c_str();
	cout << GREEN << "handleFileRequest() - full_path : " << RESET << uri << endl;
	if (access(uri, R_OK) != 0)
		return (handleError(403));

	// if(isCgiScript(uri) == OK)
	//	;

	std::ifstream	file(uri);
	if (!file.is_open())
	{
		std::cout << RED "Error : Cannot open file: " << RESET << uri << std::endl;
		return (handleError(500));
	}

	std::ostringstream	body;
	body << file.rdbuf();
	file.close();

	ostringstream	response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << getMIME(_config->full_path) << "\r\n";
	response << "Content-Length: " << body.str().size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body.str();

	_response = response.str();
	_response_len = _response.size();
}

void	Client::handleDirectoryRequest()
{
	string	uri = _config->full_path;
	if (uri.empty() || uri[uri.size() - 1] != '/')
	{
		string redirectUri = uri + "/";
		cout << RED "Redirecting to: " << redirectUri << RESET << std::endl;
		return (sendRedirect(redirectUri));
	}

	// Chercher un fichier index
    std::string indexFile = findIndexFile();
    if (!indexFile.empty()) {
        std::cout << GREEN "handleDirectoryRequest() - Index file found: " <<  RESET << indexFile << std::endl;
        _URI = indexFile;
		_config->full_path = indexFile;
        return (handleFileRequest());
    }
    
    // Si pas de fichier index, vérifier l'autoindex
    if (_config->autoindex == 1) // utiliser la macro AUTO_ON
    {
        std::cout << BLUE "Generating directory listing for: " << uri << RESET << std::endl;
		return ;
        // return generateDirectoryListing();
    }
    else
        return (handleError(403));
}

std::string Client::findIndexFile()
{
	struct stat st;
	for (std::vector<std::string>::const_iterator it = _config->index.begin(); it != _config->index.end(); ++it)
	{
		// const std::string& indexPath = _config->root + '/' + *it;
		const std::string& indexPath = _config->full_path + *it;
		if (access(indexPath.c_str(), F_OK) == 0 && access(indexPath.c_str(), R_OK) == 0)
		{
			if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
				return indexPath;
		}
	}
	return "";
}
