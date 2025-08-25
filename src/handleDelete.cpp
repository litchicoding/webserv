#include "../inc/Client.hpp"


void	Client::handleDelete() {
	struct stat st;
	string root = _config->full_path;
	cout << GREEN << root << RESET << endl;
	if (access(root.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(root.c_str(), &st) != 0)
		return(handleError(500));
	if (access(root.c_str(), W_OK) != 0)
		return handleError(403);
	if (S_ISREG(st.st_mode))
		isFileDelete();
	else if (S_ISDIR(st.st_mode))
		isDirectoryDelete();
	else
	{
		std::cout << RED "Error : Not a regular file or directory: " << _URI << RESET << std::endl;
		return (handleError(403));
	}
}

void	Client::isFileDelete()
{
	// if(isCgiScript(_URI) == OK)
	//	;
	cout << YELLOW "isfiledelete _uri = " << _URI << RESET << endl;
	if (std::remove(_URI.c_str()) != OK)
		return (handleError(500));
	
	ostringstream	response;
	response << "HTTP/1.1 204 No Content\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";

	_response = response.str();
	_response_len = _response.size();
}

void	Client::isDirectoryDelete()
{
	cout << YELLOW "isDirDelete _uri = " << _URI << RESET << endl;

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
	if (delete_all_folder_content(_URI) != OK) {
		std::cout << RED "Error: Failed to delete folder content" RESET << std::endl;
		return (handleError(500));
	}
	std::string cleanPath = _URI.substr(0, _URI.length() - 1);
	if (std::remove(cleanPath.c_str()) != OK)
		return(handleError(500));
	// sendResponse(204);
}

int Client::delete_all_folder_content(std::string URI)
{
	std::cout << GREEN "delete all folder content" RESET << URI << std::endl;
	// ...
	return 0;
}