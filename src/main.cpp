# include "webserv.hpp"

Server	*g_server_instance = NULL;

int	main(int ac, char **av)
{
	std::vector<Server*>	serv_group;

	if (ac == 2 && av[1])
		parse_config_file(av[1], serv_group);
	else if (ac == 1)
		parse_config_file("./config/default.conf", serv_group);
	else {
		std::cout << RED << "Error: Too much argument" <<  RESET << std::endl;
		return ERROR;
	}
	// webserv loop = for each server in the serv_group start and update
	std::vector<Server*>::iterator current_serv = serv_group.begin();
	while (current_serv != serv_group.end())
	{
		(*current_serv)->start();
		current_serv++;
	}
	while (true)
	{
		current_serv = serv_group.begin();
		while (current_serv != serv_group.end())
		{
			(*current_serv)->update();
			current_serv++;
		}
	}
	Server::delete_server_group(serv_group);
	return EXIT_SUCCESS;
}