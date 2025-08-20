#include "../inc/Server.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Server::Server()
{
	cout << GREEN << "*** Server Construction ***" << RESET << endl;
	_directives.autoindex = INVALID;
	_directives.client_max_body_size = DEFAULT_BODY_SIZE;
}

Server::~Server()
{
	_listen.clear();
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
	if (location.autoindex == INVALID) location.autoindex = serv.autoindex;
	if (location.client_max_body_size == 0)
		location.client_max_body_size = serv.client_max_body_size;
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
	if (_directives.root.empty()) _directives.root = DEFAULT_ROOT;
	if (_directives.index.empty()) _directives.index.push_back("index.html");
	if (_directives.methods.empty()) {
		_directives.methods.push_back("GET");
		_directives.methods.push_back("POST");
		_directives.methods.push_back("DELETE");
	}
	for (map<string, t_directives>::iterator it = _locations.begin(); it != _locations.end(); it++)
		defaultConfiguration(_directives, it->second);
}

t_directives*	Server::searchLocationMatch(const string &uri)
{
	map<string, t_directives>::iterator	location;
	string								match;
	t_directives						*result = NULL;
	size_t								prev_match_len = 0;

	if (uri.empty())
		return NULL;
	/* Cherche correspondance exacte entre l'URI de la requete et les path des location blocks */
	location = _locations.begin();
	while (location != _locations.end())
	{
		if (location->first == uri) {
			match = location->first;
			result = &(location->second);
			break ;
		}
		location++;
	}
	if (result == NULL) {
		/* Cherche la plus longue correspondance de préfixe (/images, /kapouet/admin vs /kapouet etc) */
		location = _locations.begin();
		while (location != _locations.end())
		{
			if (uri.find(location->first) != string::npos && location->first.length() > prev_match_len) {
				match = location->first;
				prev_match_len = match.length();
			}
			location++;
		}
		if (match.empty()) {
			match = "/";
			result = &(_locations.find("/")->second);
		}
		else
			result = &(_locations.find(match)->second);
	}
	if (result->root.find(match.c_str(), 0, match.length() - 1) == string::npos)
		result->full_path = result->root + uri.substr();
	else
		result->full_path = result->root + "/" + uri.substr(match.length());
	return result;
}

/*************************************************************************************************/
/* Setters ***************************************************************************************/

int	Server::setListen(const string &arg)
{
	t_listen		listen;
	size_t			pos;
	stringstream	ss;

	pos = arg.find(":", 0);
	if (pos != string::npos) {
		listen.ip = arg.substr(0, pos);
		listen.port = atoi(arg.substr(pos + 1, arg.size() - pos).c_str());
	}
	else if (arg.find(".", 0) != string::npos || arg == "localhost") {
		listen.port = DEFAULT_PORT;
		listen.ip = arg;
	}
	else {
		for (size_t i = 0; i < arg.size(); i++) {
			if (!isdigit(arg[i])) {
				cout << RED << "Error: setListen: invalid format" << RESET << endl;
				return ERROR;
			}
		}
		listen.port = atoi(arg.c_str());
		listen.ip = "0.0.0.0";
	}
	ss << listen.port;
	listen.address_port = listen.ip + ":" + ss.str();
	_listen.push_back(listen);
	return OK;
}

int	Server::setOneDirective(const string &type, const vector<string> &arg, t_directives *container)
{
	if (type.empty())
		return ERROR;
	if (type == "listen")
		setListen(arg[0]);
	if (type == "autoindex") {
		if (arg[0] == "on")
			container->autoindex = AUTO_ON;
		else if (arg[0] == "off")
			container->autoindex = AUTO_OFF;
	}
	else if (type == "client_max_body_size")
		setClientMaxBodySize(arg[0], *container);
	else if (type == "root")
		setRoot(arg[0], *container);
	else if (type == "index")
		setIndex(arg, *container);
	else if (type == "allow_methods")
		setMethods(arg, *container);
	else if (type == "return")
		container->redirection.insert(make_pair(atoi(arg[0].c_str()), arg[1]));
	else if (type == "error_page") {
		vector<string>::const_iterator it = arg.begin();
		string uri = arg.back();
		string code;
		while (it != arg.end())
		{
			if (*it == arg.back())
				break ;
			code = *it;
			it++;
			if (it->find("=", 0) != string::npos) {
				container->error_page.insert(make_pair(atoi(code.c_str()), *it));
				it++;
			}
			else
				container->error_page.insert(make_pair(atoi(code.c_str()), uri));
		}
	}
	return OK;
}

