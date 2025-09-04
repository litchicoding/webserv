#include "../inc/Client.hpp"

int	Client::handlePost()
{
	string clean_path, message, URI;
	map<string, string>::const_iterator header;
	ostringstream response;
	
	clean_path = urlDecode(_config->full_path);
	if (isValidPostRequest(clean_path) != OK)
		return (_request.code);
	if (isCgi())
		return (handleCGI());
	/*** 1. Vérifications: ***/
	// Content-Type= multipart/form-data
	map<string, string>	headerMap = _request.getHeaders();
	header = headerMap.find("Content-Type");
	if (header == headerMap.end() || header->second.empty())
		return (400);
	else if (header->second.find("application/x-www-form-urlencoded") != string::npos)
		return (handleCGI());
	else if (header->second.find("multipart/form-data") != string::npos)
		return (handleMultipartForm(header, clean_path));
	else if (header->second.find("text/plain") != string::npos)
		return (handleText(clean_path));
	else
		return (415);
}

int	Client::handleMultipartForm(const map<string, string>::const_iterator &header, const string &path)
{
	string filename, boundary;
	
	boundary = searchBoundary(header->second);
	string	body(_request.getBody().begin(), _request.getBody().end());
	if (boundary.size() <= 0 || body.find(boundary) == string::npos)
		return (400);
	// Cas 1 : header-body = filename -> upload de fichier
	// Cas 2 : != filename donc diviser par clé-valeur (ex: name=name=Yannick, name=message=bonjour)
	filename = urlDecode(findFileName());
	if (filename.size() > 0) {
		if (uploadFile(path + "/" + filename, boundary) != OK)
			return (500);
	}
	else {
		filename = extractName() + ".txt";
		if (saveData(path + "/" + filename, boundary) != OK)
			return (500);
	}
	filename = _request.getURI() + "/" + filename;
	_request.response.body = "File creation succeeded. Location : " + filename + "\n";
	_request.response.content_type = "text/plain";
	_request.response.location = filename;
	return (201);
}

int	Client::handleText(const string &path)
{
	ostringstream oss;
	string filename;
	static int count = 0;

	oss << ++count;
	filename = path + "/upload_" + oss.str() + ".txt";
	ofstream file(filename.c_str(), ofstream::out | ofstream::binary);
	if (!file.is_open())
		return (500);
	const vector<char> &body = _request.getBody();
	for (size_t i = 0; i < _request.getBodyLen(); i++)
	{
		file.write(&body[i], 1);
	}
	file.close();
	filename = _request.getURI() + "/upload_" + oss.str() + ".txt";
	_request.response.body = "File creation succeeded. Location : " + filename + "\n";
	_request.response.body = "File creation succeeded\n";
	_request.response.content_type = "text/plain";
	_request.response.location = filename;
	return (201);
}

int	Client::saveData(const string &path, const string &boundary)
{
	size_t	pos, start, end;
	string	key, value, target;

	pos = 0;
	target = "name=\"";
	string	body(_request.getBody().begin(), _request.getBody().end());
	ofstream file(path.c_str(), ofstream::out | ofstream::binary);
	if (!file.is_open())
		return (ERROR);
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
	return (OK);
}

int	Client::uploadFile(const string &filename, const string &boundary)
{
	const vector<char>&	body_original = _request.getBody();
	string		body(body_original.begin(), body_original.end());
	string		start_boundary, end_boundary;
	ofstream	file;
	size_t		pos, start, end;

	file.open(filename.c_str(), ofstream::out | ofstream::binary);
	if (!file.is_open())
		return (ERROR);
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
	return (OK);
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
	filename = body.substr(start, end - start);
	return (filename);
}

int    Client::isDirectoryPost()
{	
	string URI = _request.getURI();
   	if (URI.empty() || URI[URI.size() - 1] != '/')
	{
		_request.setRedirectURI(_request.getURI() + "/");
		_request.code = 301;
		return (OK);
	}
	std::string indexFile = findIndexFile();
	if (!indexFile.empty())
	{
	    _request.setURI(indexFile);
		// if (isCgi())
		// 	handleCGI();
		// else
		// 	_request.setCode(403); POURQUOI 403 ????????????????????????????
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
		// if (isCgi()) {
		// 	handleCGI();
		// 	return (OK);
		// }
		// _request.code = 403;
		// return (ERROR); // POURQUOI RENVOYER 403 ?????????? C'EST PAS UNE ERREUR DE PAS ETRE UN CGI SI ?????????
		return (OK);
	}
	else if (S_ISDIR(st.st_mode))
		return (isDirectoryPost());
	else
		_request.code = 403;
	return (ERROR);
}