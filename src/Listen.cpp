#include "Listen.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Listen::Listen() : _epoll_fd(INVALID)
{
	cout << GREEN << "***  Listening ports Configuration   ***" << RESET << endl;
}

Listen::~Listen()
{
	// cout << GREEN << "***  Listening ports Deconstruction  ***" << RESET << endl;
	for (map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->first >= 0)
			close(it->first);
		delete it->second;
	}
	for (map<int, t_port>::iterator it = _listeningPorts.begin(); it != _listeningPorts.end(); it++)
	{
		if (it->first >= 0)
			close(it->first);
	}
	if (_epoll_fd >= 0)
		close(_epoll_fd);
	_listeningPorts.clear();
	_clients.clear();
	_serv_blocks.clear();
}

/*************************************************************************************************/
/* Member Functions ******************************************************************************/

struct sockaddr_in	Listen::createSockaddr(const char *ip, int port)
{
	stringstream		ss;
	struct addrinfo		hints, *res;
	struct sockaddr_in	addr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // pour bind
	ss << port;
	string portStr = ss.str();
	const char* port_cstr = portStr.c_str();
	if (getaddrinfo(ip, port_cstr, &hints, &res) != 0) {
		cout << RED << "Error: createSockaddr: getaddrinfo failed" << RESET << endl;
		exit(EXIT_FAILURE);
	}
	addr = *(reinterpret_cast<struct sockaddr_in*>(res->ai_addr));
	freeaddrinfo(res);
	return addr;
}

