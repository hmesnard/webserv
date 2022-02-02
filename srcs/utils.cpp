#include "webserv.hpp"
#include "Server.hpp"
#include <sstream>

void clean_exit(std::vector<Server *> &servers)
{
	unsigned int i = 0;
	while (i < servers.size())
	{
		delete servers[i];
		i++;
	}
}

void reset_revents(std::vector<struct pollfd> &all_pfds)
{
	size_t i = 0;

	while (i < all_pfds.size())
	{
		all_pfds[i].revents = 0;
		i++;
	}
}

//id of server, index in server
std::pair<int, int> id_server(std::vector<Server *> &servers, int fd)
{
	size_t i = 0;
	size_t j;
	while (i < servers.size())
	{
		j = 0;
		while (j < servers[i]->get_pfds().size())
		{
			if (servers[i]->get_pfds()[j].fd == fd)
				return (std::make_pair(i, j));
			j++;
		}
		i++;
	}
	return (std::make_pair(0, 0));
}

void fill(std::vector<Server *> &servers, std::vector<struct pollfd> &all_pfds)
{
	size_t i = 0;

	while (i < servers.size())
	{
		all_pfds.push_back(servers[i]->get_pfds()[0]);
		i++;
	}
}

template <typename T>
std::string itoa( T Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

std::string split(std::string &src, std::string delim)
{
	size_t pos = src.find(delim);
	std::string token = src.substr(0, pos);
	src.erase(0, pos + delim.length());
	return (token);
}

std::string generate_error_page(std::string p_text, std::string code_response)
{
	std::string body = 
"<!doctype html>\
<html>\
<head>\
<title>Nope</title>\
</head>\
<body>\
<p>" + p_text + "</p>\
</body>\
</html>";
	std::string headers = 
"HTTP/1.1 " + code_response + "\r\n\
Server: webserv\r\n\
Content-Type: text/html\r\n\
Content-Length: " + itoa(body.length());
headers += "\r\nConnection: Closed\r\n\r\n";
	return (headers + body);
}

std::string add_a_tag(std::string name)
{
	return ("<a href=\"" + name + "\">" + name + "</a>");
}

//path is the server's directory
std::string autoindex(std::string path, std::string uri)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return (std::string());
	struct dirent *current_entry;

	std::string body = 
"<!doctype html> \
<html>\
	<head>\
		<title>Index of " + uri + "</title>\
	</head>\
<body>\
<h1>Index of " + uri + "</h1><hr><pre>";

	while ((current_entry = readdir(dir)))
	{
		body += add_a_tag(current_entry->d_name);
		body += "</br>";
	}

	body += 
"</pre><hr>\
</body>\
</html>";
	return (body);
}

int found_file(std::string path)
{
	FILE *file_fd = fopen(path.c_str(), "r");
	if (!file_fd)
		return (0);
	fclose(file_fd);
	return (1);
}

void *get_in_addr(struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
		return &(((struct sockaddr_in *)address)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)address)->sin6_addr);
}

//associates a file extension with the proper Content-Type of an HTML response
std::map<std::string, std::string> file_extensions_map(void)
{
	std::map<std::string, std::string> ret;
	ret.insert(std::make_pair("html", "text/html"));
	ret.insert(std::make_pair("png", "image/png"));
	ret.insert(std::make_pair("jpg", "image/jpeg"));
	ret.insert(std::make_pair("css", "text/css"));
	ret.insert(std::make_pair("ico", "image/x-icon"));
	ret.insert(std::make_pair("bmp", "image/bmp"));
	ret.insert(std::make_pair("gif", "image/gif"));
	ret.insert(std::make_pair("mp3", "audio/mpeg"));
	ret.insert(std::make_pair("mpeg", "video/mpeg"));
	ret.insert(std::make_pair("pdf", "application/pdf"));
	ret.insert(std::make_pair("mp4", "video/mp4"));

	return (ret);
}

//gets file extension from a file path
std::string get_extension(std::map<std::string, std::string> &file_extensions, std::string full_path)
{
	std::string extension = full_path.substr(full_path.find_last_of(".") + 1);
	std::map<std::string, std::string>::iterator iter = file_extensions.find(extension);
	std::string a = iter->second;
	return (a);
}

