#include "webserv.hpp"

void	signal_handler(int signal)
{
	if (signal == SIGINT && g_global_instance != NULL)
		g_global_instance->stop("");
	exit(EXIT_SUCCESS);
}