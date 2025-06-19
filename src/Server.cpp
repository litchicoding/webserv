#include "Server.hpp"

/*****************************************************************************/
/* Constructors **************************************************************/

Server::Server()
{
	std::cout << GREEN << "*** Server construction ***" << RESET << std::endl;

	// _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	// if (_socket_fd < 0)
	// {
	// 	std::cout << RED << "Error: socket()" << RESET << std::endl;
	// 	return ;
	// }
	std::memset(&_serv_addr, 0, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_port = htons(8080); // HARDCODED value (must change it later)
	_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	g_server_instance = this;
	_socket_fd = INVALID;
	// _serv_addr = INVALID;
}

// Server::Server(const std::string &config_file)
// {
// 	std::cout << GREEN << "*** Server construction ***" << std::endl;
// 	std::cout << "[Configuration file : " << config_file << "]" << RESET << std::endl;
// 	g_server_instance = this;
// 	// Must parse the config file and assign struct with all the data needed...
// 	std::cout << RED << "Error: No function to parse config file yet!" << RESET << std::endl;
// }

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

Server::~Server() {} // must check that every container is cleaned

/*****************************************************************************/
/* Setters *******************************************************************/

int	Server::setDirectives(const std::string &type, const std::vector<std::string> &arg)
{
	if (type.empty())
		return ERROR;
	_directives.insert(std::make_pair(type, arg));
	return OK;
}

int	Server::setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg)
{
	if (loc_path.empty() || type.empty())
		return ERROR;
	std::vector<t_location>::iterator it = _locations.begin();
	while (it != _locations.end())
	{
		if (it->path == loc_path)
		{
			it->directives.insert(std::make_pair(type, arg));
			return OK;
		}
		it++;
	}
	if (it == _locations.end())
	{
		t_location	newLoc;
		newLoc.path = loc_path;
		newLoc.directives.insert(std::make_pair(type, arg));
		_locations.push_back(newLoc);
		return OK;
	}
	return ERROR;
}

/*****************************************************************************/
/* Member Functions **********************************************************/

void	Server::start()
{
	if (_socket_fd == INVALID)
		return ;

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
	while (_socket_fd != INVALID)
	{
		struct sockaddr_in  client_addr;
        socklen_t           client_addr_len = sizeof(client_addr);
        
		signal(SIGINT, &signal_handler);

		int socket_client = accept(_socket_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
		
		if (socket_client < 0) {
			stop("accept");
            return ;
        }
        else
        {
            char buffer[4064];
            std::memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(socket_client, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0) {
                perror("read");
            } else {
                std::cout << BLUE << "📨 Requête reçue :\n" << RESET << buffer << std::endl;
                // Ici lire la requete dans une fonction a part qui switch entre GET POST DELETE ERROR
                write(socket_client, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 50 \r\nConnection: close\r\n\r\nWebserv", 92);
			}
        }
        close(socket_client);
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

void	Server::print_server_class()
{
	std::cout << "----------------------------" << std::endl;
	std::cout << "socket_fd = " << _socket_fd << std::endl;
	
	for (std::vector<std::string>::iterator it = _server_name.begin(); it != _server_name.end(); it++)
		std::cout << "server_name = " << *it << std::endl;

	std::map<std::string, std::vector<std::string> >::iterator it = _directives.begin();
	
	while (it != _directives.end())
	{
		std::vector<std::string>::iterator ite = it->second.begin();
		std::cout << it->first << " ";
		while (ite != it->second.end())
		{
			std::cout << *ite << std::endl;
			ite++;
		}
		it++;
	}

	for (std::vector<t_location>::iterator it = _locations.begin(); it < _locations.end(); it++)
	{
		std::cout << "location = " << it->path << std::endl;
		std::map<std::string, std::vector<std::string> >::iterator a = it->directives.begin();
		while (a != it->directives.end())
		{
			std::cout << a->first << " ";
			std::vector<std::string>::iterator ite = a->second.begin();
			while (ite != a->second.end())
			{
				std::cout << *ite << " " << std::endl;
				ite++;
			}
			a++;
		}
	}
}
