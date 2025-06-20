#include "../inc/Client.hpp"

Client::Client(int client_fd, sockaddr_in client_adrr) {
	this->socket_fd = client_fd;
	this->client_addr = client_adrr;
}

Client::~Client() {}

/******************************************************************************/
/******************************************************************************/

void Client::start() {
	parseRawRequest();
	buildResponse();
	// sendResponse();
}

/******************************************************************************/
/******************************************************************************/

void	Client::handleError(int code) {
	std::string	message, filePath;

	switch (code) {
		case 400:
		    std::cout << RED << "400 : Reussie !" << std::endl;
			// message = "400 Bad Request";
			// filePath = "";
			break;
		case 403:
		    std::cout << RED << "403 : Reussie !" << std::endl;
			break;
		case 404:
	        std::cout << RED << "404 : Reussie !" << std::endl;
			break;
		case 405:
	        std::cout << RED << "405 : Reussie !" << std::endl;
			break;
		case 500:
	        std::cout << RED << "500 : Reussie !" << std::endl;
			break;
		case 505:
	        std::cout << RED << "505 : Reussie !" << std::endl;
			break;
	}
	// std::ifstream 		file(filePath);
	// if (!file.is_open()) {
	// 	std::cerr << "In handleError() file not open.";
	// 	return ;
	// }
	// std::ostringstream	body;
	// body << file.rdbuf();

	// this->response = "HTTP/1.1 " + message + "\r\n";
	// this->response += "Content-Type: text/html\r\n";

	// this->response += "Content-Length: " + std::to_string(body.str().size()) + "\r\n";
	// this->response += "\r\n";
	// this->response += body.str();
	// Rajouter d'autres headers ?? Date ? Server ? Connection ?z
}


void	Client::buildResponse() {
	if (method == "GET")
		handleGet();
	else if (method == "POST")
		handlePost();
	else if (method == "DELETE") {
		handleDelete();
	}
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
			handleBody(line);
	}

	// Pour tester !!!
	std::cout << GREEN "Method = " << this->method << std::endl;
	std::cout << "URI = " << this->URI << std::endl;
	std::cout << "version = " << this->version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = this->headersMap.begin(); it != this->headersMap.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body : \n" << this->body << RESET << std::endl;
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
