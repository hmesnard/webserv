#include "webserv.hpp"
#include "Server.hpp"
#include "Rules.hpp"

// A SERVER HAS :
// 1. an ip:port
//		Do we have to do IPv4 and IPv6 ?
// 2. a server_name
// 3. directives on how to handle files, requests, error pages, etc.
// 4. established connections
// 5. A listening socket
// Each one has to work in a different manner, but they can have the same ip and port. 
// Just not the same server_name, if they do, only the first one will be taken into account.
// SO they can share the same socket file descriptor.



//_____________HTTP webserver logic:_____________
//
//1. parse configuration file if given, if not, have a default path to look into. Base the parsing on the "server" part of Nginx's conf file
//		see http://nginx.org/en/docs/http/ngx_http_core_module.html for documentation and response statuses on nginx directives
//		- Chose the host:port of each "server"												listen
// 		- Setup of the server_name															server_name
// 		- The first server for a host:port will be the default server for this host:port.
// 		(it will answer all the requests that don't belong to another server)
// 		- Limiting the size of the body for the clients										client_max_body_size
//		- Setup of default error pages														error_page
//		- Setup of the routes with one or multiple rules (routes won't be using regexes)	location
//				--> see pdf
//		- CGI execution for certain file extensions
//2. from this configuration file: 
//		a) For each specified port and IP, create a bound socket file descriptor on the correct ip and port.
//		b) For each server name on the same port and IP, have a different array of pfds that store the incoming connections on these "servers"
//		c) set the working directory to the correct one where files need to be fetched
//		d) CGI settings
//		e) 
//3. listen for incoming connections
//4. Wait
//5. When a connection is received, wait for the fd to be ready to read the request
//6. Wait
//7. When a fd is ready to read, parse the request
//8. Check the request's header Host field and decide to which server_name the request should be routed to. 
//	 If its value does not match any server name, or the request doesn't contain this field at all,
//	 then route the request to the default server on this port.
//9. See if the request is correctly formulated, if not send a 400 HTTP response
//10. Is it a GET, POST, DELETE request ? 
//		a) GET : If it is correctly formulated see what page was requested : is it an HTML ? or do we have to use Common Gateway Interface
//			If CGI is needed, then fork the process to execute CGI ?? 
//		b) POST :
//		c) DELETE :
//11. Check to see if fd is ready to write. (POLLOUT) Send response with chunked encoding ? If it's HTTP 1.0 then not, otherwise we might need to
//12. Shutdown connection and close fd ??
//

//TODO


char G_QUIT = 0;

void cleanup(int)
{
	G_QUIT = 1;
}

int webserv(std::vector<Server *> &servers, std::vector<struct pollfd> &all_pfds)
{
	size_t i;
	int poll_count;
	int recv = 0;

	while (1)
	{
		i = 0;

		//poll our vector of fds
		poll_count = poll(all_pfds.data(), all_pfds.size(), -1);

		if (poll_count == -1 && !G_QUIT)
		{
			std::cerr << "poll error" << std::endl;
			return (0);
		}
		//go through our array to check if one fd is ready to read
		while (i < all_pfds.size())
		{
			if (all_pfds[i].revents & POLLIN) //one is ready
			{
				//get the index of the corresponding server that should treat the request
				std::pair<int, int> id_index = id_server(servers, all_pfds[i].fd);
				recv = servers[id_index.first]->inc_data_and_response(all_pfds, i, id_index.second);
				if (recv == -1)
					break ;
			}
			// std::cout << "Opened sockets : " << all_pfds[i].fd << std::endl;
			i++;
		}
		if (recv == -1)
			continue ;
		i = 0;
		while(i < all_pfds.size())
		{
			if (all_pfds[i].revents & POLLOUT)
			{
				std::pair<int, int> id_index = id_server(servers, all_pfds[i].fd);
				if ((servers[id_index.first]->send_data(all_pfds, i, id_index.second) == -2))
					break;
			}
			i++;
		}
		if (G_QUIT == 1)
			return (1);
	}
	return (1);
}

int main(int argc, char *argv[])
{

	//input buffer
	std::string buff;

	//vector to hold pointers to each of our listening servers
	std::vector<Server *> servers;

	//avoid quitting program without being able to cleanup the vector and disconnecting the sockets. Really necessary ?

	signal(SIGINT, &cleanup);

	//if we have a path to a configuration file then parse it, otherwise parse the default 
	//configuration file in conf.d/. If the argument provided wasn't an openable file 
	// or, if not provided, the default config file wasn't openable, return an error
	

	if (argc == 2 || argc == 1)
	{
		int a;
		std::string conf_path;
		if (argc == 2)
			conf_path = argv[1];
		else
			conf_path = "conf.d/webserv.conf";
		a = conf_file(conf_path, servers);
		if (a <= 0)
		{
			if (a == -1)
				std::cerr << RED << BOLD << "Config file is empty" << NOCOLOR << NORMAL << std::endl;
			else
				std::cerr << RED << BOLD << "Config file not found" << NOCOLOR << NORMAL << std::endl;
			return (1);
		}
	}
	else
	{
		std::cerr << RED << BOLD << "Wrong number of arguments" << NOCOLOR << NORMAL << std::endl;
		return (1);
	}

	if (servers.empty())
	{
		std::cerr << RED << BOLD << "Configuration file holds no valid server configuration" << NOCOLOR << NORMAL << std::endl;
		return (1);
	}

	std::vector<struct pollfd> all_pfds;

	//fill the all_pfds vector of pfds with the listening fds of the servers
	fill(servers, all_pfds);

	//run the main webserver loop
	webserv(servers, all_pfds);

	//exit on CTRL-C, cleanup the server vector
	clean_exit(servers);

	return (0);
}
