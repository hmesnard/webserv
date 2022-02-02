#include <unistd.h>
#include "Server.hpp"
#include "webserv.hpp"

std::string    get_file_extension(std::string & uri)
{
    int last_dot_position;

    last_dot_position = uri.find_last_of(".");
    if (last_dot_position <= 0)
        return (std::string());
    return (uri.substr(last_dot_position + 1));
}

std::pair<bool, std::string> Server::treat_post_request(Request & request, Location &location, std::string path, std::string server_directory)
{
    std::string http_response;

    if (get_file_extension(request.uri) == "php") {
        return (php_cgi(request, server_directory, path, location));
    }
    else if (request.headers["Content-Type"] == "multipart/form-data;") {
        return (check_upload_file(request, location, path));
    }
    else {
        return (treat_get_request(request, location, path, server_directory));
    }
}

