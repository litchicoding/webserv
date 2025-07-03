#include "Listen.hpp"
#include <arpa/inet.h>

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Listen::Listen()
{
	std::cout << GREEN << "*** Listen Construction ***" << RESET << std::endl;
}

Listen::~Listen()
{
	std::cout << GREEN << "*** Listen Deconstruction ***" << RESET << std::endl;
	// must delete everything needed
	for (std::map<int, t_port>::iterator it = _listeningPorts.begin(); it != _listeningPorts.end(); it++)
	{
		close(it->second.listen_fd);
		close(it->second.epoll_fd);
	}
	_listeningPorts.clear();
	_clients.clear();
	_serv_blocks.clear();
}

/*************************************************************************************************/
/* Member Functions ******************************************************************************/

struct sockaddr_in	Listen::createSockaddr(const char *ip, int port)
{
	std::stringstream	ss;
	struct addrinfo		hints, *res;
	struct sockaddr_in	addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // pour bind
	ss << port;
	std::string portStr = ss.str();
	const char* port_cstr = portStr.c_str();
	std::cout << "createSockaddr: ip=" << ip << " port=" << port << std::endl;
	if (getaddrinfo(ip, port_cstr, &hints, &res) != 0) {
		std::cout << RED << "Error: createSockaddr: getaddrinfo failed" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	addr = *(reinterpret_cast<struct sockaddr_in*>(res->ai_addr));
	freeaddrinfo(res);
	return addr;
}

void	Listen::configuration()
{
	std::vector<Server>::iterator			serv = _serv_blocks.begin();
	std::vector<t_listen>::const_iterator	listen, listen_end;
	t_port									new_port;

	while (serv != _serv_blocks.end())
	{
		listen = serv->getListen().begin();
		listen_end = serv->getListen().end();
		while (listen != listen_end)
		{
			new_port.address_port = (*listen).address_port;
			std::cout << "configuration: ip=" << (*listen).ip.c_str() << " port=" << (*listen).port << std::endl;
			new_port.port_addr = createSockaddr((*listen).ip.c_str(), (*listen).port);
			if ((new_port.listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
				std::cout << RED << "Error: socket()" << RESET << std::endl;
				exit(EXIT_FAILURE);
			}
			int opt = 1;
			setsockopt(new_port.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
			new_port.epoll_fd = epoll_create1(EPOLL_CLOEXEC);
			_listeningPorts.insert(std::make_pair(new_port.listen_fd, new_port));
			listen++;
		}
		serv++;
	}
}

int	Listen::start_connexion()
{
	std::map<int, t_port>::iterator current_port = _listeningPorts.begin();
	int								listen_fd, epoll_fd;
	struct sockaddr_in				address;

	while (current_port != _listeningPorts.end())
	{
		listen_fd = current_port->first;
		epoll_fd = current_port->second.epoll_fd;
		address = current_port->second.port_addr;
		if (add_fd_to_epoll(epoll_fd, listen_fd) != OK)
			return ERROR;
		if (bind(listen_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != OK) {
			std::cout << "Trying to bind to IP: " << inet_ntoa(address.sin_addr) << " Port: " << ntohs(address.sin_port) << std::endl;
			std::cout << RED << "Error: while bind()" << RESET << std::endl;
			perror("bind");
			return ERROR;
		}
		if (listen(listen_fd, MAX_CLIENT_WAITING) != OK) {
			std::cout << RED << "Error: while listen()" << RESET << std::endl;
			return ERROR;
		}
		current_port++;
	}
	return OK;
}

int	Listen::update_connexion()
{
	std::map<int, t_port>::iterator	current_port;
	epoll_event						events[MAX_EVENTS];
	int								nfds, epoll_fd, listen_fd;

	std::cout << GREEN << "🟢 Serveur en écoute" << RESET << std::endl;
	while (true)
	{
		signal(SIGINT, &signal_handler);
		current_port = _listeningPorts.begin();
		while (current_port != _listeningPorts.end())
		{
			listen_fd = current_port->second.listen_fd;
			epoll_fd = current_port->second.epoll_fd;
			if ((nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT)) == ERROR) {
				if (errno != EINTR)
					return perror("epoll_wait"), ERROR;
				return OK;
			}
			for (int i = 0; i < nfds; ++i)
			{
				signal(SIGINT, &signal_handler);
				if (events[i].data.fd == listen_fd) //cas 1 : evenement sur le socket du serveur -> nouvelle connexion prete a etre acceptee
					addNewClient(listen_fd, epoll_fd);
				else //cas 2 : evenement sur le socket d'un client existant ->pret a etre lu
					handleClientRequest(events[i].data.fd, epoll_fd);
			}
			current_port++;
		}
	}
	return OK;
}

int	Listen::handleClientRequest(int client_fd, int epoll_fd)
{
	char buffer[4064];
	memset(buffer, 0, sizeof(buffer));

	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	if (bytes_read <= 0)
	{
		std::cout << "Error handleClientRequest" << std::endl;
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		delete _clients[client_fd];
		_clients.erase(client_fd);
		return ERROR;
	}
	std::cout << BLUE << "📨 Requête reçue :\n" << RESET << buffer << std::endl;
	write(client_fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello Web!", 92);
	// epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	// close(client_fd);
	// delete _clients[client_fd];
	// _clients.erase(client_fd);

	// retreive the client with client_fd
	// store the request in associated client class, with buffer and read, and
	// parse the request and start filling datas in client class
	// retreive the appropriate server block config with PORT and HOST of clientRequest
	// build response based on request and server config
	return OK;
}

void	Listen::stop(const std::string &msg)
{
	// must close, clear and delete every thing
	if (!msg.empty())
		std::cout << RED << "Error: " << msg << "()" << RESET << std::endl;
	for (std::map<int, t_port>::iterator it = _listeningPorts.begin(); it != _listeningPorts.end(); it++)
	{
		// epoll_ctl(it->second.epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(it->second.listen_fd);
		it->second.listen_fd = INVALID;
		close(it->second.epoll_fd);
		it->second.epoll_fd = INVALID;
	}
   	std::cout << GREEN << "🛑 Connexion fermée." << RESET << std::endl;
}


/*************************************************************************************************/
/* Getters ***************************************************************************************/

std::map<int, t_port>&	Listen::getListeningPorts() { return _listeningPorts; }

// const std::map<int, t_port>&	Listen::getListeningPorts() const { return _listeningPorts; }

t_port&	Listen::getListeningPort(int listen_fd)
{
	std::map<int, t_port>::iterator it = _listeningPorts.find(listen_fd);
	// if (it != _listeningPorts.end())
		return it->second;
	// t_port error;
	// error.listen_fd = INVALID;
	// return error;
}

std::map<int, Client*>&	Listen::getClients() { return _clients; }
	
std::vector<Server>&	Listen::getServerBlocks() { return _serv_blocks; }

/*************************************************************************************************/
/* Setters ***************************************************************************************/

void	Listen::setServerBlocks(const std::vector<Server> &serv_blocks)
{
	_serv_blocks = serv_blocks;
}

void	Listen::addNewClient(int listen_fd, int epoll_fd)
{
	Client	*newClient = new Client(listen_fd, epoll_fd);
	_clients.insert(std::make_pair(newClient->getClientFd(), newClient));
}

/*************************************************************************************************/
/* Operator Overload *****************************************************************************/

std::ostream&	operator<<(std::ostream &os, Listen &src)
{
	os << BLUE << "*** Display what's inside Listen object ***" << RESET << std::endl;
	int i = 1;

	for (std::vector<Server>::iterator it = src.getServerBlocks().begin();
										it != src.getServerBlocks().end(); it++)
	{
		os << BLUE << "(serv block " << i << ")" << RESET << "--------------------" << std::endl;
		os << *it << std::endl;
		os << std::endl << "--------------------------------------" << std::endl;
		i++;
	}

	os << BLUE << "listening ports -> " << RESET << std::endl;
	i = 1;
	for (std::map<int, t_port>::iterator it = src.getListeningPorts().begin();
										it != src.getListeningPorts().end(); it++)
	{
		os << BLUE << "(port " << i << ")" << RESET << std::endl;
		os << BLUE << "	address_port: " << RESET << it->second.address_port << std::endl;
		os << BLUE << "	listen_fd: " << RESET << it->second.listen_fd << std::endl;
		os << BLUE << "	epoll_fd: " << RESET << it->second.epoll_fd << std::endl;
		os << BLUE << "	port_addr: " << std::endl;
		os << "		sin_port=" << RESET << it->second.port_addr.sin_port << std::endl;
		os << BLUE << "		sin_family=" << RESET << it->second.port_addr.sin_family << std::endl;
		os << BLUE << "		sin_addr=" << RESET << it->second.port_addr.sin_addr.s_addr << std::endl;
		os << BLUE << "		sin_zero=" << RESET << it->second.port_addr.sin_zero << std::endl;
		os << std::endl << "--------------------------------------" << std::endl;
		i++;
	}

	return os;
}
