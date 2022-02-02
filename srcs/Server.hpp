#ifndef SERVER_HPP
#define SERVER_HPP

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
#include "webserv.hpp"
#include "Request.hpp"

class Internet_socket;
class Virtual_server;

#include "Internet_socket.hpp"
#include "Rules.hpp"

enum REQ_METHOD{GET, POST, DELETE};

class Server
{
	typedef std::vector<struct pollfd>::size_type size_type;

	private:

		//out socket file descriptor class, holds addrinfo(IP), socket_fd and service (port)
		Internet_socket				sock;

		//Servers that have the same IP and port as the default server (first one in the conf file)
		//Other servers are just a different server_name and different set of rules. They get the 
		//request from the listening server if the Request's header's host field if the same as their
		//server_name. They then get routed the request as a string, treat it with their own rules and
		//return it as a string for the Server to send it back to the correct connection.
		std::vector<Virtual_server>	v_servers;
	
		//vector that holds all our fds, either our bound socket in index 0 or a remote connection
		std::vector<struct pollfd>	pfds;
		
	protected:

		//rule sets, determines how the Server or virtual server should treat the request
		Rules 						rule_set;
	
	public:
		Server(Internet_socket const &socket_fd);
		Server(const char* service = "80", const char* hostname = NULL);
		virtual ~Server(){};


//		___________GETTER/SETTERS___________

		Internet_socket				&get_sock(void);
		std::vector<Virtual_server>	&get_v_servers(void);
		std::vector<struct pollfd>	&get_pfds(void);
		Rules						&get_rules(void);
		void						set_rules(Rules &rules);

//		___________UTILS___________

		//add a new virtual server to our vector
		void							push_v_server(Virtual_server new_server);

		//add a new connection to our vector of fds
		void							push_fd(struct pollfd new_fd);
		void							push_fd(std::vector<struct pollfd> &all_pfds, int fd, int events);

		//remove a connection from our vector of fds
		void							pop_fd(void);
		
		size_t							vector_size();
		
		//poll our fds to see if a request came through from a connection and send the response if there was
		int								inc_data_and_response(std::vector<struct pollfd> &all_pfds, int all_index, int server_index);



		//treat the request according to the set of rules of our servers
		std::pair<bool, std::string>	build_http_response(Request &request);
		int								send_http_response(std::vector<struct pollfd> &all_pfds, int all_index, int server_index);
		int								receive_http_header(int i);
		std::pair<bool, std::string>	treat_request(Request &req);
		std::pair<bool, Location> 		match_location(std::string requested_page);
		void							display_IP(void);
		int								close_connection(std::vector<struct pollfd> &all_pfds, int all_index, int server_index);
		std::pair<bool, std::string>	treat_get_request(Request &req, Location &location, std::string path, std::string server_directory);
		std::pair<bool, std::string>	treat_post_request(Request & request, Location &location, std::string path, std::string server_directory);
		std::string						treat_delete_request(std::string path);
		int								send_all(int fd, std::vector<struct pollfd> &all_pfds, int all_index);
		int								send_data(std::vector<struct pollfd> &all_pfds, int all_index, int server_index);
		int								receive_http_body(int i);

};


class Virtual_server : public Server
{
	//inherited from Server :
	// Rules			rule_set;

	private:

	public:
		Virtual_server(){};
		Virtual_server(Rules const &serv_rules);
		virtual ~Virtual_server(){};

		Rules	&get_rules(void);
};

//fills a vector of Servers up with pointer to new Listening Servers
size_t conf_file(std::string path, std::vector<Server *> &servers);


#endif
