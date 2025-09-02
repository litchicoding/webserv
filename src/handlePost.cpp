#include "../inc/Client.hpp"

void	Client::handlePost()
{
	string clean_path, filename, message, boundary, URI;
	map<string, string>::const_iterator header;
	ostringstream response;
	
	clean_path = urlDecode(_config->full_path);
	if (access(clean_path.c_str(), F_OK) != 0) {
		_request.code = 404;
		return ;
	}
	if (access(clean_path.c_str(), W_OK) != 0) {
		_request.code = 403;
		return ;
	}
	/*** 1. Vérifications: ***/ 
	// Content-Type= multipart/form-data
	map<string, string>	headerMap = _request.getHeaders();
	if (header == headerMap.end()) {
		_request.code = 400;
		return ;
	}
	if (header->second.find("multipart/form-data") != string::npos || handleMultipartForm(clean_path) != OK)
		return ;
	else if (header->second.find("application/x-www-form-urlencoded") != string::npos || handleEncodedForm(clean_path) != OK)
		return ;
	else {
		_request.code = 415;
		return ;
	}
	// Boundary is here and correct
	boundary = searchBoundary(header->second);
	string	body(_request.getBody().begin(), _request.getBody().end());
	if (boundary.size() <= 0 || body.find(boundary) == string::npos) {
		_request.code = 400;
		return ;
	}
	/*** 2. Parsing: ***/
	// Lire boundary et découper le body.
	// Cas 1 : header-body = filename -> upload de fichier
	// Cas 2 : != filename donc diviser par clé-valeur (ex: name=name=Yannick, name=message=bonjour)
	filename = urlDecode(findFileName());
	if (filename.size() > 0)
		uploadFile(clean_path + "/" + filename, boundary);
	else {
		filename = extractName() + ".txt";
		saveData(clean_path + "/" + filename, boundary);
	}
	/*** 3.Response HTTP ***/
	URI = _request.getURI();
	message = "File creation succeeded\n";
	response << "HTTP/1.1 201 Created\r\n";
	response << "Location: " << URI;
	if (!filename.empty())
		response << ( (URI[URI.size() - 1] == '/') ? "" : "/" ) << filename + "\r\n";
	response << "Content-Type: text/plain\r\n";
	response << "Content-Length: " << message.size() << "\r\n";
	header = _request.getHeaders().find("Connection");
	if (header != _request.getHeaders().end() && header->second.find("keep-alive") != string::npos)
		response << "Connection: keep-alive\r\n";
	else
		response << "Connection: close\r\n";
	response << "\r\n";
	response << message;
	_request.response = response.str();
}

void	Client::saveData(const string &root, const string &boundary)
{
	size_t	pos, start, end;
	string	key, value, target;

	pos = 0;
	target = "name=\"";
	string	body(_request.getBody().begin(), _request.getBody().end());
	ofstream file(root.c_str(), ofstream::out);
	if (!file.is_open()) {
		_request.code = 500;
		return ;
	}
	while (true)
	{
		// header = key
		pos = body.find(target, pos);
		if (pos == string::npos)
			break ;
		start = pos + target.size();
		end = body.find("\"", start);
		if (end == string::npos)
			break ;
		key = body.substr(start, end - start);
		// \n
		pos = body.find("\r\n\r\n", end);
		if (pos == string::npos)
			break ;
		// data = value
		start = pos + 4;
		end = body.find("--" + boundary, pos);
		if (end == string::npos)
			break ;
		end -= 1;
		value = body.substr(start, end - start);
		// boundary
		pos = end + 2 + boundary.size();
		file << key + " = " + value;
	}
	file.close();
}

void	Client::uploadFile(const string &filename, const string &boundary)
{
	const vector<char>&	body_original = _request.getBody();
	string		body(body_original.begin(), body_original.end());
	string		start_boundary, end_boundary;
	ofstream	file;
	size_t		pos, start, end;

	file.open(filename.c_str(), ofstream::out | ofstream::binary);
	if (!file.is_open()) {
		_request.code = 500;
		return ;
	}
	start_boundary = "--" + boundary;
	end_boundary = "--" + boundary + "--";
	pos = 0;
	while (true)
	{
		pos = body.find(start_boundary, pos);
		if (pos == string::npos)
			break ;
		pos = body.find("\r\n\r\n", (pos + start_boundary.length()));
		if (pos == string::npos)
			break ;
		start = pos + 4;
		pos = body.find("\r\n--" + boundary, start);
		if (pos != string::npos)
			end = pos;
		else {
			pos = body.find("\r\n" + end_boundary, start);
			if (pos != string::npos) {
				end = pos;
				end -= 2;
			}
			else
				end = body_original.size();
		}
		file.write(&body_original[start], end - start);
		pos = end + 2 + boundary.length();
	}
	file.close();
}

string	Client::searchBoundary(const string &arg)
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
	boundary = arg.substr(start, (arg.length() - 1) - start);
	return (boundary);
}

string	Client::findFileName()
{
	string	body(_request.getBody().begin(), _request.getBody().end());
	string	filename, target;
	size_t	start, end;

	start = body.find("filename=\"");
	if (start == string::npos)
		return ("");
	target = "filename=\"";
	if (body.compare(start, target.size(), target))
		return ("");
	start = start + target.size();
	end = start;
	while (body[end] != '\0' && body[end] != 34)
		end++;
	filename = body.substr(start, end - start);
	return (filename);
}

string	Client::extractName()
{
	string	body(_request.getBody().begin(), _request.getBody().end());
	string	filename, target;
	size_t	start, end;

	target = "\r\n\r\n";
	start = body.find("\r\n\r\n");
	if (start == string::npos)
		return ("");
	start += target.length();
	end = body.find("--", start);
	if (end == string::npos)
		return ("");
	end -= 2;
	cout << YELLOW << body[end] << endl;
	filename = body.substr(start, end - start);
	return (filename);
}

int    Client::isDirectoryPost()
{	
	string URI = _request.getURI();
   	if (URI.empty() || URI[URI.size() - 1] != '/')
	{
		_request.setRedirectURI(URI + "/");
		_request.setCode(301);
		return (OK);
	}
	std::string indexFile = findIndexFile();
	if (!indexFile.empty())
	{
	    _request.setURI(indexFile);
		if (isCgi())
			handleCGI();
		else
			_request.setCode(403);
	    return (OK);
	}
	else {
		_request.code = 403;
		return (ERROR);
	}
}
int	Client::isValidPostRequest(const string &path)
{
	struct stat st;

	if (access(path.c_str(), F_OK) != 0) {
		_request.code = 404;
		return (ERROR);
	}
	if (access(path.c_str(), W_OK) != 0) {
		_request.code = 403;
		return (ERROR);
	}
	if (stat(path.c_str(), &st) != 0) {
		_request.code = 500;
		return (ERROR);
	}
	if (S_ISREG(st.st_mode))
	{
		if (isCgi())
			handleCGI();
		else
			_request.setCode(403);
		return (ERROR);
	}
	else if (S_ISDIR(st.st_mode)) {
		if (isDirectoryPost() != OK)
			return (ERROR);
	}
	else {
		_request.code = 403;
		return (ERROR);
	}
	return (ERROR);
}