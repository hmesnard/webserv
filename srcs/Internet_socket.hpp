#ifndef INTERNET_SOCKET_HPP
#define INTERNET_SOCKET_HPP

#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>
#include <sstream>
#include <map>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"

#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"

#define NOCOLOR "\033[0m" 

#define NO_BOUND -1
#define LISTEN_FAIL -2

class Internet_socket
{
	private:
		struct sockaddr_in ip_port;
		struct addrinfo	hints;
		int				socket_fd;
		std::string		_service;

	public:
		Internet_socket();
		Internet_socket(Internet_socket const &cpy);
		~Internet_socket();

//		___________OPERATORS___________

		Internet_socket &operator=(Internet_socket const &cpy);


//		___________GETTER/SETTERS___________

		int				bind_listen(const char* hostname = NULL, const char *service = "80");
		struct addrinfo	get_hints(void);
		int				get_socket_fd(void);
		std::string		get_service(void);
		void			display_IP(void);
};

#endif
