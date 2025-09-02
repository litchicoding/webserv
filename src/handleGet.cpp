#include "../inc/Client.hpp"

int	Client::handleGet() {
	struct stat st;
	string clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), F_OK) != 0)
		return (404);
	if (stat(clean_path.c_str(), &st) != 0)
		return (500);
	if (S_ISREG(st.st_mode))
		return (handleFileRequest());
	else if (S_ISDIR(st.st_mode))
		return (handleDirectoryRequest());
	else
		return (403);
}

int	Client::handleFileRequest()
{
	string clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), R_OK) != 0)
		return (403);
	if (isCgi())
		return (handleCGI());

	std::ifstream	file(clean_path.c_str());
	if (!file.is_open())
		return (500);

	std::ostringstream	body;
	body << file.rdbuf();
	file.close();
	_request.response.body = body.str();
	_request.response.content_type = getMIME(_config->full_path);
	return (200);
}

int	Client::handleDirectoryRequest()
{
	string	URI = _config->full_path;
	if (URI.empty() || URI[URI.size() - 1] != '/')
	{
		_request.setRedirectURI(URI + "/");
		return (301);
	}
	// Chercher un fichier index
	std::string indexFile = findIndexFile();
	if (!indexFile.empty()) {
		_config->full_path = indexFile;
		return (handleFileRequest());
	}
	// Si pas de fichier index, vérifier l'autoindex
	if (_config->autoindex == AUTO_ON)
		return (generateDirectoryListing());
	else
		return (403);
}

string Client::findIndexFile()
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

int	Client::generateDirectoryListing()
{
	ostringstream body;
	string path = _config->full_path;

	body << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << _request.getURI() << "</title>\n";
	body << "<style>body { font-family: monospace; } a { text-decoration: none; }</style>\n";
	body << "</head>\n<body>\n";
	body << "<h1>Index of " << _request.getURI() << "</h1>\n<hr>\n<ul>\n";

	DIR *dir = opendir(path.c_str());
	if (!dir)
		return (500);
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		struct stat st;
		string name = entry->d_name;
		if (name == ".")
			continue ;
		string fullPath = path + name;
		string link = _request.getURI();
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
		return (500);
	body << "</ul>\n<hr>\n</body>\n</html>\n";
	_request.response.content_type = "text/html";
	_request.response.body = body.str();
	return (200);
}