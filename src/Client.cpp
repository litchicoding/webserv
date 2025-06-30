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
}

/**************************************************************************************************/
/* Parsing ****************************************************************************************/

/**************************************************************************************************/
/* Member Fucntions *******************************************************************************/

/**************************************************************************************************/
/* Getters ****************************************************************************************/

int	Client::getClientFd() { return _client_fd; }