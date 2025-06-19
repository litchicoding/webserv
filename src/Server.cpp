#include "../inc/Server.hpp"

/*****************************************************************************/
/* Constructors **************************************************************/

Server::Server()
{
	std::cout << GREEN << "*** Server construction ***" << std::endl;
	std::cout << "[Configuration file : default ]" << RESET << std::endl;

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0)
	{
		std::cout << RED << "Error: socket()" << RESET << std::endl;
		return ;
	}
	std::memset(&_serv_addr, 0, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_port = htons(8080); // HARDCODED value (must change it later)
	_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	g_server_instance = this;
}

Server::Server(const std::string &config_file)
{
	std::cout << GREEN << "*** Server construction ***" << std::endl;
	std::cout << "[Configuration file : " << config_file << "]" << RESET << std::endl;
	g_server_instance = this;
	// Must parse the config file and assign struct with all the data needed...
	std::cout << RED << "Error: No function to parse config file yet!" << RESET << std::endl;
}

Server::Server(const Server &copy) { *this = copy; }

Server&	Server::operator=(const Server &copy)
{
	// TO DO (ou pas ? doit on coder les classes sous forme canonique pour ce projet?)
	(void)copy;
	// if (this != &copy)
	// {
	// }
	return *this;
}

/*****************************************************************************/
/* Deconstructor *************************************************************/

Server::~Server() {} // freeaddrinfo() ?

/*****************************************************************************/
/* Member Functions **********************************************************/

void	Server::start()
{
	if (_socket_fd == INVALID)
		return ;
	
	_epoll_fd = epoll_create1(EPOLL_CLOEXEC);

	if (add_fd_to_epoll(_epoll_fd, _socket_fd) != OK)
	{
		stop("epoll_ctl");
		return;
	}

	if (bind(_socket_fd, reinterpret_cast<sockaddr*>(&_serv_addr), sizeof(_serv_addr)) != OK)
	{
		stop("bind");
		return ;
	}

	if (listen(_socket_fd, 5) != OK)
	{
		stop("listen");
		return ;
	}

	std::cout << BLUE << "🟢 Serveur en écoute sur le port 8080..." << RESET << std::endl;
}

void	Server::update()
{
	epoll_event events[MAX_EVENTS];
	while (_socket_fd != INVALID)
	{
		signal(SIGINT, &signal_handler);
		int nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, TIMEOUT);
		if (nfds == INVALID)
		{
			perror("epoll_wait");
			break;
		}

		for (int i=0; i < nfds; ++i)
		{
			if (events[i].data.fd == _socket_fd) //cas 1 : evenement sur le socket du serveur -> nouvelle connexion prete a etre acceptee
			{
				struct sockaddr_in  client_addr;
       			socklen_t           client_addr_len = sizeof(client_addr);

				int socket_client = accept(_socket_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
				if (socket_client == INVALID)
				{
					perror("accept");
					continue;
				}
				if (add_fd_to_epoll(_epoll_fd, socket_client) != OK)
					continue;
				Client* new_client = new Client(socket_client, client_addr);
				new_client->start();
				_clients[socket_client] = new_client;
				std::cout << GREEN << "✅ Nouveau client connecté: " << new_client->getSocketFd()
                          << RESET << std::endl;
			}
			else	//cas 2 : evenement sur le socket d'un client existant ->pret a etre lu
			{
				int socket_client = events[i].data.fd;
				Client* client = _clients[socket_client];
				client->update();//to do
				delete client;
				_clients.erase(socket_client);
				close(socket_client); // pas sur qu'on garde a voir !
				epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, socket_client, NULL);
			}
		}
    }

}

void	Server::stop(const std::string &msg)
{
	if (_socket_fd == INVALID)
		return ;
	if (!msg.empty())
		std::cout << RED << "Error: " << msg << "()" << RESET << std::endl;
	close(_socket_fd);
	_socket_fd = INVALID;
   	std::cout << GREEN << "🛑 Connexion fermée." << RESET << std::endl;
}
