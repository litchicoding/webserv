#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"

# define DEFAULT_PORT 8080
# define DEFAULT_ADDRESS_IP "0.0.0.0"
# define DEFAULT_SERVER_NAME ""
# define DEFAULT_BODY_SIZE 1048576
# define DEFAULT_ROOT "html"
# define DEFAULT_INDEX "index.html"
# define INCOMPLETE 2

# define AUTO_ON 1
# define AUTO_OFF 0

using namespace std;

typedef struct	s_listen
{
	int					port;
	string				ip;
	string				address_port;
}				t_listen;

typedef struct	s_directives
{
	int					autoindex;
	size_t				client_max_body_size;
	string				root;
	string				full_path;
	string				locationBlocName;
	string				query_string;
	vector<string>		index;
	vector<string>		methods;
	map<int, string>	redirection; // <code, text ou url>
	map<int, string>	error_page; // <error_code, error_uri_path>
}				t_directives;

class	Server
{
private :
/* Configuration *******************************************************************************/
	vector<t_listen>			_listen;
	t_directives				_directives;
	map<string, t_directives>	_locations; // <uri_path, directives>

public :
	Server();
	~Server();

	/* Member Functions ****************************************************************************/
	void						defaultLocConfiguration(t_directives &server, t_directives &location);
	void						defaultConfiguration();
	t_directives*				searchLocationMatch(const string &request_uri);

	/* Setters *************************************************************************************/
	int							setListen(const string &arg);
	int							setOneDirective(const string &type, const vector<string> &arg, t_directives *container);
	int							setLocation(const string &loc_path, const string &type, const vector<string> &arg);
	int							setRedirection(const vector<string> &arg, t_directives &dir);
	void						setClientMaxBodySize(const string &value, t_directives &dir);
	void						setRoot(const string &root, t_directives &dir);
	void						setIndex(const vector<string> &index, t_directives &dir);
	void						setMethods(const vector<string> &methods, t_directives &dir);
	
	/* Getters *************************************************************************************/
	const vector<t_listen>&		getListen() const;
	const t_directives&			getDirectives() const;
	t_directives&				getDirectives();
	const map<string, t_directives>&	getLocations() const;
};

/* Operator Overload *******************************************************************************/
ostream&	operator<<(ostream &os, const Server &src);
void		print_directives(ostream &os, const t_directives &directives);

#endif