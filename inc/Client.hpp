#pragma once

#include "webserv.hpp"

class Client {
private:
	int									socket_fd;
	std::string							method;
	std::string							URI;
	std::string							version;
	std::string							request;
	std::string							body;
	std::map<std::string, std::string>	headersMap;
	struct sockaddr_in					client_addr;
	std::string							response;

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

public:
	Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	void			start();

	// getter
	int         getSocketFd(void);
	std::string getMethod();
	std::string getURI();
	std::string getVersion();
	std::string getRequest();
};