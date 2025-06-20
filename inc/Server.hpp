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

# define DEFAULT_PORT 80
# define DEFAULT_SERVER_NAME ""
# define DEFAULT_ADDRESS_IP "0.0.0.0"

typedef struct	s_listen
{
	int			port;
	std::string	address;
	std::string	original;
}				t_listen;

typedef struct	s_directives
{
	bool												autoindex;
	int													client_max_body_size;
	std::string											root;
	std::vector<std::string>							index;
	std::vector<std::string>							methods;
	std::map<int, std::string>							return_code; // <error_code, text ou url>
	std::map<int, std::string>							error_page; // <error_code, error_uri_path>
}				t_directives;

typedef struct	s_location
{
	std::string											uri_path;
	std::map<std::string, std::vector<std::string> >	directives;
}				t_location;

class	Server
{
private :
	int													_socket_fd;
	struct sockaddr_in									_serv_addr;
	std::vector<t_listen>								_listen;
	std::vector<std::string>							_server_name;
	t_directives										_directives;
	std::vector<t_location>								_locations;

public :
	Server();
	Server(const Server &copy);
	Server&	operator=(const Server &copy);
	~Server();

	int		setListen(const std::string &arg);
	void	setServerName(const std::vector<std::string> &names);
	int		setDirectives(const std::string &type, const std::vector<std::string> &arg);
	int		setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg);
	void	setClientMaxBodySize(const int &value);
	void	setRoot(const std::string &root);
	void	setIndex(const std::vector<std::string> &index);
	void	setMethods(const std::vector<std::string> &methods);

	void	start();
	void	stop(const std::string &msg);
	void	update();
	void	print_server_class();

	static void	delete_server_group(std::vector<Server*> &server);
};

#endif