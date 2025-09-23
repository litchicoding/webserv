# include "webserv.hpp"

Listen	*g_global_instance = NULL;

int	main(int ac, char **av)
{
	Listen	listenPorts;
	string	config_file;

	if (ac > 2) {
		cout << RED << "Error: Too much argument" <<  RESET << endl;
		return ERROR;
	}
	if (!av[1])
		config_file = "";
	else
		config_file = av[1];
	if (parse_config_file(config_file, listenPorts) != OK) {
		cout << RED << "Can't continue: invalid format flagged in configuration file\n" << RESET;
		return ERROR;
	}
	g_global_instance = &listenPorts;
	listenPorts.configuration();
	// cout << listenPorts;
	listenPorts.debug = false;
	if (listenPorts.start_connection() != OK)
		return ERROR;
	listenPorts.update_connection();
	return EXIT_SUCCESS;
}