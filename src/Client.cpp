#include "../inc/Client.hpp"

Client::Client(int client_fd, sockaddr_in client_adrr) {
	this->socket_fd = client_fd;
	this->client_addr = client_adrr;
	this->request = "";
	this->method = "";
	this->URI = "";
	this->version = "";
	this->body = "";
}

Client::~Client() {}

/******************************************************************************/
/******************************************************************************/

void Client::start() {
	try {
		parseRawRequest(); // recupere et initialise le tout 
		buildResponse();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

/******************************************************************************/
/******************************************************************************/

std::string	Client::collect_request() {
	std::string		newRequest;
	const size_t	buffer_size = 1024;
	char			buffer[buffer_size];

	while (true) {
		std::memset(buffer, 0, buffer_size);
		ssize_t bytes_read = recv(this->getSocketFd(), buffer, buffer_size - 1, 0);
		if (bytes_read < 0) {
			throw std::runtime_error("Error: ..."); // ERROR numm ???
		}
		else if (bytes_read == 0)
			break ;
		else
			newRequest.append(buffer, bytes_read);
	}
	return newRequest;
}

void	Client::handleHeaders(std::string& line)
{
	size_t delimiterPos = line.find(':');
	if (delimiterPos != std::string::npos) {
		std::string key = line.substr(0, delimiterPos);
		if (key.empty())
			throw std::runtime_error("Error 400: Header without key");
		std::string value = line.substr(delimiterPos + 1);
		this->headersMap[key] = value;
	}
	else
		throw std::runtime_error("Error 400: Malformed header");
}

void	Client::handleMethodLine(std::string& line)
{
	std::istringstream  iss(line);
	iss >> this->method >> this->URI >> this->version;

	if (method != "GET" && method != "POST" && method != "DELETE") {
		handleError(405);
		return ;
	}
	if (URI.empty() || URI[0] != '/') {
		handleError(400);
		return ;
	}
	if (URI.find("..") != std::string::npos) {
		handleError(403);
		return ;
	}
	if (version != "HTTP/1.1") {
		handleError(505);
		return ;
	}
	// rajouter si fichier sors du root du config ?? 403
	// rajouter verifications des headers
	// verification fichier si existe et si on peux le lire ! 404 403

}

void	Client::parseRawRequest() {
	this->request = collect_request();
	std::istringstream  iss(this->request);
	std::string line;
	bool isFirstLine = true;
	bool headers = true;

	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1, 1);
		if (isFirstLine)
		{
			handleMethodLine(line);
			isFirstLine = false;
		}
		else if (line.empty())
			headers = false;
		else if (headers)
			handleHeaders(line);
		else
			this->body.append(line + "\n");
	}

	// Pour tester !!!
	// std::cout << "Method = " << this->method << std::endl;
	// std::cout << "URI = " << this->URI << std::endl;
	// std::cout << "version = " << this->version << std::endl;
	// std::cout << "request = \n" << this->request << std::endl;
	// std::cout << "Headers:" << std::endl;
	// for (std::map<std::string, std::string>::const_iterator it = this->headersMap.begin(); it != this->headersMap.end(); ++it) {
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// }
	// std::cout << "Body : \n" << this->body << std::endl;
}

























/***************************************************************/
/**                      setter                               **/
/***************************************************************/

void    Client::setSocketFd(int set) {
	this->socket_fd = set;
}
void    Client::setMethod(std::string set) {
	this->method = set;
}
void    Client::setURI(std::string set) {
	this->URI = set;
}
void    Client::setVersion(std::string set) {
	this->version = set;
}
void    Client::setRequest(std::string set) {
	this->request = set;
}

/***************************************************************/
/**                      getter                               **/
/***************************************************************/

int	Client::getSocketFd() {
	return this->socket_fd;
}

std::string Client::getMethod() {
	return this->method;
}

std::string Client::getURI() {
	return this->URI;
}

std::string Client::getVersion() {
	return this->version;
}

std::string Client::getRequest() {
	return this->request;
}
