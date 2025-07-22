#include "../inc/Client.hpp"

void	Client::handlePost()
{
	if (access(_URI.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(_URI.c_str(), &st) != 0)
		return(handleError(500));
	if (S_ISREG(st.st_mode))
	{
	 	std::cout << RED "file" RESET << std::endl;
		isFilePost();
	}
	if (S_ISDIR(st.st_mode))
	{
	 	std::cout << RED "register" RESET << std::endl;
		isDirectoryPost();
	}
	else
	{
		std::cout << RED "Error : Not a regular file or directory: " << _URI << RESET << std::endl;
		return(handleError(403));
	}


	// std::ofstream	outfile(_URI.c_str());
	// if (!outfile.is_open())
	// 	return(handleError(500));
}

void    Client::isFilePost()
{
    // if(isCgiScript(_URI) == OK)
	//	;
	if (1)
		std::cout << "a supprimer" << std::endl;
	else
		return (handleError(403));
}

void    Client::isDirectoryPost()
{
   	if (_URI.empty() || _URI.back() != '/')
	{
		std::string redirectUri = _URI + "/";
		std::cout << RED "Redirecting to: " << redirectUri << RESET << std::endl;
		// return sendRedirect(redirectUri); // 301 Redirection
	}
	std::string indexFile = findIndexFile();
	if (!indexFile.empty())
	{
	    std::cout << GREEN "Index file found: " << indexFile << RESET << std::endl;
	    _URI = indexFile;
	    isFilePost();
	}
	else
		return (handleError(403));
}
