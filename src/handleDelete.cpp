#include "../inc/Client.hpp"


void	Client::handleDelete() {
	struct stat st;
	if (access(_URI.c_str(), F_OK) != 0)
		return(handleError(404));
	if (stat(_URI.c_str(), &st) != 0)
		return(handleError(500));
    if (access(_URI.c_str(), W_OK) != 0)
		return handleError(403);
	if (S_ISREG(st.st_mode))
	{
	 	std::cout << RED "file" RESET << std::endl;
		isFileDelete();
	}
	else if (S_ISDIR(st.st_mode))
	{
	 	std::cout << RED "register" RESET << std::endl;
		isDirectoryDelete();
	}
	else
	{
		std::cout << RED "Error : Not a regular file or directory: " << _URI << RESET << std::endl;
		return(handleError(403));
	}
}

void    Client::isFileDelete()
{
    // if(isCgiScript(_URI) == OK)
	//	;
    if (std::remove(_URI.c_str()) != 0)
		return(handleError(500));
	std::cout << GREEN "File Delete" RESET << std::endl;
    // sendResponse(204);
}

void    Client::isDirectoryDelete()
{
   	if (_URI.empty() || _URI[_URI.size() - 1] != '/')
        return (handleError(409));
    // if(isCgiScript(_URI) == OK)
    //     index_files
	// 	;
    if (delete_all_folder_content(_URI) != 0) {
        std::cout << RED "Error: Failed to delete folder content" RESET << std::endl;
        return (handleError(500));
    }
    std::string cleanPath = _URI.substr(0, _URI.length() - 1);
    if (std::remove(cleanPath.c_str()) != 0)
		return(handleError(500));
    std::cout << GREEN "Directory deleted successfully: " << _URI << RESET << std::endl;
    // sendResponse(204);
}

int Client::delete_all_folder_content(std::string URI)
{
	std::cout << GREEN "delete all folder content" RESET << URI << std::endl;
    // ...
	return 0;
}