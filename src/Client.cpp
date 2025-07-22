#include "Client.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Client::Client(int listen_fd, int epoll_fd) : _listen_fd(listen_fd), _server_config(NULL), _config(NULL)
{
	std::cout << GREEN << "*** Client Construction ***" << RESET << std::endl;
	socklen_t	client_addr_len = sizeof(_client_addr);
	_client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&_client_addr), &client_addr_len);
	if (_client_fd == INVALID) {
		perror("accept");
		return ;
	}
	add_fd_to_epoll(epoll_fd, _client_fd);
}

Client::~Client()
{
	std::cout << GREEN << "*** Client Deconstruction ***" << RESET << std::endl;
	// must delete everything needed
	close(_client_fd);
}

/**************************************************************************************************/
/* Parsing ****************************************************************************************/

void	Client::handleError(int code) {
	std::string	message, filePath;

	switch (code) {
		case 400:
		    std::cout << RED << "400 : Error !" RESET << std::endl;
			// message = "400 Bad Request";
			// filePath = "";
			break;
		case 403:
		    std::cout << RED << "403 : Error !" RESET << std::endl;
			break;
		case 404:
	        std::cout << RED << "404 : Error !" RESET << std::endl;
			break;
		case 405:
	        std::cout << RED << "405 : Error !" RESET << std::endl;
			break;
		case 500:
	        std::cout << RED << "500 : Error !" RESET << std::endl;
			break;
		case 505:
	        std::cout << RED << "505 : Error !" RESET << std::endl;
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

void	Client::start()
{
	if (_method == "GET")
		handleGet();
	if (_method == "POST")
		handlePost();
	if (_method == "DELETE")
		handleDelete();
}

void	Client::parseRawRequest() {
	std::istringstream  iss(this->_request);
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
	std::cout << GREEN "Method = " << this->_method << std::endl;
	std::cout << "URI = " << this->_URI << std::endl;
	std::cout << "version = " << this->_version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = this->_headersMap.begin(); it != this->_headersMap.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body : \n" << this->_body << RESET << std::endl;
}


/**************************************************************************************************/
/* Member Fucntions *******************************************************************************/

void	Client::setRequest(const std::string &request, const int &len)
{
	_request = request;
	_request_len = len;
}

void	Client::setServerConfig(Server *server_config) { _server_config = server_config; }

void	Client::setConfig()
{
	if (_server_config == NULL)
		return ;
	std::cout << RED << "URI : " << _URI << RESET << std::endl;
	_config = _server_config->searchLocationMatch(_URI);
	if (_config == NULL)
		std::cout << RED << "Error: setServerConfig(): no match with URI found" << RESET << std::endl;
}

/**************************************************************************************************/
/* Getters ****************************************************************************************/

int	Client::getClientFd() { return _client_fd; }

Server*	Client::getServerConfig() const { return _server_config; }

std::string Client::getMethod() { return this->_method; }

std::string Client::getURI() { return this->_URI; }

std::string Client::getVersion() { return this->_version; }

std::string Client::getRequest() { return this->_request; }

std::string	Client::getResponse() const { return _response; }

size_t	Client::getResponseLen() const { return _response_len; }

int	Client::getListenFd() const { return _listen_fd; }