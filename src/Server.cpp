#include "../inc/Server.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Server::Server()
{
	std::cout << GREEN << "*** Server construction ***" << RESET << std::endl;
	_directives.autoindex = INVALID;
	_directives.client_max_body_size = INVALID;
}

Server::~Server()
{
	_listen.clear();
	_server_name.clear();
	_directives.index.clear();
	_directives.methods.clear();
	_directives.redirection.clear();
	_directives.error_page.clear();
	_locations.clear();
}

/*************************************************************************************************/
/* Member Functions ******************************************************************************/

void	Server::defaultConfiguration(t_directives serv, t_directives &location)
{
	if (location.client_max_body_size == INVALID)
		location.client_max_body_size = DEFAULT_BODY_SIZE;
	if (location.root.empty()) location.root = serv.root;
	if (location.index.empty()) location.index = serv.index;
	if (location.methods.empty()) location.methods = serv.methods;
}

void	Server::defaultConfiguration()
{
	if (_listen.empty()) {
		t_listen	listen;
		listen.port = DEFAULT_PORT;
		listen.ip = "0.0.0.0";
		listen.address_port = "0.0.0.0:80";
		_listen.push_back(listen);
	}
	if (_server_name.empty()) {
		std::string	name = DEFAULT_SERVER_NAME;
		_server_name.push_back(name);
	}
	if (_directives.client_max_body_size == INVALID)
		_directives.client_max_body_size = DEFAULT_BODY_SIZE;
	if (_directives.root.empty()) _directives.root = DEFAULT_ROOT;
	if (_directives.index.empty()) _directives.index.push_back("index.html");
	if (_directives.methods.empty()) {
		_directives.methods.push_back("GET");
		_directives.methods.push_back("POST");
		_directives.methods.push_back("DELETE");
	}
	for (std::map<std::string, t_directives>::iterator it = _locations.begin(); it != _locations.end(); it++)
		defaultConfiguration(_directives, it->second);
}

t_directives*	Server::searchLocationMatch(const std::string &request_uri)
{
	/* cherche correspondance exacte */
	// = définit une correspondance exacte entre l'URI et une chaîne. 
	// Si correspondance exacte, la recherche s'arrête.
	std::map<std::string, t_directives>::iterator location_block = _locations.begin();
	while (location_block != _locations.end())
	{
		if (location_block->first == request_uri)
			return &(location_block->second);
		location_block++;
	}
	/* Test des chaînes de préfixe */
	// teste l'URI contre tous les préfixes (/images, / etc) et stocke la plus longue correspondance

	/* Modificateur ^~ */
	// Si ^~ précède la chaîne de préfixe correspondante la plus longue, les expressions régulières sont pas vérifiées. 
	// Ce modificateur interrompt l'évaluation des regex.

	/* Expressions régulières */
	// Teste l'URI contre les expressions régulières dans l'ordre où elles apparaissent dans le fichier
	// ~ pour une correspondance sensible à la casse
	// ~* pour une correspondance insensible à la casse

	/* Sélection finale */
	// s'arrête lorsque la première expression régulière correspondante est trouvée
	return NULL;
}

/*************************************************************************************************/
/* Setters ***************************************************************************************/

int	Server::setListen(const std::string &arg)
{
	t_listen	listen;
	size_t		pos;
	listen.address_port = arg;
	pos = arg.find(":", 0);
	if (pos != std::string::npos) {
		listen.ip = arg.substr(0, pos);
		listen.port = atoi(arg.substr(pos + 1, arg.size() - pos).c_str());
	}
	else if (arg.find(".", 0) != std::string::npos || arg == "localhost") {
		listen.port = 80;
		listen.ip = arg;
	}
	else {
		for (size_t i = 0; i < arg.size(); i++) {
			if (!isdigit(arg[i])) {
				std::cout << RED << "Error: setListen: invalid format" << RESET << std::endl;
				return ERROR;
			}
		}
		listen.port = atoi(arg.c_str());
		listen.ip = "0.0.0.0";
	}
	_listen.push_back(listen);
	return OK;
}

void	Server::setServerName(const std::vector<std::string> &names) { _server_name = names; }

