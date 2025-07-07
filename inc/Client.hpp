#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserv.hpp"

class	Server;

class	Client
{
private:
	/* Client ID **********************************************************************************/
	int									_client_fd;
	struct sockaddr_in					_client_addr;
	Server								*_server_config;
	t_directives						*_config;
	/* Request ************************************************************************************/
	std::string							_method;
	std::string							_URI;
	std::string							_version;
	std::string							_request;
	std::map<std::string, std::string>	_headersMap;
	std::string							_body;
	std::string							_response;
	std::string							_root;
	int									_request_len;
	size_t								_response_len;

	/* Parsing ************************************************************************************/
	void								handleError(int code);
	void								handleMethodLine(std::string& line);
	void								handleHeaders(std::string& line);
	void								handleBody(std::string& line);
	std::string							collect_request();
	void								handleGet();
	void								handlePost();
	void								handleDelete();
	std::string							getMIME(std::string& URI);
	bool								URI_Not_Printable(std::string& URI);
	int									HeadersCorrect(std::string method);

public:
	Client(int listen_fd, int epoll_fd);
	// Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	/* Member Function ****************************************************************************/
	void								start();
	void								parseRawRequest();
	void								buildResponse();

	/* Setters ************************************************************************************/
	void								setRequest(const std::string &request, const int &len);
	void								setServerConfig(Server *server_config);
	void								setConfig();

	/* Getters ************************************************************************************/
	int									getClientFd();
	std::string							getMethod();
	std::string							getURI();
	std::string							getVersion();
	std::string							getRequest();
	Server*								getServerConfig() const;
	std::string							getResponse() const;
	size_t								getResponseLen() const;
};

#endif