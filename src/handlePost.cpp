#include "../inc/Client.hpp"

void	Client::handlePost()
{
	struct stat st;
	string root, filename, message;
	
	root = _config->full_path;
	if (access(root.c_str(), W_OK) != 0)
		return (handleError(403));
	if (_URI == "/upload")
	{
		map<string, string>::iterator it = _headersMap.find("Content-Type");
		if (it == _headersMap.end() || it->second.find("multipart/form-data") == string::npos)
			return (handleError(400));
		filename = findFileName();
		if (filename.empty() || !filename.size())
			return (handleError(400));
		copyFile(root + "/" + filename);
		message = "File creation succeeded\n";

		ostringstream response;
		response << "HTTP/1.1 201 Created\r\n";
		response << "Location: " + root + "/" + filename + "\r\n";
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

void	Client::copyFile(const string &filename)
{
	size_t	start, end, pos;
	string	boundary, target;

	ofstream file(filename.c_str(), ofstream::out);
	if (!file.is_open())
		return (handleError(500));

	// Copy file and skip boundaries + headers
	pos = _body.find("Content-Type:");
	if (pos == string::npos)
		return ;
	start = _body.find("\n", pos);
	if (start == string::npos)
		return ;
	start += 1;
	end = _body.find("--", start);
	if (end == string::npos)
		return ;
	if (_body[end - 1] == '\n')
		end -= 2;
	file << _body.substr(start, end - start);
	file.close();
}

string	Client::findFileName()
{
	string	filename, target;
	size_t	start, end;
	
	start = _body.find("filename=\"");
	if (start == string::npos)
		return ("");
	target = "filename=\"";
	if (_body.compare(start, target.size(), target))
		return ("");
	start = start + target.size();
	end = start;
	while (_body[end] != '\0' && _body[end] != 34)
		end++;
	filename = _body.substr(start, end - start);
	return (filename);
}