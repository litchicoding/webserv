#include "../inc/Client.hpp"

void	Client::handleGet() {
	struct stat st;
	string clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), F_OK) != 0)
		return (handleError(404));
	if (stat(clean_path.c_str(), &st) != 0)
		return (handleError(500));
	if (S_ISREG(st.st_mode))
		handleFileRequest();
	else if (S_ISDIR(st.st_mode))
		handleDirectoryRequest();
	else
	{
		cout << "handleGet(): Not a File and Not a Dir" << endl;
		return (handleError(403));
	}
}

void	Client::handleFileRequest()
{
		string clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), R_OK) != 0)
	{
		cout << YELLOW "not permission" RESET << endl;
		return (handleError(403));
	}
	if (isCgi())
	{
		handleCGI();
		return;
	}

	std::ifstream	file(clean_path.c_str());
	if (!file.is_open())
	{
		std::cout << RED "Error : Cannot open file: " << _config->full_path << RESET << std::endl;
		return (handleError(500));
	}

	std::ostringstream	body;
	body << file.rdbuf();
	file.close();

	ostringstream	response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << getMIME(_config->full_path) << "\r\n";
	response << "Content-Length: " << body.str().size() << "\r\n";
	map<string, string>::iterator header = _headersMap.find("Connection");
	if (header != _headersMap.end() && header->second.find("keep-alive") != string::npos)
		response << "Connection: keep-alive\r\n";
	else
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
		_config->full_path = indexFile;
		return (handleFileRequest());
	}
		
	// Si pas de fichier index, vérifier l'autoindex
	if (_config->autoindex == 1)
	{
		return generateDirectoryListing();
	}
	else
		return (handleError(403));
}

std::string Client::findIndexFile()
{
	struct stat st;
	for (std::vector<std::string>::const_iterator it = _config->index.begin(); it != _config->index.end(); ++it)
	{
		const std::string& indexPath = _config->full_path + *it;
		if (access(indexPath.c_str(), F_OK) == 0 && access(indexPath.c_str(), R_OK) == 0)
		{
			if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
				return indexPath;
		}
	}
	return "";
}

#include<dirent.h>

void	Client::generateDirectoryListing()
{
	ostringstream body;
	string path = _config->full_path;

	body << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << _URI << "</title>\n";
	body << "<style>body { font-family: monospace; } a { text-decoration: none; }</style>\n";
	body << "</head>\n<body>\n";
	body << "<h1>Index of " << _URI << "</h1>\n<hr>\n<ul>\n";

	DIR *dir = opendir(path.c_str());
	if (!dir)
		return (handleError(500));
	
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		struct stat st;
		string name = entry->d_name;
		if (name == ".")
			continue ;

		string fullPath = path + name;
		string link = _URI;

		if (!link.empty() && link[link.size() - 1] != '/')
			link += '/';
		link += name;
		if (stat(fullPath.c_str(), &st) == OK && S_ISDIR(st.st_mode))
		{
			link += "/";
			name += "/";
		}
		body << "<li><a href=\"" << link << "\">" << name << "</a></li>\n";
	}
	if (closedir(dir) != OK)
		return (handleError(500));

	body << "</ul>\n<hr>\n</body>\n</html>\n";

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body.str().size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body.str();

	_response = response.str();
	_response_len = _response.size();
}