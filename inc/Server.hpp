#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <unistd.h>
# include <netinet/in.h> // sockaddr_in
# include <sys/socket.h>
# include <vector>
# include <map>

# include "webserv.hpp"

typedef struct	s_listen
{
	int	port;
	int	ip;
}				t_listen;

typedef struct	s_location
{
	std::string											path;
	std::map<std::string, std::vector<std::string> >	directives;
}				t_location;

class	Server
{
private :
	int													_socket_fd;
	struct sockaddr_in									_serv_addr;
	std::vector<t_listen>								_listen;
	std::vector<std::string>							_server_name;
	std::map<std::string, std::vector<std::string> >	_directives;
	std::vector<t_location>								_locations;

public :
	Server();
	Server(const Server &copy);
	Server&	operator=(const Server &copy);
	~Server();

	int		setDirectives(const std::string &type, const std::vector<std::string> &arg);
	int		setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg);

	void	start();
	void	stop(const std::string &msg);
	void	update();
	void	print_server_class();
};

#endif