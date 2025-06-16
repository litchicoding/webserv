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
	void			parseRequest();
	std::string		collect_request();

public:
	Client(int client_fd, sockaddr_in client_adrr);
	~Client();

	void			start();

	// setter
	void    setSocketFd(int set);
	void    setMethod(std::string set);
	void    setURI(std::string set);
	void    setVersion(std::string set);
	void    setRequest(std::string set);

	// getter
	int         getSocketFd(void);
	std::string getMethod();
	std::string getURI();
	std::string getVersion();
	std::string getRequest();
};