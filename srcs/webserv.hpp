#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
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
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <algorithm>
#include <cctype>
#include "Request.hpp"
#include "Location.hpp"


#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define NORMAL "\x1b[0m"
#define NOCOLOR "\033[0m" 

class Server;

typedef std::basic_stringstream<char> stringstream;

enum DT {CURRENT, LAST_MODIFIED};


void								*get_in_addr(struct sockaddr *address);
std::map<std::string, std::string>	file_extensions_map(void);
std::string							get_extension(std::map<std::string, std::string> &file_extensions, std::string full_path);
unsigned long						file_byte_dimension(std::string full_path);
std::string							file_content(std::string full_path, int from_php);
std::string							dt_string(std::string full_path, DT which);
std::string							get_response(std::string full_path, std::string req_uri, std::string http_v, int status, int from_php);
void								cleanup(int);
int									found_file(std::string path);
std::string							generate_error_page(std::string p_text, std::string code_response);
std::string							split(std::string &src, std::string delim);
void								display_map(std::map<std::string, std::string> map);
void								fill(std::vector<Server *> &servers, std::vector<struct pollfd> &all_pfds);
void								clean_exit(std::vector<Server *> &servers);
std::pair<int, int>					id_server(std::vector<Server *> &servers, int fd);
void								reset_revents(std::vector<struct pollfd> &all_pfds);

// cgi
std::string                     get_file_extension(std::string & uri);
std::pair<bool, std::string>    internal_server_error();
int                             fork_cgi_process(int tubes[2], int file_fd, char *cgi_args[3], char *env[13]);
void                            php_fill_env(Request & request, std::string path, char **env[10]);
std::pair<bool, std::string>    php_cgi(Request & request, std::string server_directory, std::string path, Location & location);
std::pair<bool, std::string>    internal_server_error();

// upload files
std::pair<bool, std::string>    forbidden_page();
std::pair<bool, std::string>    request_entity_too_large();
long                            get_max_body_size(Location & location);
std::pair<bool, std::string>    check_upload_file(Request & request, Location & location, std::string path);
std::pair<bool, std::string>    upload_file(Request & request, std::string upload_path, std::string path);

#endif
