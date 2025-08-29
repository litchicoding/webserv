#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

# define READ_HEADERS 101
# define READ_BODY 202
# define READ_END 303

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
	int					readData(int epoll_fd);
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

	/* Request Parsing ***************************************************************************/
	int					processBuffer();
	int	    			isRequestWellFormedOptimized();
	int					isRequestWellChunked(const map<string, string> &headers);
	string				getMIME(string& URI);
	bool				URI_Not_Printable(string& URI);
	string				urlDecode(const string &str);

	/* Response Function *************************************************************************/
	void				buildResponse(int code);
	void				handleError(int code);
	void				sendRedirect(const string &URI);
	
	/* Delete Method *****************************************************************************/
	void				handleDelete();
	void				isFileDelete();
	void				isDirectoryDelete();
	int					delete_all_folder_content(string URI);
	
	/* Get Method ********************************************************************************/
	void				handleGet();
	void				handleFileRequest();
	void				handleDirectoryRequest();
	string				findIndexFile();
	void				generateDirectoryListing();

	/* Post Method *******************************************************************************/
	void				handlePost();
	void				isFilePost();
	// void				isDirectoryPost();
	string				findFileName();
	// void				uploadFile(const string &filename);
	void				uploadFile(const string &filename, const string &boundary);
	void				saveData(const string &root,  const string &boundary);
	string				searchBoundary(const string &arg);

	/* CGI ***************************************************************************************/
	bool				isCgi();
	bool				isQueryStringValid();
	bool				isValid();
	char**				buildCgiEnv();
	void				handleCGI();
	void				buildHttpResponseFromCgiOutput(const string& cgiOutput);

public:
	/* Setters ************************************************************************************/
	void				setConfig(const string &URI);
	void				setServerConfig(Server *server_config) { _server = server_config; }

	/* Getters ************************************************************************************/
	int					getClientFd() const { return _client_fd; }
	Server*				getServerConfig() const { return _server; }
	int					getListenFd() const { return _listen_fd; }
	HTTPRequest&		getRequest() { return _request; }
};

#endif