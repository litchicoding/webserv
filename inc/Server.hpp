#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"

# define DEFAULT_PORT 8080
# define DEFAULT_ADDRESS_IP "0.0.0.0"
# define DEFAULT_SERVER_NAME ""
# define DEFAULT_BODY_SIZE 1
# define DEFAULT_ROOT "html"
# define DEFAULT_INDEX "index.html"

# define AUTO_ON 1
# define AUTO_OFF 0

typedef struct	s_listen
{
	int							port;
	std::string					ip;
	std::string					address_port;
}				t_listen;

typedef struct	s_directives
{
	int							autoindex;
	int							client_max_body_size;
	std::string					root;
	std::vector<std::string>	index;
	std::vector<std::string>	methods;
	std::map<int, std::string>	redirection; // <error_code, text ou url>
	std::map<int, std::string>	error_page; // <error_code, error_uri_path>
}				t_directives;

class	Server
{
private :
	/* Server ID ***********************************************************************************/
	std::vector<t_listen>				_listen;
	std::vector<std::string>			_server_name;
	/* Configuration *******************************************************************************/
	t_directives						_directives;
	std::map<std::string, t_directives>	_locations; // <uri_path, directives>

public :
	Server();
	~Server();

	/* Member Functions ****************************************************************************/
	void								defaultConfiguration(t_directives server, t_directives &location);
	void								defaultConfiguration();

	/* Setters *************************************************************************************/
	int									setListen(const std::string &arg);
	void								setServerName(const std::vector<std::string> &names);
	int									setOneDirective(const std::string &type, const std::vector<std::string> &arg, t_directives *container);
	int									setLocation(const std::string &loc_path, const std::string &type, const std::vector<std::string> &arg);
	void								setClientMaxBodySize(const int &value, t_directives &dir);
	void								setRoot(const std::string &root, t_directives &dir);
	void								setIndex(const std::vector<std::string> &index, t_directives &dir);
	void								setMethods(const std::vector<std::string> &methods, t_directives &dir);
	
	/* Getters *************************************************************************************/
	const std::vector<t_listen>&		getListen() const;
	const std::vector<std::string>&		getServerName() const;
	const t_directives&					getDirectives() const;
	t_directives&						getDirectives();
	const std::map<std::string, t_directives>&	getLocations() const;
};

/* Operator Overload *******************************************************************************/
std::ostream&	operator<<(std::ostream &os, const Server &src);
void			print_directives(std::ostream &os, const t_directives &directives);

#endif