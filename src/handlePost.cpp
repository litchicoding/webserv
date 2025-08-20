#include "../inc/Client.hpp"


void	Client::handlePost()
{
	struct stat st;
	string root = _config->full_path;
	cout << GREEN "Welcome to PostFonction, root is : " << root << RESET << endl;
	if (access(root.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(root.c_str(), &st) != 0)
		return(handleError(500));
	if (S_ISREG(st.st_mode))
	{
	 	std::cout << RED "file" RESET << std::endl;
		return (isFilePost());
	}
	else if (S_ISDIR(st.st_mode))
	{
	 	std::cout << RED "register" RESET << std::endl;
		return (isDirectoryPost());
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
		std::cout << "CGI à faire" << std::endl;
	else
		return (handleError(403));
}

void    Client::isDirectoryPost()
{
   	if (_URI.empty() || _URI[_URI.size() - 1] != '/')
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
