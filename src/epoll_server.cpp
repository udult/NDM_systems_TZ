// server
#include <ctime>
#include <cstdio>
#include <cstring>

#include <string>
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>

#include <sys/epoll.h>
#include <sys/socket.h>

#define PORT 2000
#define IP "127.0.0.1"

#define QUEUE_LIM 10
#define EPOLL_LIM 10

std::string get_data_time();
std::string command_handle(std::string, int, int);

int main()
{
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_fd == -1)
	{
		perror("socket tcp");
		return 1;
	}

	int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_fd == -1)
	{
		perror("socket udp");
		return 1;
	}

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(PORT);

	if (bind(tcp_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("bind tcp");
		return 1;
	}

	if (bind(udp_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("bind udp");
		return 1;
	}

	if (listen(tcp_fd, QUEUE_LIM) == -1)
	{
		perror("listen");
		return 1;
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		return 1;
	}

	struct epoll_event event;

	event.events = EPOLLIN;
	event.data.fd = tcp_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_fd, &event) == -1)
	{
		perror("epoll_ctl tcp");
		return 1;
	}

	event.data.fd = udp_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_fd, &event) == -1)
	{
		perror("epoll_ctl udp");
		return 1;
	}

	std::cout << "Server is running at " << IP << ":" << PORT << std::endl;

	int total  = 0;
	int active = 0;

	struct epoll_event events[EPOLL_LIM];

	while (true)
	{
		memset(events, 0, sizeof(events));

		int ready_fds = epoll_wait(epoll_fd, events, EPOLL_LIM, -1);
		if (ready_fds == -1)
		{
			perror("epoll_wait");
			return 1;
		}

		for (int i = 0; i < ready_fds; i++)
		{
			int sockfd = events[i].data.fd;

			if (events[i].events & (EPOLLRDHUP | EPOLLERR))
			{
				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, NULL) == -1)
				{
					perror("epoll_ctl del");
					return 1;
				}

				close(sockfd);
				std::cout << "Client is disconnected (fd: " << sockfd << ")" << std::endl;
				active--;
			}

			else if (events[i].events & EPOLLIN)
			{
				if (sockfd == tcp_fd)
				{
					int client_fd = accept(tcp_fd, NULL, NULL);
					if (client_fd == -1)
					{
						perror("accept");
						continue;
					}

					total++;
					active++;

					std::cout << "New client is accepted (fd: " << client_fd << ')' << std::endl;

					struct epoll_event event;

					event.events  = EPOLLIN | EPOLLRDHUP | EPOLLERR;
					event.data.fd = client_fd;

					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
						perror("epoll_ctl add");
				}
					
				else if (sockfd == udp_fd)
				{
					std::string reply;
					char buffer[1024] = {0};

					struct sockaddr_in client_addr;
                	socklen_t addrlen = sizeof(client_addr);
				
            	   	recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &addrlen);

                	if (buffer[0] == '/') reply = command_handle(buffer, active, total);
                	else reply = buffer;

                	sendto(sockfd, reply.data(), reply.size(), 0, (struct sockaddr*) &client_addr, addrlen);
				}

				else
				{
					std::string reply;
					char buffer[1024] = {0};

					recv(sockfd, buffer, sizeof(buffer), 0);

					if (buffer[0] == '/') reply = command_handle(buffer, active, total);
                	else reply = buffer;

					send(sockfd, reply.data(), reply.size(), 0);
				}
			}
		}
	}

	return 0;
}

std::string command_handle(std::string cmd, int active, int total)
{
	size_t pos = cmd.find_last_not_of(" \t\r\n");
	
	if (pos != std::string::npos)
		cmd.erase(pos + 1);

	if (cmd == "/time")
		return get_data_time();

	if (cmd == "/stats")
		return "clients total: " + std::to_string(total) + '\n' + "clients in lobby: " + std::to_string(active);

	if (cmd == "/shutdown")
	{
		std::cout << "shutdown command has been received" << std::endl;
		exit(EXIT_SUCCESS);
	}

	return "Invalid command";
}

std::string get_data_time()
{
	time_t now = time(NULL);
	tm *data_time = localtime(&now);

	std::stringstream ss;

	ss << data_time->tm_year + 1900 << '-' << data_time->tm_mon + 1 << '-' << data_time->tm_mday << ' '
	   << data_time->tm_hour << ':' << data_time->tm_min << ':' << data_time->tm_sec;

	return ss.str();
}

/* */