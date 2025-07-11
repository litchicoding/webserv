# include "webserv.hpp"

Listen	*g_global_instance = NULL;

int	main(int ac, char **av)
{
	Listen	listenPorts;

	if (ac > 2) {
		cout << RED << "Error: Too much argument" <<  RESET << endl;
		return ERROR;
	}
	if (av[1] && parse_config_file(av[1], listenPorts) != OK)
		return ERROR;
	else if (parse_config_file("", listenPorts) != OK)
		return ERROR;
	g_global_instance = &listenPorts;
	listenPorts.configuration();
	// cout << listenPorts;
	if (listenPorts.start_connexion() != OK)
		return ERROR;
	listenPorts.update_connexion();
	return EXIT_SUCCESS;
}