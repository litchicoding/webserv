#include "webserv.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

HTTPRequest::HTTPRequest() 
: _expected_body_len(0), _headers_ended(false), _body_ended(false), _chunked(false), code(0)
{
	// cout << GREEN << "***   New Request   ***" << RESET << endl;
}

HTTPRequest::~HTTPRequest()
{
	_body.clear();
	_headers.clear();
}

/**************************************************************************************************/
/* Parsing ****************************************************************************************/

int	HTTPRequest::parsingHeaders(const string &data)
{
	istringstream	stream(data);
	string			line, key, value;
	size_t			delimiter;
	set<string>		headers_seen;
	set<string>		critical_headers;

	critical_headers.insert("Host");
	critical_headers.insert("Content-Length");
	critical_headers.insert("Transfer-Encoding");
	critical_headers.insert("Authorization");
	
	if (getline(stream, line))
		handleMethodLine(line);
	while (getline(stream, line) && !line.empty() && line != "\r")
	{
		delimiter = line.find(':');
		if (delimiter != string::npos) {
			key = line.substr(0, delimiter);
			value = line.substr(delimiter + 1);
			// Trim whitespace
			key.erase(0, key.find_first_not_of(" \t\r\n"));
			key.erase(key.find_last_not_of(" \t\r\n") + 1);
			value.erase(0, value.find_first_not_of(" \t\r\n"));
			value.erase(value.find_last_not_of(" \t\r\n") + 1);
			if (key.empty() || value.empty()) {
				code = 400;
				return (OK);
			}
			cout << BLUE << key << RESET << endl;
			// Doublon detected
			if (critical_headers.find(key) != critical_headers.end())
			{
				cout << RED << key << RESET << endl;
				if (headers_seen.find(key) != headers_seen.end())
				{
					cout << "ABBBBBBBBBBBBBBBBBBBBBBB\n";
					code = 400;
					return (OK);
				}
				headers_seen.insert(key);
			}

			// Store data in headers map
			_headers[key] = value;
		}
	}
	map<string, string>::iterator header = _headers.find("Content-Length");
	if (header != _headers.end() && !header->second.empty())
		_expected_body_len = atoll(header->second.c_str());
	header = _headers.find("Transfer-Encoding");
	if (header != _headers.end())
		_chunked = true;
	_headers_ended = true;
	return (OK);
}

int	HTTPRequest::handleMethodLine(std::string& line)
{
	std::istringstream  stream(line);
	if (!(stream >> this->_method >> this->_URI >> this->_version)) {
		code = 400;
		return (ERROR);
	}
	return OK;
}

void	HTTPRequest::appendBodyData(const char *data, size_t len)
{
	_body.insert(_body.end(), data, data + len);
}

bool	HTTPRequest::isBodyEnded()
{
	if (_body.size() >= _expected_body_len)
		_body_ended = true;
	return (_body_ended);
}

void	HTTPRequest::resetRequest()
{
	code = 0;
	response.body.clear();
	response.location.clear();
	response.content_type.clear();
	response.res.clear();
	_method.clear();
	_URI.clear();
	_RedirectURI.clear();
	_version.clear();
	_expected_body_len = 0;
	_headers_ended = false;
	_body_ended = false;
	_chunked = false;
	_body.clear();
	_headers.clear();
}

/* Operator Overload *****************************************************************************/

std::ostream&	operator<<(std::ostream &os, HTTPRequest &src)
{
	map<string, string>::const_iterator	headers;
	vector<char>::const_iterator		body;

	os << endl;
	os << src.getMethod() << " " << src.getURI() << " " << src.getVersion() << endl;
	headers = src.getHeaders().begin();
	for (map<string, string>::const_iterator headers = src.getHeaders().begin();
		headers != src.getHeaders().end(); headers++)
	{
		os << headers->first << " : [" << headers->second << "]" << endl; 
	}
	for (vector<char>::const_iterator body = src.getBody().begin();
		body != src.getBody().end(); body++)
	{
		os << *body;
	}
	os << endl;
	return (os);
}

/* Setter *****************************************************************************/

void	HTTPRequest::setURI(const string& URI)
{
	this->_URI = URI;
}

void	HTTPRequest::setCode(int num)
{
	this->code = num;
}

void	HTTPRequest::setRedirectURI(const string& URI)
{
	this->_RedirectURI = URI;
}