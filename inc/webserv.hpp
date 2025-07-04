#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define OK 0
# define ERROR -1
# define INVALID -1
# define MAX_EVENTS 10
# define TIMEOUT -1

# define RED "\e[31m"
# define GREEN "\e[32m"
# define BLUE "\e[34m"
# define RESET "\e[0m"

# include <iostream>
# include <cstring>
# include <cstdlib>
# include <cstdio>
# include <csignal>
# include <vector>
# include <fstream>
# include <string>
#include <fcntl.h>

# include "Server.hpp"
# include "parser.hpp"

class	Server;

extern Server	*g_server_instance;

void	signal_handler(int signal);
int		add_fd_to_epoll(int epoll_fd, int fd);

#endif