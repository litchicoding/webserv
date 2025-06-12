# include "webserv.hpp"

Server	*g_server_instance = NULL;

int	main(int ac, char **av)
{
	if (ac == 2 && av[1])
		Server	server(av[1]);
	else if (ac == 1) {
		Server	server;
		server.start();
		server.update();
	}
	else {
		std::cout << RED << "Error: Too much argument" <<  RESET << std::endl;
		return ERROR;
	}
	return EXIT_SUCCESS;
}