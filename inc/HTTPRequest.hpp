#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "webserv.hpp"

using namespace std;

class HTTPRequest
{
private:
	string						_method;
	string						_URI;
	string						_RedirectURI;
	string						_version;
	size_t						_expected_body_len;
	bool						_headers_ended;
	bool						_body_ended;
	bool						_chunked;
	vector<char>				_body;
	map<string, string>			_headers;


public:
	HTTPRequest();
	~HTTPRequest();

	size_t						code;
	string						response;

	/* Parsing ***********************************************************************************/
	int							parsingHeaders(const string &data);
	int							handleMethodLine(string &line);
	void						appendBodyData(const char *data, size_t len);
	void						resetRequest();
	
	/* Getters ***********************************************************************************/
	bool						isBodyEnded();
	bool						isChunked() const { return _chunked; }
	bool						areHeadersEnded() const { return _headers_ended; }
	size_t						getExpectedBodyLen() const { return _expected_body_len; }
	size_t						getBodyLen() const { return _body.size(); }
	string&						getMethod() { return _method; }
	string&						getRedirectURI() { return _RedirectURI; }
	string&						getURI() { return _URI; }
	const string&				getVersion() const { return _version; }
	const map<string, string>&	getHeaders() const { return _headers; }
	const vector<char>&			getBody() const { return _body; }
	void						setURI(const string& URI);
	void						setRedirectURI(const string& URI);
	void						setCode(int num);
};

/* Operator Overload *****************************************************************************/
std::ostream&	operator<<(std::ostream &os, HTTPRequest &src);

#endif