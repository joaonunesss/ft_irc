/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rumachad <rumachad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 12:23:16 by rumachad          #+#    #+#             */
/*   Updated: 2024/10/02 16:55:87by rumachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::should_end = false;

/* Constructors/Destructors */
Server::Server(int port, std::string pass) : active_fd(1)
{
	/*std::cout << "Server port Constructor" << std::endl;*/
	std::memset(&this->_address, 0, sizeof(this->_address));
	this->_address.sin_family = AF_INET;
	this->_address.sin_port = htons(port);
	this->_password = pass;
	this->_commands["JOIN"] = new Join(*this); //JOIN <channel>
	this->_commands["WHO"] = new Who(*this);   //who
	this->_commands["MODE"] = new Mode(*this); //MODE <channel> +/- <mode>  || MODE <channel> +/- <mode> <nickname>
	this->_commands["NICK"] = new Nick(*this); //NICK <new_name>
	this->_commands["QUIT"] = new Quit(*this); //QUIT :<msg>
	this->_commands["PRIVMSG"] = new PrivMsg(*this); // PRIVMSG <name> <msg> || PRIVMSG <channel> <msg>
	this->_commands["KICK"] = new Kick(*this); //KICK <channel> <nickname> :<reason> || KICK <channel> <nickname>
	//whois whois <nick>
	//part PART <channel> :<msg>
	//topic TOPIC <channel> || TOPIC <channel> <new_topic>
	//invite INVITE <nick> <channel>
	//pass? PASS <password>
	//ping? PING <>
	//pong? PONG <>
}

Server::~Server()
{
	for (std::map<std::string, ACommand *>::iterator it = this->_commands.begin(); it != this->_commands.end(); ++it)
		delete it->second;
	/*std::cout << "Server Destructor" << std::endl;	*/
}

/* -------------------------------------------- */

int Server::create_server()
{
	int on = 1;
	pollfd sock;

	sock.fd = socket(this->_address.sin_family, SOCK_STREAM, 0);
	if (sock.fd == -1)
		print_error("Socket Error");
	if (setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)))
		print_error("setsockopt Error");
	if (bind(sock.fd, (struct sockaddr *)&this->_address, 
				sizeof(this->_address)) != 0)
		print_error("Bind Error");
	if (listen(sock.fd, 5) == -1)
		print_error("Listen Error");
	sock.events = POLLIN;
	this->fds.push_back(sock);
	return (EXIT_SUCCESS);
}

void Server::close_all_fds()
{
	for (it_fd it = this->fds.begin(); it != this->fds.end(); it++)
		close(it->fd);
	this->fds.erase(this->fds.begin(), this->fds.end());
}

pollfd Server::connect_client()
{
	pollfd		client;
	sockaddr_in	client_info;
	socklen_t	len = sizeof(client_info);

	client.fd = accept(this->fds[0].fd, (struct sockaddr *)&client_info, &len);
	if (client.fd == -1)
		print_error("Accept Error");
	client.events = POLLIN;
	this->_clients[client.fd] = User(inet_ntoa(client_info.sin_addr));
	this->active_fd++;
	std::cout << "New client " << this->active_fd << " connected" << std::endl;
	return (client);
}

void Server::receive_msg(it_user user)
{
	int msg_bytes;
	char buffer[1024] = {0};
	msg_bytes = recv(user->first, buffer, sizeof(buffer), 0);

	if (msg_bytes == -1)
		print_error("recv Error");

	user->second.set_buffer(buffer);
	std::cout << buffer;	//printa msg do client para o server
}

