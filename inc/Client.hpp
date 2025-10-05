#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

# define READ_HEADERS 101
# define READ_BODY 202
# define READ_END 303
# define CHUNKED 2

class Server;
class HTTPRequest;
class Listen;
typedef struct s_directives t_directives;

typedef struct s_cgi
{
	pid_t	pid;
	int		stdin_fd;
	int		stdout_fd;
	char**	envp;
	char*	argv[3];
	bool	is_running;
	string	buffer;
} t_cgi;

using namespace std;

class	Client
{
public:
	Client(int listen_fd, int epoll_fd, Listen* listen);
	~Client();
	
	time_t				last_activity;
	int					state;
	void				sendResponse();
	int					readData();
	int					processRequest();
	void				resetRequest();
	
private:
	int					_client_fd;
	int					_listen_fd;
	struct sockaddr_in	_client_addr;
	Server				*_server;
	Listen				*_listen;
	t_directives		*_config;
	HTTPRequest			_request;
	string				_buffer;
	bool				_keep_alive;
	string 				_writeBuffer;
	t_cgi				_cgi;
	int					_epoll_fd;

	/* Request Parsing ***************************************************************************/
	int					processBuffer();
	int	    			isRequestWellFormedOptimized();
	int					isRequestWellChunked(const map<string, string> &headers);
	void				isRedirectionNeeded();
	string				getMIME(string& URI);
	bool				URI_Not_Printable(string& URI);
	string				urlDecode(const string &str);
	size_t				parseChunked(string &data);
	string				findHeaderConnection();

	/* Response Function *************************************************************************/
	void				buildResponse(int code);
	string				getCodeMessage(int code);
	void				handleError(int code);
	string				obtainDateHeader();	
	
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
	int					isDirectoryPost();
	int					handleMultipartForm(const map<string, string>::const_iterator &header,
											const string &path);
	int					handleText(const string &path);
	string				findFileName();
	string				extractName();
	int					uploadFile(const string &filename, const string &boundary);
	int					saveData(const string &root,  const string &boundary);
	string				searchBoundary(const string &arg);
	int					isMultipartWellFormed(const string &boundary, bool isFilename);
	int					isValidPostRequest(const string &path);

	/* CGI ***************************************************************************************/
	bool				isQueryStringValid();
	bool				isValid();
	char**				buildCgiEnv();
	int					handleCGI();
	int					buildHttpResponseFromCgiOutput(const string& cgiOutput);

public:
	/* Setters ************************************************************************************/
	void				setConfig(const string &URI);
	void				setServerConfig(Server *server_config) { _server = server_config; }
	void				setListen(Listen* listen) { _listen = listen; }

	/* Getters ************************************************************************************/
	int					getClientFd() const { return _client_fd; }
	Server*				getServerConfig() const { return _server; }
	int					getListenFd() const { return _listen_fd; }
	HTTPRequest&		getRequest() { return _request; }
	bool				isKeepAliveConnection() const { return _keep_alive; }

	/* CGI ***************************************************************************************/
	bool				isCgi();
	bool				getIsCgiRunning() const { return _cgi.is_running; }
	void				processCGI(int fd);
	t_cgi&				getCgi() { return _cgi; }
};

#endif