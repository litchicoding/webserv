#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

class	Server;

typedef struct s_directives t_directives;

using namespace std;

class	Client
{
private:
	/* Client ID **********************************************************************************/
	int					_client_fd;
	int					_listen_fd;
	struct sockaddr_in	_client_addr;
	Server				*_server_config;
	t_directives		*_config;
	/* Request ************************************************************************************/
	string				_method;
	string				_URI;
	string				_version;
	string				_request;
	map<string, string>	_headersMap;
	string				_body;
	string				_response;
	string				_root;
	int					_request_len;
	size_t				_response_len;

	/* Parsing ************************************************************************************/
	void				handleError(int code);
	void				handleMethodLine(string& line);
	void				handleHeaders(string& line);
	void				handleBody(string& line);
	string				collect_request();
	void				handleGet();
	void				handlePost();
	void				handleDelete();
	string				getMIME(string& URI);
	bool				URI_Not_Printable(string& URI);
	int					HeadersCorrect(string method);

public:
	Client(int listen_fd, int epoll_fd);
	// Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	/* Member Function ****************************************************************************/
	void				start();
	void				parseRawRequest();
	void				buildResponse();

	/* Setters ************************************************************************************/
	void				setRequest(const string &request, const int &len);
	void				setServerConfig(Server *server_config);
	void				setConfig();

	/* Getters ************************************************************************************/
	int					getClientFd();
	string				getMethod();
	string				getURI();
	string				getVersion();
	string				getRequest();
	Server*				getServerConfig() const;
	string				getResponse() const;
	size_t				getResponseLen() const;
	int					getListenFd() const;
};

#endif