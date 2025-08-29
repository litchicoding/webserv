#include "../inc/Client.hpp"


void	Client::handleDelete() {

	// if (_URI.rfind("/upload/", 0) != OK)
	// 	return (handleError(403));


	// if (_URI == "/upload/")
	// 	return (handleError(403));

	struct stat st;
	string clean_path = urlDecode(_config->full_path);
	cout << GREEN << "clean path is : " << clean_path << RESET << endl;

	if (access(clean_path.c_str(), F_OK) != 0) {
		_request.code = 404;
		return ;
	}
	if (stat(clean_path.c_str(), &st) != 0)
	{
		cout << BLUE << "stat" << RESET << endl;
		_request.code = 500;
		return ;
	}
	if (access(clean_path.c_str(), W_OK) != 0)
		_request.code = 403;
	if (S_ISREG(st.st_mode))
		return (isFileDelete());
	else if (S_ISDIR(st.st_mode))
		return (isDirectoryDelete());
	else
	{
		std::cout << RED "Error handleDelete() : Not a regular file or directory: " << _request.getURI() << RESET << std::endl;
		_request.code = 403;
		return ;
	}
}

void	Client::isFileDelete()
{
	// if(isCgiScript(_URI) == OK)
	//	;
	string clean_path = urlDecode(_config->full_path);
	cout << YELLOW "isfileDelete clean_path = " << clean_path << RESET << endl;
	if (std::remove(clean_path.c_str()) != OK)
		return (handleError(500));
	
	ostringstream	response;
	response << "HTTP/1.1 204 No Content\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	_request.response = response.str();
}

void	Client::isDirectoryDelete()
{
	string URI = _request.getURI();
	if (URI.empty() || URI[URI.size() - 1] != '/')
		return (handleError(409));

	// if(isCgiScript(_URI) == OK)
	// {
	// 	std::string indexFile = findIndexFile();
	// 	if (!indexFile.empty()) {
	// 		std::cout << GREEN "Index file found: " << indexFile << RESET << std::endl;
	// 		_config->full_path = indexFile;
	// 		// CGI à faire ici
	// 	}
	// 	else
	// 		return (handleError(403));
	// }

	string clean_path = urlDecode(_config->full_path);
	if (delete_all_folder_content(clean_path) != OK)
	{
		if (access(clean_path.c_str(), W_OK) != OK)
			return (handleError(403));
		return (handleError(500));
	}

	if (std::remove(clean_path.c_str()) != OK)
		return(handleError(500));

	ostringstream	response;
	response << "HTTP/1.1 204 No Content\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	_request.response = response.str();
}

int Client::delete_all_folder_content(std::string dirPath)
{
	DIR	*dir = opendir(dirPath.c_str());
	if (!dir)
		return ERROR;
	
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		string name = entry->d_name;

		if (name == "." || name == "..")
			continue;

		string fullpath = dirPath + "/" + name;

		struct stat st;
		if (stat(fullpath.c_str(), &st) == -1)
			return ERROR;
		if (S_ISDIR(st.st_mode))
		{
			if (delete_all_folder_content(fullpath) != OK)
			{
				closedir(dir);
				return ERROR;
			}
		}
		cout << "exemple : " << fullpath.c_str() << endl;
		if (remove(fullpath.c_str()) != OK)
		{
			closedir(dir);
			return ERROR;
		}
	}
	closedir(dir);
	return OK;
}