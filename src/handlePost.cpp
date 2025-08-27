#include "../inc/Client.hpp"

void	Client::handlePost()
{
	string clean_path, filename, message, boundary;
	int	content_length;
	map<string, string>::iterator header;
	ostringstream response;

	clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), F_OK) != 0)
		return (handleError(404));
	if (access(clean_path.c_str(), W_OK) != 0)
		return (handleError(403));
	/*** 1. Vérifications: ***/ 
	// Content-Type= multipart/form-data
	header = _headersMap.find("Content-Type");
	if (header == _headersMap.end() || header->second.find("multipart/form-data") == string::npos)
		return (handleError(415));
	// Boundary is here and correct
	boundary = searchBoundary(header->second);
	if (boundary.size() <= 0 || _body.find(boundary) == string::npos)
		return (handleError(400));
	// Content-Length coherent
	header = _headersMap.find("Content-Length");
	if (header == _headersMap.end() && _headersMap.find("Transfer-Encoding") == _headersMap.end())
		return (handleError(400));
	content_length = atoi(header->second.c_str());
	/*** 2. Parsing: ***/
	// Lire boundary et découper le body.
	// Cas 1 : header-body = filename -> upload de fichier
	// Cas 2 : != filename donc diviser par clé-valeur (ex: name=name=Yannick, name=message=bonjour)
	filename =urlDecode(findFileName());
	if (filename.size() > 0)
		uploadFile(clean_path + "/" + filename, content_length);
	else
		saveData(clean_path + "/data.txt", boundary, content_length);
	/*** 3.Response HTTP ***/
	message = "File creation succeeded\n";
	response << "HTTP/1.1 201 Created\r\n";
	// response << "Location: " + root + "/" + filename + "\r\n";
	response << "Location: " << _URI;
	if (!filename.empty())
		response << ( (_URI[_URI.size() - 1] == '/') ? "" : "/" ) << filename;
	response << "Content-Type: text/plain\r\n";
	response << "Content-Length: " << message.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << message;
	_response = response.str();
	_response_len = _response.size();
}

void	Client::saveData(const string &root, const string &boundary, int size)
{
	size_t	pos, start, end, count;
	string	key, value, target;

	pos = 0;
	count = 0;
	target = "name=\"";
	ofstream file(root.c_str(), ofstream::out);
	if (!file.is_open())
		return (handleError(500));
	while (true)
	{
		// header = key
		pos = _body.find(target, pos);
		if (pos == string::npos)
			break ;
		start = pos + target.size();
		end = _body.find("\"", start);
		if (end == string::npos)
			break ;
		key = _body.substr(start, end - start);
		// \n
		pos = _body.find("\n", end);
		if (pos == string::npos)
			break ;
		// data = value
		start = pos + 1;
		end = _body.find("--" + boundary, pos);
		if (end == string::npos)
			break ;
		end -= 1;
		value = _body.substr(start, end - start);
		// boundary
		pos = end + 2 + boundary.size();
		count += key.size() + value.size();
		file << key + " = " + value + "\n";
	}
	if (count != (size_t)size)
		return (handleError(400));
	file.close();
}

void	Client::uploadFile(const string &filename, int size)
{
	size_t	start, end, pos;

	ofstream file(filename.c_str(), ofstream::out);
	if (!file.is_open())
		return (handleError(500));
	// Copy file and skip boundaries + headers
	pos = _body.find("Content-Type");
	if (pos == string::npos)
		return ;
	start = _body.find("\n", pos);
	if (start == string::npos)
		return ;
	start += 1;
	end = _body.find("--", start);
	if (end == string::npos)
		return ;
	end -= 1;
	if ((size_t)size != end - start)
		return (handleError(400));
	file << _body.substr(start, end - start);
	file.close();
}

string	Client::searchBoundary(string &arg)
{
	string	boundary;
	size_t	start;

	start = arg.find("boundary=");
	if (start == string::npos)
		return ("");
	start = arg.find("=", start);
	if (start == string::npos)
		return ("");
	start++;
	boundary = arg.substr(start, arg.length() - start);
	return (boundary);
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

void    Client::isDirectoryPost()
{
   	if (_URI.empty() || _URI[_URI.size() - 1] != '/')
	{
		std::string redirectUri = _URI + "/";
		return ;
	}
	std::string indexFile = findIndexFile();
	if (!indexFile.empty())
	{
	    _URI = indexFile;
	    return ;
	}
	else
		return (handleError(403));
}