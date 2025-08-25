#include "../inc/Client.hpp"


void	Client::handlePost()
{
	struct stat st;
	string root = _config->full_path;
	
	if (access(root.c_str(), W_OK) != 0)
		return (handleError(403));

	map<string, string>::const_iterator it = _headersMap.find("Content-Type");
	if (it == _headersMap.end() || it->second.find("multipart/form-data") == string::npos)
		return (handleError(400));
	
	cout << GREEN "Welcome to PostFonction, root is : " << root << RESET << endl;
	
	if (_URI == "/upload")
	{
		// trouver le filename dans les headers !

		string filename = root + "/text.txt";

		ofstream file(filename.c_str(), ofstream::out);
		if (file.fail())
			return (handleError(500));
		file << _body;
		file.close();
	    cout << "Fichier " << filename << " créé avec succès !" << std::endl;

		string message = "Fichier créé avec succès\n";

		ostringstream response;
		response << "HTTP/1.1 201 Created\r\n";
		response << "Location: " + filename + "\r\n";
		response << "Content-Type: text/plain\r\n";
		response << "Content-Length: " << message.size() << "\r\n";
		response << "Connection: close\r\n";
		response << "\r\n";
		response << message;

		_response = response.str();
		_response_len = _response.size();
		return ;
	}

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
}

void	Client::isFilePost()
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
		return sendRedirect(redirectUri);
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
