#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <unistd.h>
# include <netinet/in.h> // sockaddr_in
# include <sys/socket.h>

# include "webserv.hpp"

class	Server
{
private :
	int					_socket_fd;
	struct sockaddr_in	_serv_addr;

public :
	Server();
	Server(const std::string &config_file);
	Server(const Server &copy);
	Server&	operator=(const Server &copy);
	~Server();

	void	start();
	void	stop(const std::string &msg);
	void	update();
};

#endif