#include "../inc/Client.hpp"


void	Client::handleDelete() {

	if (_URI != "/upload/")

	struct stat st;
	string root = _config->full_path;
	cout << GREEN << "ROOT is : " << root << RESET << endl;

	if (access(root.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(root.c_str(), &st) != 0)
	{
		cout << BLUE << "stat" << RESET << endl;
		return(handleError(500));
	}
	if (access(root.c_str(), W_OK) != 0)
		return handleError(403);
	if (S_ISREG(st.st_mode))
		return (isFileDelete());
	else if (S_ISDIR(st.st_mode))
		return (isDirectoryDelete());
	else
	{
		std::cout << RED "Error handleDelete() : Not a regular file or directory: " << _URI << RESET << std::endl;
		return (handleError(403));
	}
}

void	Client::isFileDelete()
{
	// if(isCgiScript(_URI) == OK)
	//	;
	cout << YELLOW "isfileDelete _uri = " << _config->full_path << RESET << endl;
	if (std::remove(_config->full_path.c_str()) != OK)
		return (handleError(500));
	
	std::cout << GREEN "File Delete" RESET << std::endl;

	ostringstream	response;
	response << "HTTP/1.1 204 No Content\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";

	_response = response.str();
	_response_len = _response.size();
}

void	Client::isDirectoryDelete()
{
	if (_URI.empty() || _URI[_URI.size() - 1] != '/')
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

	if (delete_all_folder_content(_config->full_path) != OK)
	{
		if (access(_config->full_path.c_str(), W_OK) != OK)
			return (handleError(403));
		return (handleError(500));
	}

	if (std::remove(_config->full_path.c_str()) != OK)
		return(handleError(500));

	ostringstream	response;
	response << "HTTP/1.1 204 No Content\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";

	_response = response.str();
	_response_len = _response.size();
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