int	Server::setLocation(const string &loc_path, const string &type, const vector<string> &arg)
{
	if (loc_path.empty() || type.empty())
		return ERROR;

	map<string, t_directives>::iterator it = _locations.find(loc_path);
	if (it != _locations.end())
		setOneDirective(type, arg, &it->second);
	else {
		t_directives	newDirectives;
		newDirectives.autoindex = INVALID;
		newDirectives.client_max_body_size = 0;
		// setOneDirective(type, arg, &_locations[loc_path]);
		setOneDirective(type, arg, &newDirectives);
		_locations.insert(make_pair(loc_path, newDirectives));
	}
	return OK;
}

void	Server::setClientMaxBodySize(const string &value, t_directives &dir)
{
	if (value.empty())
		return ;
	
	size_t	nb = 0;
	for (size_t i = 0; i < value.size(); i++) {
		if (isdigit(value[i]))
			nb = nb * 10 + (value[i] - '0');
		else
			break ;
	}
	if (value.find("KB") != string::npos || value.find("K") != string::npos)
		dir.client_max_body_size = nb * 1024;
	else if (value.find("MB") != string::npos || value.find("M") != string::npos)
		dir.client_max_body_size =  nb * 1024 * 1024;
	else if (value.find("GB") != string::npos || value.find("G") != string::npos)
		dir.client_max_body_size =  nb * 1024 * 1024 * 1024;
	else
		dir.client_max_body_size =  nb * 1024;
}

void	Server::setRoot(const string &root, t_directives &dir) { dir.root = root; }

void	Server::setIndex(const vector<string> &index, t_directives &dir) { dir.index = index; }
	
void	Server::setMethods(const vector<string> &methods, t_directives &dir) { dir.methods = methods; }

/* Getters ***********************************************************************************************************/

const vector<t_listen>&	Server::getListen() const { return _listen; }

const t_directives&	Server::getDirectives() const { return _directives; }

t_directives&	Server::getDirectives() { return _directives; }

const map<string, t_directives>&	Server::getLocations() const { return _locations; }

/*********************************************************************************************************************/
/* Operator Overload *************************************************************************************************/

ostream&	operator<<(ostream &os, const Server &src)
{
	os << BLUE << "*** Display what's inside Server block ***" << endl;

	os << "listen directive -> " << RESET << endl;
	for (vector<t_listen>::const_iterator it = src.getListen().begin(); it != src.getListen().end(); it++) {
		os <<  BLUE << "port/ip/address_port= " << RESET;
		os << it->port << "/" << it->ip << "/" << it->address_port << endl;
	}

	print_directives(os, src.getDirectives());
	
	os << BLUE << "Location block -> " << RESET << endl;
	int	i = 1;
	for (map<string, t_directives>::const_iterator it = src.getLocations().begin(); it != src.getLocations().end(); it++)
	{
		os << YELLOW << "(loc block " << i << ") uri path : " << it->first << RESET << endl;
		print_directives(os, it->second);
		i++;
	}
	return os;
}

void	print_directives(ostream &os, const t_directives &directives)
{
	os << BLUE << "autoindex directive -> " << RESET;
	if (directives.autoindex == AUTO_ON)
		os << "on" << endl;
	else
		os << "off" << endl;

	os << BLUE << "client_max_body_size directive -> " << RESET;
	os << directives.client_max_body_size << endl;

	os << BLUE << "root directive -> " << RESET << directives.root << endl;
	
	os << BLUE << "index directive -> " << RESET;
	for (vector<string>::const_iterator it = directives.index.begin(); it != directives.index.end(); it++) {
		if (!it->empty())
			os << *it << " " << endl;
	}
	
	os << BLUE << "method directive -> " << RESET;
	for (vector<string>::const_iterator it = directives.methods.begin(); it != directives.methods.end(); it++)
		os << *it << " ";
	os << endl;

	os << BLUE << "redirection directive -> " << RESET;
	for (map<int, string>::const_iterator it = directives.redirection.begin(); it != directives.redirection.end(); it++)
		os << it->first << " " << it->second << endl;
	os << endl;

	os << BLUE << "error_page directive -> " << RESET;
	for (map<int, string>::const_iterator it = directives.error_page.begin(); it != directives.error_page.end(); it++)
		os << it->first << " " << it->second << endl;
	os << endl;
}
