#include "Server.hpp"
#include "webserv.hpp"

//TODO

std::pair<bool, std::string> Server::treat_get_request(Request &req, Location &location, std::string path, std::string server_directory)
{
	(void)location;
	(void)server_directory;

    //treating HTTP/1.1 request
    if (req.protocol == "HTTP/1.1" || req.protocol == "HTTP/1.0")
    {
        if (get_file_extension(req.uri) == "php") {
            return (php_cgi(req, server_directory, path, location));
        }
        else {
            std::string http_response = get_response(path, req.uri, req.protocol , 200, 0);
            return (std::make_pair(true,http_response));
        }
    }
	else
		return (std::make_pair(true, generate_error_page("Unsupported HTTP version", "505 Version Not Supported")));
}

