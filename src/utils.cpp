/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: knavarre <knavarre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 16:37:46 by sdeutsch          #+#    #+#             */
/*   Updated: 2025/06/20 21:49:28 by knavarre         ###   ########.fr       */
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

