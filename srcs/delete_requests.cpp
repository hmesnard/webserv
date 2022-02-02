#include "Server.hpp"

std::string Server::treat_delete_request(std::string path)
{
	std::string response;
	if (remove(path.c_str()))
		return ("HTTP/1.1 204 OK\r\n\r\n");
	response = "HTTP/1.1 200 OK\r\n\r\n";
	response += 
"<html>\
  <body>\
    <h1>File deleted.</h1>\
  </body>\
</html>";
	return (response);
}