unsigned long file_byte_dimension(std::string full_path)
{
	struct stat stat_buf;
	int rc = stat(full_path.c_str(), &stat_buf);
	//stat : on success, 0 is returned
	return (rc == 0 ? stat_buf.st_size : 0);
}

//return content of HTML_FILE as a std::string
std::string file_content(std::string full_path, int from_php)
{
	//create an input file stream to read our file and put it in our buffer
	std::ifstream content(full_path);
	stringstream buffer;

	std::string	line;
	std::string	parsed_php = "";

	if (!from_php) {
		buffer << content.rdbuf();
		return (buffer.str());
	}
	else {
		std::getline(content, line);
		while (line != "\r") {
			std::getline(content, line);
		}
		while (std::getline(content, line))
			parsed_php += line + "\n";
		return (parsed_php); 
	}
}

std::string time_headers(std::string &path)
{
	std::string buff;
	buff = "Date: ";
	buff += dt_string(path, CURRENT);
	buff += "\r\n";

	//get time of last modification of file
	buff += "Last modified: ";
	buff += dt_string(path, LAST_MODIFIED);
	buff += "\r\n";

	return (buff);
}

std::string dt_string(std::string full_path, DT which)
{
	char buff[1000];
	//we need the current time string
	if (which == CURRENT)
	{
		time_t now = time(0);
		struct tm ltm;
		ltm = *localtime(&now);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &ltm);
	}
	else //we need the last modified time string
	{
		struct stat a;
		if (stat(full_path.c_str(), &a) != 0)
			return (std::string());
		struct tm last_modified = *localtime(&a.st_mtime);
		strftime(buff, sizeof(buff), "%a, %d %b %Y %T %Z", &last_modified);
	}
	return (buff);
}

std::string get_response(std::string path, std::string req_uri, std::string http_v, int status, int from_php)
{
	//starts with HTTP/1.1 or HTTP/1.0
	std::string response;
	if (status == 405)
		response = "HTTP/1.1";
	else
		response = http_v;
	std::string body;
	std::string extension;

	// response status line
	if (status == 200)
		response += " " + itoa(status) + " OK\r\n";

	if (status == 404)
		response += " " + itoa(status) + " Page not found\r\n";

	if (status == 301)
	{
		response += " " + itoa(status) + " Moved permanently\r\n";
		//path is actually the full url requested 
		response += "Location: " + req_uri + "\r\n\r\n";
		return (response);
	}

	if (status == 1)
		response += " 200 OK\r\n";

	if (status == 405)
	{
		response += " " + itoa(status) + " Method not allowed\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Allow: ";
		if (path == "true")
			response += "GET, ";
		if (req_uri == "true")
			response += "POST, ";
		if (http_v == "true")
			response += "DELETE";
		if (response.substr(response.length() - 2) == ", ")
			response.resize(response.length() - 2);
		response += "\r\n\r\n";
		response += "<h1>405 Method Not Allowed</h1>";
		return (response);
	}

	response += time_headers(path);
	//name of the server and OS it's running on
	response += "Server: webserv\n\r";

	//time headers

	//Content-Type of file
	if (status == 1)
		response += "Content-Type: text/html\r\n";
	else
	{
		std::map<std::string, std::string> file_types = file_extensions_map();
		extension = get_extension(file_types, path);
		response += "Content-Type: ";
		response += extension;
		response += "\r\n";
	}


	//autoindex status
	if (status == 1)
	{
		body = autoindex(path, req_uri);
		response += "Content-Length: " + itoa(body.length());
		response += "\r\n";
		response += "Connection: Closed\r\n\r\n";
		response += body;
		return (response);
	}
	else
	{
		//get content of HTML file
		body = file_content(path, from_php);
		response += "Content-Length: ";
		response += itoa(body.length());
		response += "\r\n";
		response += "Connection: Closed\r\n";
		response += "Keep-Alive: timeout=100, max=100\r\n\r\n";
		response += body;
		return (response);
	}
}

