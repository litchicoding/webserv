#pragma once

#include "webserv.hpp"

class Client {
private:
	int									_socket_fd;
	std::string							_method;
	std::string							_URI;
	std::string							_version;
	std::string							_request;
	std::string							_body;
	std::map<std::string, std::string>	_headersMap;
	struct sockaddr_in					_client_addr;
	std::string							_response;
	std::string							_root;

	void			handleError(int code);
	void			buildResponse();
	void			parseRawRequest();
	void			handleMethodLine(std::string& line);
	void			handleHeaders(std::string& line);
	void			handleBody(std::string& line);
	std::string		collect_request();
	void			handleGet();
	void			handlePost();
	void			handleDelete();
	std::string		getMIME(std::string& URI);
	bool			URI_Not_Printable(std::string& URI);
	int				HeadersCorrect(std::string method);

public:
	Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	void			start();

	// getter
	int			getSocketFd(void);
	std::string getMethod();
	std::string getURI();
	std::string getVersion();
	std::string getRequest();
};
