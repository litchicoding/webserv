#ifndef LISTEN_HPP
# define LISTEN_HPP

# include "webserv.hpp"
# include <iostream>

# define MAX_CLIENT_WAITING 5

class	Server;
class	Client;

typedef struct s_port
{
	std::string				address_port;
	struct sockaddr_in		port_addr;
	int						listen_fd;
}							t_port;

class	Listen
{
private:
	/* Listen Port ID *****************************************************************************/
	int						_epoll_fd;
	std::map<int, t_port>	_listeningPorts; // <listen_fd, port struct>
	std::map<int, Client*>	_clients; // <client_fd, client class>
	std::vector<Server>		_serv_blocks;

public:
	Listen();
	~Listen();

	void					configuration();
	int						start_connexion();
	int						update_connexion();
	bool					isClientTimeOut(int client_fd);
	void					closeClientConnection(int client_fd);
	bool					isListeningSocket(int fd);
	void					stop(const std::string &msg);
	struct sockaddr_in		createSockaddr(const char *ip, int port);
	int						handleClientRequest(int client_fd, int listen_fd);
	Server*					findServerConfig(const int &listen_fd);

	/* Getters ************************************************************************************/
	std::map<int, t_port>&	getListeningPorts();
	std::map<int, Client*>&	getClients();
	std::vector<Server>&	getServerBlocks();
	
	/* Setters ************************************************************************************/
	void					setServerBlocks(const std::vector<Server> &serv_blocks);
	void					addNewClient(int listen_fd, int epoll_fd);
};

/* Operator Overload ******************************************************************************/
std::ostream&	operator<<(std::ostream &os, Listen &src);

#endif