int	Server::setOneDirective(const std::string &type, const std::vector<std::string> &arg, t_directives *container)
{
	if (type.empty())
		return ERROR;
	if (type == "listen")
		setListen(arg[0]);
	else if (type == "server_name")
		setServerName(arg);
	if (type == "autoindex") {
		if (arg[0] == "on")
			container->autoindex = AUTO_ON;
		else if (arg[0] == "off")
			container->autoindex = AUTO_OFF;
	}
	else if (type == "client_max_body_size")
		setClientMaxBodySize(atoi(arg[0].c_str()), *container);
	else if (type == "root")
		setRoot(arg[0], *container);
	else if (type == "index")
		setIndex(arg, *container);
	else if (type == "allow_methods")
		setMethods(arg, *container);
	else if (type == "return")
		container->redirection.insert(std::make_pair(atoi(arg[0].c_str()), arg[1]));
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
				container->error_page.insert(std::make_pair(atoi(code.c_str()), *it));
				it++;
			}
			else
				container->error_page.insert(std::make_pair(atoi(code.c_str()), uri));
		}
	}
	return OK;
}

int	Server::setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg)
{
	if (loc_path.empty() || type.empty())
		return ERROR;

	std::map<std::string, t_directives>::iterator it = _locations.find(loc_path);
	if (it != _locations.end())
		setOneDirective(type, arg, &it->second);
	else {
		t_directives	newDirectives;
		newDirectives.autoindex = false;
		newDirectives.client_max_body_size = INVALID;
		_locations.insert(std::make_pair(loc_path, newDirectives));
		setOneDirective(type, arg, &_locations[loc_path]);
	}
	return OK;
}

void	Server::setClientMaxBodySize(const int &value, t_directives &dir) { dir.client_max_body_size = value; }

void	Server::setRoot(const std::string &root, t_directives &dir) { dir.root = root; }

void	Server::setIndex(const std::vector<std::string> &index, t_directives &dir) { dir.index = index; }
	
void	Server::setMethods(const std::vector<std::string> &methods, t_directives &dir) { dir.methods = methods; }

/* Getters ***********************************************************************************************************/

const std::vector<t_listen>&	Server::getListen() const { return _listen; }

const std::vector<std::string>&	Server::getServerName() const { return _server_name; }

const t_directives&	Server::getDirectives() const { return _directives; }

t_directives&	Server::getDirectives() { return _directives; }

const std::map<std::string, t_directives>&	Server::getLocations() const { return _locations; }

/*********************************************************************************************************************/
/* Operator Overload *************************************************************************************************/

std::ostream&	operator<<(std::ostream &os, const Server &src)
{
	os << BLUE << "*** Display what's inside Server block ***" << std::endl;

	os << "listen directive -> " << std::endl;
	for (std::vector<t_listen>::const_iterator it = src.getListen().begin(); 
										it != src.getListen().end(); it++)
	{
		os << BLUE << "port/ip/address_port= " << RESET;
		os << it->port << "/" << it->ip << "/" << it->address_port << std::endl;
	}

	os << BLUE << "server_name directive -> " << RESET << std::endl;
	for (std::vector<std::string>::const_iterator it = src.getServerName().begin();
											it != src.getServerName().end(); it++)
		os << *it << std::endl;

	print_directives(os, src.getDirectives());
	
	os << BLUE << "Location block -> " << RESET << std::endl;
	int	i = 1;
	for (std::map<std::string, t_directives>::const_iterator it = src.getLocations().begin(); it != src.getLocations().end(); it++)
	{
		os << BLUE << "(loc block " << i << ")" << RESET << "--------------------" << std::endl;
		print_directives(os, it->second);
		i++;
	}
	return os;
}

void	print_directives(std::ostream &os, const t_directives &directives)
{
	os << BLUE << "autoindex directive -> " << RESET;
	if (directives.autoindex == AUTO_ON)
		os << "on" << std::endl;
	else
		os << "off" << std::endl;

	os << BLUE << "client_max_body_size directive -> " << RESET;
	os << directives.client_max_body_size << std::endl;

	os << BLUE << "root directive -> " << RESET << directives.root << std::endl;
	
	os << BLUE << "index directive -> " << RESET;
	for (std::vector<std::string>::const_iterator it = directives.index.begin(); it != directives.index.end(); it++) {
		if (!it->empty())
			os << *it << " " << std::endl;
	}
	
	os << BLUE << "method directive -> " << RESET;
	for (std::vector<std::string>::const_iterator it = directives.methods.begin(); it != directives.methods.end(); it++)
		os << *it << " ";
	os << std::endl;

	os << BLUE << "redirection directive -> " << RESET;
	for (std::map<int, std::string>::const_iterator it = directives.redirection.begin(); it != directives.redirection.end(); it++)
		os << it->first << " " << it->second << std::endl;
	
	os << BLUE << "error_page directive -> " << RESET;
	for (std::map<int, std::string>::const_iterator it = directives.error_page.begin(); it != directives.error_page.end(); it++)
		os << it->first << " " << it->second << std::endl;
	std::cout << std::endl;
}
