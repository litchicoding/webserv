/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeutsch <sdeutsch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 16:37:46 by sdeutsch          #+#    #+#             */
/*   Updated: 2025/08/22 19:00:54 by sdeutsch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int add_fd_to_epoll(int epoll_fd, int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == INVALID)
	{
		std::cout << RED << "Error: fcntl()" << RESET << std::endl;
		close(fd);
		return -1;
	}
	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
		std::cout << RED << "Error: epoll_ctl()" << RESET << std::endl;
		close(fd);
		return -1;
	}
    return 0;
}

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string stripQueryString(const std::string &uri)
{
    size_t pos = uri.find('?');
    if (pos != std::string::npos)
        return uri.substr(0, pos);
    return uri;
}

void	signal_handler(int signal)
{
	if (signal == SIGINT && g_global_instance != NULL)
		g_global_instance->stop("");
	g_global_instance = NULL;
	// exit(EXIT_SUCCESS);
}