void	Listen::configuration()
{
	vector<Server>::iterator			serv = _serv_blocks.begin();
	vector<t_listen>::const_iterator	listen, listen_end;
	t_port									new_port;

	_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	while (serv != _serv_blocks.end())
	{
		listen = serv->getListen().begin();
		listen_end = serv->getListen().end();
		while (listen != listen_end)
		{
			new_port.address_port = (*listen).address_port;
			new_port.port_addr = createSockaddr((*listen).ip.c_str(), (*listen).port);
			if ((new_port.listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
				cout << RED << "Error: socket()" << RESET << endl;
				exit(EXIT_FAILURE);
			}
			int opt = 1;
			setsockopt(new_port.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
			_listeningPorts.insert(make_pair(new_port.listen_fd, new_port));
			listen++;
		}
		serv++;
	}
}

int	Listen::start_connexion()
{
	map<int, t_port>::iterator current_port = _listeningPorts.begin();
	int								listen_fd;
	struct sockaddr_in				address;

	while (current_port != _listeningPorts.end())
	{
		listen_fd = current_port->first;
		address = current_port->second.port_addr;
		if (add_fd_to_epoll(_epoll_fd, listen_fd) != OK)
			return ERROR;
		if (bind(listen_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != OK) {
			cout << RED << "Error: while trying to bind() to IP:" << address.sin_addr.s_addr;
			cout << " Port: " << ntohs(address.sin_port) << RESET << endl;
			perror("bind");
			return ERROR;
		}
		if (listen(listen_fd, MAX_CLIENT_WAITING) != OK) {
			cout << RED << "Error: while trying to listen() to socket_fd" << listen_fd << RESET << endl;
			return ERROR;
		}
		current_port++;
	}
	return OK;
}

bool	Listen::isClientTimeOut(int client_fd)
{
	map<int, Client*>::iterator client = _clients.find(client_fd);
	if (client == _clients.end())
		return false;
	else if (client->second == NULL)
		return false;
	else if (client->second->last_activity == 0)
		return false;
	double timeout = 1.0;
	time_t start = _clients[client_fd]->last_activity;
	time_t end;
	time(&end);
	double diff = difftime(end, start);
	// cout << "TimeOut Limit (seconds): " << timeout << endl;
	// cout << "start: " << start << endl;
	// cout << "end: " << end << endl;
	// cout << "Elapsed seconds: " << diff << " / Limit: " << timeout << endl;
	if (diff > timeout) {
		cout << GREEN << "--- timeout for client [socket:" << client_fd << "]" RESET << endl;
		return true;
	}
	return false;
}

int	Listen::update_connexion()
{
	map<int, t_port>::iterator	current_port;
	epoll_event					events[MAX_EVENTS];
	int							nfds;

	cout << PURPLE << "🟢 - SERVER IS LISTENING ON PORTS..." << RESET << endl << endl;
	while (g_global_instance)
	{
		signal(SIGINT, &signal_handler);
		nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, TIMEOUT);
		if (nfds < 0)
			continue ;
		for (map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (isClientTimeOut(it->first) == true) {
				closeClientConnection(it->first);
				break ;
			}
		}
		for (int i = 0; i < nfds; ++i)
		{
			signal(SIGINT, &signal_handler);
			if (isListeningSocket(events[i].data.fd)) //cas 1 : evenement sur le socket du serveur -> nouvelle connexion prete a etre acceptee
				addNewClient(events[i].data.fd, _epoll_fd);
			else //cas 2 : evenement sur le socket d'un client existant ->pret a etre lu
			{
				if (_clients.find(events[i].data.fd) == _clients.end())
					continue ;
				if (isClientTimeOut(events[i].data.fd) == true) {
					closeClientConnection(events[i].data.fd);
					continue ;
				}
				int listen_fd = _clients[events[i].data.fd]->getListenFd();
				if (handleClientRequest(events[i].data.fd, listen_fd) == ERROR)
					closeClientConnection(events[i].data.fd);
			}
		}
	}
	return OK;
}

bool	Listen::isListeningSocket(int fd)
{
	map<int, t_port>::iterator port = _listeningPorts.find(fd);
	if (port != _listeningPorts.end())
		return true;
	return false;
}


int	Listen::handleClientRequest(int client_fd, int listen_fd)
{
	time(&(_clients[client_fd]->last_activity));
	if (_clients[client_fd]->readData() != OK) {
		closeClientConnection(client_fd);
		return (ERROR);
	}
	if (_clients[client_fd]->state != READ_END)
		return (OK);
	_clients[client_fd]->setServerConfig(findServerConfig(listen_fd));
	if (_clients[client_fd]->getServerConfig() == NULL) {
		stop("no match for server configuration");
		return (ERROR);
	}
	_clients[client_fd]->processRequest();
	_clients[client_fd]->sendResponse();
	_clients[client_fd]->resetRequest();
	if (_clients[client_fd]->isKeepAliveConnection() == false)
		closeClientConnection(client_fd);
	return (OK);
}

void	Listen::closeClientConnection(int client_fd)
{
	if (client_fd < 0)
		return ;
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	map<int, Client*>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		close(client_fd);
		delete it->second;
		_clients.erase(it);
	}
}

Server*	Listen::findServerConfig(const int &listen_fd)
{
	vector<Server>::iterator			serv_block, serv_blocks_end;
	vector<t_listen>::const_iterator	listen_serv, listen_serv_end;
	map<int, t_port>::iterator			current_listening_port;

	current_listening_port = _listeningPorts.find(listen_fd);
	if (current_listening_port == _listeningPorts.end()) {
		stop("no match found for current listen fd");
		return NULL;
	}
	serv_block = _serv_blocks.begin();
	serv_blocks_end = _serv_blocks.end();
	while (serv_block != serv_blocks_end)
	{
		listen_serv = serv_block->getListen().begin();
		listen_serv_end = serv_block->getListen().end();
		while (listen_serv != listen_serv_end)
		{
			if (listen_serv->address_port == current_listening_port->second.address_port)
				return &(*serv_block);
			listen_serv++;
		}
		serv_block++;
	}
	return NULL;
}

void	Listen::stop(const string &msg)
{
	// must close, clear and delete every thing
	if (!msg.empty())
		cout << RED << "Error: " << msg << "()" << RESET << endl;
	for (map<int, t_port>::iterator it = _listeningPorts.begin(); it != _listeningPorts.end(); it++)
	{
		// epoll_ctl(it->second.epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		it->second.listen_fd = INVALID;
		_epoll_fd = INVALID;
	}
   	cout << PURPLE << endl << "🛑 - CONNECTION CLOSED." << RESET << endl;
}


/*************************************************************************************************/
/* Getters ***************************************************************************************/

map<int, t_port>&	Listen::getListeningPorts() { return _listeningPorts; }

map<int, Client*>&	Listen::getClients() { return _clients; }
	
vector<Server>&	Listen::getServerBlocks() { return _serv_blocks; }

/*************************************************************************************************/
/* Setters ***************************************************************************************/

void	Listen::setServerBlocks(const vector<Server> &serv_blocks)
{
	_serv_blocks = serv_blocks;
}

void	Listen::addNewClient(int listen_fd, int epoll_fd)
{
	Client	*newClient = new Client(listen_fd, epoll_fd);
	time(&newClient->last_activity);
	_clients.insert(make_pair(newClient->getClientFd(), newClient));
}

/*************************************************************************************************/
/* Operator Overload *****************************************************************************/

ostream&	operator<<(ostream &os, Listen &src)
{
	os << BLUE << "*** Display what's inside Listen object ***" << RESET << endl;
	int i = 1;

	for (vector<Server>::iterator it = src.getServerBlocks().begin();
										it != src.getServerBlocks().end(); it++)
	{
		os << "---------" <<  YELLOW << "(serv block " << i << ")" << RESET << "---------" << endl;
		os << *it << endl;
		os << endl << "--------------------------------------" << endl;
		i++;
	}

	os << BLUE << "listening ports -> " << RESET << endl;
	i = 1;
	for (map<int, t_port>::iterator it = src.getListeningPorts().begin();
										it != src.getListeningPorts().end(); it++)
	{
		os << YELLOW << "(port " << i << ")" << RESET << endl;
		os << BLUE << "	address_port: " << RESET << it->second.address_port << endl;
		os << BLUE << "	listen_fd: " << RESET << it->second.listen_fd << endl;
		os << BLUE << "	port_addr: " << endl;
		os << "		sin_port=" << RESET << it->second.port_addr.sin_port << endl;
		os << BLUE << "		sin_family=" << RESET << it->second.port_addr.sin_family << endl;
		os << BLUE << "		sin_addr=" << RESET << it->second.port_addr.sin_addr.s_addr << endl;
		os << BLUE << "		sin_zero=" << RESET << it->second.port_addr.sin_zero << endl;
		os << endl << "--------------------------------------" << endl;
		i++;
	}

	return os;
}