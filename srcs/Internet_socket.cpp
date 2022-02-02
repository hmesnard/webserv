#include "Internet_socket.hpp"
#include <string.h>

Internet_socket::Internet_socket()
{
	
}

Internet_socket::Internet_socket(Internet_socket const &cpy)
{
	*this = cpy;
}

Internet_socket::~Internet_socket()
{
	if (socket_fd > -1)
		close(socket_fd);
}

void Internet_socket::display_IP()
{
	void *addr;
	addr = &(ip_port.sin_addr);
	char addr_str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET,  addr, addr_str, sizeof(addr_str));
	std::cout << "IP : " << addr_str << std::endl;
}

int Internet_socket::bind_listen(const char* hostname, const char *service)
{
	struct addrinfo *res;
	struct addrinfo *iter;
	int yes = 1;

	_service = service;

	memset((void *)&hints, 0, sizeof(hints));
	//Take both IPv4 or IPv6
	hints.ai_family = AF_UNSPEC;

	//get a stream socket
	hints.ai_socktype = SOCK_STREAM;

	//hints.ai_protocol is set to 0 by bzero, so the protocol is "any" that fits the other criterias
	
	//get my IP automatically if the ip wasn't specified
	if (!hostname)
		hints.ai_flags = AI_PASSIVE;

	//stores a list of potentially usable addresses in res, we need to iterate through it
	//to find a valid address to create a socket and bind that socket
	getaddrinfo(hostname, service, &hints, &res);

	//iterate through all the addresses resulting from the call to getaddrinfo 
	//to try to get a socket a	nd bind it
	for (iter = res; iter != NULL; iter = iter->ai_next)
	{
		socket_fd = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if (socket_fd < 0)
			continue;

		fcntl(socket_fd, F_SETFL, O_NONBLOCK);

		// Stop the "address already in use" error message
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (bind(socket_fd, iter->ai_addr, iter->ai_addrlen) < 0)
		{
			close(socket_fd);
			continue;
		}
		break;
	}

	if (iter)
	{
		//display address bound to the socket
		if (iter->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter->ai_addr;
			void *addr;
			addr = &(ipv4->sin_addr);
			char addr_str[INET6_ADDRSTRLEN];
			inet_ntop(iter->ai_family,  addr, addr_str, sizeof(addr_str));
			std::cout << std::endl << GREEN << BOLD << "Server up and running" << NOCOLOR << NORMAL << std::endl;
			std::cout << "IP : " << addr_str << std::endl;
			std::cout << "Port : " << service << std::endl << std::endl;
		}
		else
		{
			std::cout << std::endl << BLUE << BOLD << "Server up and running" << NOCOLOR << NORMAL << std::endl;
			std::cout << "IPv6 address" << std::endl;
		}
	}
	else
		std::cout << RED << "No socket was connected for this server" << NOCOLOR << std::endl;


	// if we got to the end of the linked list, that means no socket_fd was binded
	if (iter == NULL)
	{
		socket_fd = NO_BOUND;
		return (NO_BOUND);
	}

	ip_port = *(struct sockaddr_in *)iter->ai_addr;
	freeaddrinfo(res);

	if (listen(socket_fd, 100000) == -1)
	{
		socket_fd = LISTEN_FAIL;
		return (LISTEN_FAIL);
	}
	return (0);
}

struct addrinfo Internet_socket::get_hints(void)
{
	return (hints);
}

int Internet_socket::get_socket_fd(void)
{
	return (socket_fd);
}

std::string Internet_socket::get_service(void)
{
	return (_service);
}


Internet_socket &Internet_socket::operator=(Internet_socket const &cpy)
{
	if (this != &cpy)
	{
		hints = cpy.hints;
		socket_fd = cpy.socket_fd;
		_service = cpy._service;
	}
	return (*this);
}
