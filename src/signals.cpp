/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmarinho <jmarinho@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 13:30:02 by jmarinho          #+#    #+#             */
/*   Updated: 2024/10/08 17:10:34 by jmarinho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::signal_handler(int signum)
{
	Server serv;

	std::cout << std::endl;
	if (signum == SIGINT)
		std::cout << "Received signal Ctrl + C: Exiting server" << std::endl;
	else if (signum == SIGQUIT)
		std::cout << "Received signal Ctrl + \\: Exiting server" << std::endl;
	sleep(1);
	serv.close_fds();
		
}

void	Server::close_fds()
{
	std::cout << "Closing Sockets..." << std::endl;
	for (it_fd it = fds.begin(); it != fds.end(); it++)
		close(it->fd);
	sock.fd = 0;
	while (sock.fd)
	{
		std::cout << "sock.fd " << sock.fd << std::endl;
		close(sock.fd++);
	}
	should_end = true;
}