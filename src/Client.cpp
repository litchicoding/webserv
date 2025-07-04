#include "Client.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Client::Client(int listen_fd, int epoll_fd)
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

/**************************************************************************************************/
/* Member Fucntions *******************************************************************************/

void	Client::setRequest(const std::string &request, const int &len)
{
	_request = request;
	_request_len = len;
}

void	Client::setServerConfig(Server *server_config) { _server_config = server_config; }

/**************************************************************************************************/
/* Getters ****************************************************************************************/

int	Client::getClientFd() { return _client_fd; }

Server*	Client::getServerConfig() const { return _server_config; }