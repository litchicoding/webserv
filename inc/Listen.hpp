#ifndef LISTEN_HPP
# define LISTEN_HPP

# include "webserv.hpp"

# define MAX_CLIENT_WAITING 5

class	Server;
class	Client;

typedef struct s_port
{
	std::string				port_address;
	struct sockaddr_in		port_addr;
	int						listen_fd;
	int						epoll_fd;
}							t_port;

class	Listen
{
private:
	/* Listen Port ID *********************************************************/
	std::map<int, t_port>	_listeningPorts; // <listen_fd, port struct>
	std::map<int, Client*>	_clients; // <client_fd, client class>
	std::vector<Server*>	_serv_blocks;
	std::string				_defaultServer;

public:
	Listen();
	~Listen();

	void					configuration();
	int						start_connexion();
	int						update_connexion();
	void					stop(const std::string &msg);
	struct sockaddr_in		createSockaddr(const char *ip, int port);
	int						handleClientRequest(int client_fd, int epoll_fd);

	/* Getters ****************************************************************/
	t_port					getListeningPort(int socket_fd) const;
	std::map<int, Client*>	getClients() const;
	std::vector<Server*>	getServerBlocks() const;
	
	/* Setters ****************************************************************/
	void					setServerBlocks(const std::vector<Server*> &serv_blocks);
	void					addNewClient(int listen_fd, int epoll_fd);
};

#endif