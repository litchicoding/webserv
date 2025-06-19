#include "Server.hpp"

/*****************************************************************************/
/* Constructors **************************************************************/

Server::Server()
{
	std::cout << GREEN << "*** Server construction ***" << RESET << std::endl;
	/* OLD CONSTRUCTION */
	// _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	// if (_socket_fd < 0)
	// {
	// 	std::cout << RED << "Error: socket()" << RESET << std::endl;
	// 	return ;
	// }
	// std::memset(&_serv_addr, 0, sizeof(_serv_addr));
	// _serv_addr.sin_family = AF_INET;
	// _serv_addr.sin_port = htons(8080); // HARDCODED value (must change it later)
	// _serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* NEW CONSTRUCTION */
	_socket_fd = INVALID;
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

Server::~Server()
{
	_listen.clear();
	_server_name.clear();
	_directives.index.clear();
	_directives.methods.clear();
	_directives.return_code.clear();
	_directives.error_page.clear();
	_locations.clear();
}

/*****************************************************************************/
/* Setters *******************************************************************/

int	Server::setListen(const std::string &arg)
{
	t_listen	listen;
	size_t		pos;

	listen.original = arg;
	pos = arg.find(":", 0);
	if (pos != std::string::npos) {
		listen.address = arg.substr(0, pos);
		listen.port = atoi(arg.substr(pos, arg.size() - pos).c_str());
	}
	else if (arg.find(".", 0) != std::string::npos || arg == "localhost") {
		listen.port = 80;
		listen.address = arg;
	}
	else {
		for (size_t i = 0; i < arg.size(); i++) {
			if (!isdigit(arg[i])) {
				std::cout << RED << "Error: setListen: invalid format" << RESET << std::endl;
				return ERROR;
			}
		}
		listen.port = atoi(arg.c_str());
		listen.address = "0.0.0.0";
	}
	_listen.push_back(listen);
	return OK;
}

void	Server::setServerName(const std::vector<std::string> &names) { _server_name = names; }

int	Server::setDirectives(const std::string &type, const std::vector<std::string> &arg)
{
	if (type.empty())
		return ERROR;
	if (type == "listen")
		setListen(arg[0]);
	else if (type == "server_name")
		setServerName(arg);
	if (type == "autoindex") {
		if (arg[0] == "on")
			_directives.autoindex = true;
	}
	else if (type == "client_max_body_size")
		setClientMaxBodySize(atoi(arg[0].c_str()));
	else if (type == "root")
		setRoot(arg[0]);
	else if (type == "index")
		setIndex(arg);
	else if (type == "allow_methods")
		setMethods(arg);
	else if (type == "return")
		_directives.return_code.insert(std::make_pair(atoi(arg[0].c_str()), arg[1]));
	else if (type == "error_page") {
		std::vector<std::string>::const_iterator it = arg.begin();
		std::string uri = arg.back();
		std::string code;
		while (it != arg.end())
		{
			if (*it == arg.back())
				break ;
			code = *it;
			it++;
			if (it->find("=", 0) != std::string::npos) {
				_directives.error_page.insert(std::make_pair(atoi(code.c_str()), *it));
				it++;
			}
			else
				_directives.error_page.insert(std::make_pair(atoi(code.c_str()), uri));
		}
	}
	return OK;
}

int	Server::setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg)
{
	if (loc_path.empty() || type.empty())
		return ERROR;

	std::vector<t_location>::iterator it = _locations.begin();
	while (it != _locations.end())
	{
		if (it->uri_path == loc_path)
		{
			it->directives.insert(std::make_pair(type, arg));
			return OK;
		}
		it++;
	}
	if (it == _locations.end())
	{
		t_location	newLoc;
		newLoc.uri_path = loc_path;
		newLoc.directives.insert(std::make_pair(type, arg));
		_locations.push_back(newLoc);
		return OK;
	}
	return ERROR;
}

void	Server::setClientMaxBodySize(const int &value) { _directives.client_max_body_size = value; }

void	Server::setRoot(const std::string &root) { _directives.root = root; }

void	Server::setIndex(const std::vector<std::string> &index) { _directives.index = index; }
	
void	Server::setMethods(const std::vector<std::string> &methods) { _directives.methods = methods; }

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

void	Server::delete_server_group(std::vector<Server*> &server)
{
	for (std::vector<Server*>::iterator it = server.begin(); it != server.end(); it++)
		delete *it;
}

void	Server::print_server_class()
{
	std::cout << "SERVER BLOCK ----------------------------" << std::endl;
	std::cout << "socket_fd = " << _socket_fd << std::endl;
	
	std::cout << "Directives = " << std::endl;

	for (std::vector<std::string>::iterator it = _server_name.begin(); it != _server_name.end(); it++)
		std::cout << "	server_name = " << *it << std::endl;

	for (std::vector<t_listen>::iterator it = _listen.begin(); it != _listen.end(); it++)
		std::cout << "	listen = " << it->original << "(" << it->address << "/" << it->port << ")" << std::endl;

	if (_directives.autoindex)
		std::cout << "	autoindex = on" << std::endl;
	else
		std::cout << "	autoindex = off" << std::endl;
	std::cout << "	client_max_body_size = " << _directives.client_max_body_size << std::endl;
	std::cout << "	root = " << _directives.root << std::endl;

	std::cout << "	index = ";
	for (std::vector<std::string>::iterator it = _directives.index.begin(); it != _directives.index.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;

	std::cout << "	methods = ";
	for (std::vector<std::string>::iterator it = _directives.methods.begin(); it != _directives.methods.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;

	std::cout << "	return_code = ";
	for (std::map<int, std::string>::iterator it = _directives.return_code.begin(); it != _directives.return_code.end(); it++)
		std::cout << it->first << " " << it->second << std::endl;

	std::cout << "	error_page = ";
	for (std::map<int, std::string>::iterator it = _directives.error_page.begin(); it != _directives.error_page.end(); it++)
		std::cout << it->first << " " << it->second << std::endl;
	
	for (std::vector<t_location>::iterator it = _locations.begin(); it < _locations.end(); it++)
	{
		std::cout << "Location block = " << "URI = " << it->uri_path << std::endl;
		std::map<std::string, std::vector<std::string> >::iterator a = it->directives.begin();
		while (a != it->directives.end())
		{
			std::cout << "	" << a->first << " ";
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
