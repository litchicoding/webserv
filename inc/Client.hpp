#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

# define READ_HEADERS 101
# define READ_BODY 202
# define READ_END 303
# define CHUNKED 2

class Server;
class HTTPRequest;
typedef struct s_directives t_directives;

using namespace std;

class	Client
{
public:
	Client(int listen_fd, int epoll_fd);
	~Client();
	
	int					state;
	int					readData();
	int					processRequest();
	void				sendResponse();
	void				resetRequest();
	
private:
	int					_client_fd;
	int					_listen_fd;
	struct sockaddr_in	_client_addr;
	Server				*_server;
	t_directives		*_config;
	HTTPRequest			_request;
	string				_buffer;
	bool				_keep_alive;
	// string				_current_redirection;

	/* Request Parsing ***************************************************************************/
	int					processBuffer();
	void				isRedirectionNeeded();
	int	    			isRequestWellFormedOptimized();
	int					isRequestWellChunked(const map<string, string> &headers);
	size_t				parseChunked(string &data);
	string				getMIME(string& URI);
	bool				URI_Not_Printable(string& URI);
	string				urlDecode(const string &str);

	/* Response Function *************************************************************************/
	void				buildResponse(int code);
	string				getCodeMessage(int code);
	void				handleError(int code);
	
	/* Delete Method *****************************************************************************/
	int					handleDelete();
	int					isFileDelete();
	int					isDirectoryDelete();
	int					delete_all_folder_content(string URI);
	
	/* Get Method ********************************************************************************/
	int					handleGet();
	int					handleFileRequest();
	int					handleDirectoryRequest();
	string				findIndexFile();
	int					generateDirectoryListing();

	/* Post Method *******************************************************************************/
	int					handlePost();
	void				isFilePost();
	int					isDirectoryPost();
	int					handleMultipartForm(const map<string, string>::const_iterator &header,
											const string &path);
	int					handleText(const string &path);
	string				findFileName();
	string				extractName();
	int					uploadFile(const string &filename, const string &boundary);
	int					saveData(const string &root,  const string &boundary);
	string				searchBoundary(const string &arg);
	int					isValidPostRequest(const string &path);

	/* CGI ***************************************************************************************/
	bool				isCgi();
	bool				isQueryStringValid();
	bool				isValid();
	char**				buildCgiEnv();
	int					handleCGI();
	int					buildHttpResponseFromCgiOutput(const string& cgiOutput);

public:
	/* Setters ************************************************************************************/
	void				setConfig(const string &URI);
	void				setServerConfig(Server *server_config) { _server = server_config; }

	/* Getters ************************************************************************************/
	int					getClientFd() const { return _client_fd; }
	Server*				getServerConfig() const { return _server; }
	int					getListenFd() const { return _listen_fd; }
	bool				isKeepAliveConnection() const { return _keep_alive; }
	HTTPRequest&		getRequest() { return _request; }
};

#endif