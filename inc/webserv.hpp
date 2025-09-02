#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <cstring>
# include <cstdlib>
# include <cstdio>
# include <csignal>
# include <string>
# include <set>

# include <fstream>
# include <sstream>

# include <vector>
# include <map>

# include <dirent.h>
# include <unistd.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/epoll.h>
# include <sys/types.h>
# include <netdb.h>
# include <algorithm>
#include <sys/wait.h>

# include "HTTPRequest.hpp"
# include "Server.hpp"
# include "Client.hpp"
# include "Listen.hpp"
# include "parser.hpp"

# define DEFAULT_CONFIG_FILE "./config/webserv.conf"

# define OK 0
# define SUCCESS 0
# define ERROR -1
# define INVALID -1
# define MAX_EVENTS 10
# define TIMEOUT 5000

# define RED "\e[31m"
# define GREEN "\e[32m"
# define YELLOW "\e[1;33m"
# define BLUE "\e[1;34m"
# define CYAN "\e[1;36m"
# define PURPLE "\e[1;44m"
# define RESET "\e[0m"

extern Listen	*g_global_instance;

using namespace std;

class	Server;
class   Client;
class   HTTPRequest;

void		signal_handler(int signal);
int			add_fd_to_epoll(int epoll_fd, int fd);
std::string trim(const std::string &s);
std::string stripQueryString(const std::string &uri);

#endif