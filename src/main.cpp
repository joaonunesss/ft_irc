/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmarinho <jmarinho@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 11:43:48 by cacarval          #+#    #+#             */
/*   Updated: 2024/10/08 13:17:30 by jmarinho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"
#include "User.hpp"


void print_error(std::string error_msg)
{
	std::perror(error_msg.c_str());
	exit(EXIT_FAILURE);
}

int main(int argc, char  **argv)
{
	//#TODO got to handle ctrl+c e ctrl+\ (ver function signal_handler)
	
	if (argc < 2) 
	{
		std::cerr << "Usage: ./ircserv <port>" << std::endl;
		return EXIT_FAILURE;
	}
	Server irc_server(std::atoi(argv[1]));
	
	irc_server.create_server();
	irc_server.main_loop();
/*  Function to use the server with the pc ip
	if (inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0)
	{
		std::cout <<("Invalid address or address not supported") << std::endl;
		return EXIT_FAILURE;
	} */
	return(0);
}