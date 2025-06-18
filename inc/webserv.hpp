#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define OK 0
# define SUCCESS 0
# define ERROR -1
# define INVALID -1

# define RED "\e[31m"
# define GREEN "\e[32m"
# define BLUE "\e[34m"
# define RESET "\e[0m"

# include <iostream>
# include <cstring>
# include <cstdlib>
# include <cstdio>
# include <csignal>

# include <fstream>
# include <sstream>
# include <vector>
# include <map>
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>

// # include "Server.hpp"

// class	Server;
class   Client;

// extern Server	*g_server_instance;

// void	signal_handler(int signal);

#endif