#include "../inc/Client.hpp"

int	Client::handleDelete()
{
	struct stat st;
	string clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), F_OK) != 0)
		return (404);
	if (stat(clean_path.c_str(), &st) != 0)
		return (500);
	if (access(clean_path.c_str(), W_OK) != 0)
		return (403);
	if (S_ISREG(st.st_mode))
		return (isFileDelete());
	else if (S_ISDIR(st.st_mode))
		return (isDirectoryDelete());
	else
		return (403);
}

int	Client::isFileDelete()
{
	string clean_path = urlDecode(_config->full_path);
	if (std::remove(clean_path.c_str()) != OK)
		return (500);
	return (204);
}

int	Client::isDirectoryDelete()
{
	string URI = _request.getURI();
	if (URI.empty() || URI[URI.size() - 1] != '/')
		return (409);
	string clean_path = urlDecode(_config->full_path);
	if (delete_all_folder_content(clean_path) != OK) {
		if (access(clean_path.c_str(), W_OK) != OK)
			return (403);
		return (500);
	}
	if (remove(clean_path.c_str()) != OK)
		return (500);
	return (204);
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
		// cout << "exemple : " << fullpath.c_str() << endl;
		if (remove(fullpath.c_str()) != OK)
		{
			closedir(dir);
			return ERROR;
		}
	}
	closedir(dir);
	return OK;
}