void Server::send_msg(it_user msg_sender, int i)
{
	for (it_user user = this->_clients.begin(); user != this->_clients.end(); user++)
	{
		if (user != msg_sender && i == 0)
			send(user->first, msg_sender->second.ge                                                          t_buffer().c_str(),
				msg_sender->second.get_buffer().length(), 0);
		else if (i == 1)
			send(user->first, msg_sender->second.get_buffer().c_str(),
				msg_sender->second.get_buffer().length(), 0);
		else if (i == 2)
		{
			User user;
			std::string msg01 = std::string(":") + user.get_hostname() + " " + RPL_WELCOME + " " + user.get_nick() + " :Welcome to the " + SERVER_NAME + " Internet Relay Network, " + user.get_nick() + "!\r\n";
			std::string msg02 = std::string(":") + user.get_hostname() + " " + RPL_YOURHOST + " " + user.get_nick() + " :Your host is " + _serverHostname + ", running version v0.1\r\n";
			std::string msg03 = std::string(":") + user.get_hostname() + " " + RPL_CREATED + " " + user.get_nick() + " :This server was created " + std::asctime(std::localtime(&_serverCreationTime));
			std::string msg04 = std::string(":") + user.get_hostname() + " " + RPL_MYINFO + " " + user.get_nick() + " " + _serverHostname + " v0.1 o iklt\r\n";

			send(user->first, msg01.c_str(), msg01.length(), 0); //user->first deveria ser socket
			send(user->first, msg02.c_str(), msg02.length(), 0);
			send(user->first, msg03.c_str(), msg03.length(), 0);
			send(user->first, msg04.c_str(), msg04.length(), 0);
		}
	}
}

void Server::msg_user(const int receiver_fd, User &msg_sender)
{
	send(receiver_fd, msg_sender.get_buffer().c_str(), msg_sender.get_buffer().length(), 0);
}

bool	Server::check_password(User &user)
{
	if (_password == user.getPassword())
		return true;
	return false;
}

int Server::main_loop()
{
	int ret;

	while (!should_end)
	{
		std::vector<pollfd> tmp;
		ret = poll(this->fds.data(), this->active_fd, -1);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			std::perror("Poll Error");
			break;
		}
		if (ret == 0)
		{
			std::cout << "Pool timeout" << std::endl;
			return (EXIT_FAILURE);
		}
		for (it_fd it = this->fds.begin(); it != this->fds.end(); it++)
		{
			if (it->revents == 0)
				continue;
			if (it->revents != POLLIN)
				print_error("Error revents");
			if (it->fd == this->fds[0].fd)
			{
				tmp.push_back(this->connect_client());
				it = this->fds.begin();
			}
			else
			{
				it_user user = advance_map(this->_clients, it->fd);
				this->receive_msg(user);
				if (user->second.get_info())
					break;
				if (!check_password(user)) //Rui vai mudar it_user para Class User
					return EXIT_FAILURE;
				welcome_message(user); //Rui vai mudar it_user para Class User
				try
				{
					// Parser::parser(user->second.get_buffer());
					if (this->handle_commands(user))
						break;
				}
				catch (std::exception &e)
				{
					std::cerr << "Command Not Found" << std::endl;
				}
			}
		}
		this->fds.insert(this->fds.end(), tmp.begin(), tmp.end());
	}
	return (0);
}

int Server::handle_commands(it_user &user)
{
	const std::string msg = user->second.get_buffer();
	const std::string command_name = msg.substr(0, msg.find_first_of(" "));
	std::cout << "CMD: " << command_name << std::endl;
	const size_t command_name_len = command_name.length() + 1;

	if (command_name.compare("CAP") == 0 || command_name.compare(".") == 0
		|| command_name.compare(" .") == 0)
		return (0);
	ACommand * command = this->_commands.at(command_name);

	command->set_args(msg.substr(command_name_len, msg.length() - command_name_len));
	command->set_user(user);
	if (command->run())
		return (1);
	return (0);
}

it_user Server::get_user(const std::string &nick)
{
	it_user it;
	
	for (it = this->_clients.begin(); it != this->_clients.end()
		&& it->second.get_nick().compare(nick) != 0; it++)
		;
	return (it);
}

void Server::disconnect_user(it_user &user)
{
	close(user->first);
	this->active_fd--;
	this->_clients.erase(user);
	this->fds.erase(find_fd(this->fds, user->first));
}

it_fd find_fd(std::vector<pollfd> &vec, const int fd)
{
	it_fd it;
	for (it = vec.begin(); it->fd != fd; it++)
		;
	return (it);
}
