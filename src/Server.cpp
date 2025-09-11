#include "../inc/Server.hpp"

/**************************************************************************************************/
/* Constructor and Deconstructor ******************************************************************/

Server::Server()
{
	cout << GREEN << "***  New Server Configuration  ***" << RESET << endl;
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
	if (location.redirection.empty()) location.redirection = serv.redirection;
	if (location.error_page.empty()) location.error_page = serv.error_page;
}

void	Server::defaultConfiguration()
{
	if (_listen.empty()) {
		t_listen	listen;
		listen.port = DEFAULT_PORT;
		listen.ip = "0.0.0.0";
		listen.address_port = "0.0.0.0:8080";
		_listen.push_back(listen);
	}
	if (_directives.autoindex == INVALID) _directives.autoindex = AUTO_OFF;
	if (_directives.root.empty()) _directives.root = DEFAULT_ROOT;
	if (_directives.index.empty()) _directives.index.push_back("index.html");
	if (_directives.methods.empty()) {
		_directives.methods.push_back("GET");
		_directives.methods.push_back("POST");
		_directives.methods.push_back("DELETE");
	}
	if (_locations.empty()) {
		t_directives	newDirectives;
		newDirectives.autoindex = INVALID;
		newDirectives.client_max_body_size = 0;
		_locations.insert(make_pair("/", newDirectives));
	}
	for (map<string, t_directives>::iterator it = _locations.begin(); it != _locations.end(); it++)
		defaultConfiguration(_directives, it->second);
}



t_directives*	Server::searchLocationMatch(const string &request_uri)
{
	map<string, t_directives>::iterator	location;
	string								uri, longest_loc_match, query_string, loc_path;
	t_directives						*result = NULL;

	if (request_uri.empty())
		return NULL;
	uri = request_uri;
	/*separe la query string si il y en a une*/ // <- Faire dans une fonction à part
	size_t qpos = request_uri.find('?');
	if (qpos != string::npos) {
		uri = request_uri.substr(0, qpos);
		query_string = request_uri.substr(qpos + 1);
	}

	/*Vérifie d'abord si une extension correspond exactement à un bloc*/
	size_t dot_pos = uri.rfind('.');
	if (dot_pos != string::npos) {
		string extension = uri.substr(dot_pos);
		location = _locations.find(extension);
		if (location != _locations.end()) {
			result = &(location->second);

			string root = result->root;
			if (!root.empty() && root[root.length() - 1] == '/')
				root.erase(root.length() - 1);

			if (uri.find("/cgi-bin/") == 0)
			{
				string relative = uri.substr(string("/cgi-bin").length());
				result->full_path = root + relative;
			}
			else
				result->full_path = root + uri;

			result->query_string = query_string;
			return result;
		}
	}

	/* Sinon, cherche la plus longue correspondance de préfixe (ex: uri=/kapouet/admin path_1=/kapouet/admin path_2=/kapouet path_1 is selected) */
	location = _locations.begin();
	while (location != _locations.end())
	{
		loc_path = location->first;
		// cout << GREEN << "path = " << loc_path << RESET << endl;
		if (uri.length() >= loc_path.length() && uri.substr(0, loc_path.length()) == loc_path) {
			// if (loc_path[loc_path.length() - 1] == '/' || uri[loc_path.length() - 1] == '/') {
				if (loc_path.length() > longest_loc_match.length())
					longest_loc_match = loc_path;
			// }
		}
		location++;
	}
	if (longest_loc_match.empty())
	{
		longest_loc_match = "/";
		location = _locations.find("/");
		if (location != _locations.end())
			result = &(location->second);
		else
			return (NULL);
	}
	else
		result = &(_locations.find(longest_loc_match)->second);
	// cout << "root = " << result->root << endl;
	// cout << "longest loc match = " << longest_loc_match << endl;
	// cout << "uri = " << uri << endl;
	string root = result->root;
	if (!root.empty() && root[root.length() - 1] == '/')
		root.erase(root.length() - 1);
	result->locationBlocName = longest_loc_match;
	result->full_path = root + uri;
	result->query_string = query_string;
	return (result);
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
	else if (type == "return" && container->redirection.empty())
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
	if (loc_path.empty())
		return ERROR;

	map<string, t_directives>::iterator it = _locations.find(loc_path);
	if (it != _locations.end())
		setOneDirective(type, arg, &it->second);
	else {
		t_directives	newDirectives;
		newDirectives.autoindex = INVALID;
		newDirectives.client_max_body_size = 0;
		if (!arg.empty())
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
