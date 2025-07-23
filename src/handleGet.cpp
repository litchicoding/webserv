#include "../inc/Client.hpp"

void	Client::handleGet() {
	struct stat st;
	if (access(_URI.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(_URI.c_str(), &st) != 0)
		return(handleError(500));
	if (S_ISREG(st.st_mode))
	{
	 	std::cout << RED "file" RESET << std::endl;
		handleFileRequest();
	}
	else if (S_ISDIR(st.st_mode))
	{
	 	std::cout << RED "register" RESET << std::endl;
		handleDirectoryRequest();
	}
	else
	{
		std::cout << RED "Error : Not a regular file or directory: " << _URI << RESET << std::endl;
		return(handleError(403));
	}
}

void	Client::handleFileRequest()
{
	cout << "hey hey hey" << endl;
	if (access(_URI.c_str(), R_OK) != 0)
		return (handleError(403));
	
	// if(isCgiScript(_URI) == OK)
	//	;

	std::ifstream	file(_URI.c_str());
	if (!file.is_open())
	{
		std::cout << RED "Error : Cannot open file: " << _URI << RESET << std::endl;
		return (handleError(500));
	}
	std::ostringstream	body;
	body << file.rdbuf();

	std::cout << GREEN "handleFileRequest(): Get parsing File fonctionne !" RESET << std::endl;	

}

void	Client::handleDirectoryRequest()
{
	std::cout << "HEY handleDirResquest()" << std::endl;
   	if (_URI.empty() || _URI[_URI.size() - 1] != '/')
	{
		std::string redirectUri = _URI + "/";
		std::cout << RED "Redirecting to: " << redirectUri << RESET << std::endl;
		return (sendRedirect(redirectUri));
	}

	// Chercher un fichier index
    std::string indexFile = findIndexFile();
    if (!indexFile.empty()) {
        std::cout << GREEN "Index file found: " << indexFile << RESET << std::endl;
        _URI = indexFile;
        handleFileRequest();
    }
    
    // Si pas de fichier index, vérifier l'autoindex
    if (_config->autoindex == 1)
    {
        std::cout << BLUE "Generating directory listing for: " << _URI << RESET << std::endl;
        // return generateDirectoryListing();
    }
    else
        return (handleError(403));
}

std::string Client::findIndexFile()
{
	cout << "coucou" << endl;
	struct stat st;
	for (std::vector<std::string>::const_iterator it = _config->index.begin(); it != _config->index.end(); ++it)
    {
	    const std::string& indexPath = *it;
        if (access(indexPath.c_str(), F_OK) == 0 && access(indexPath.c_str(), R_OK) == 0)
		{
            if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
                return indexPath;
        }
    }
	cout << "coucou22222" << endl;
    return "";
}
