#include "webserv.hpp"

void	signal_handler(int signal)
{
	if (signal == SIGINT)
		g_server_instance->stop("");
}