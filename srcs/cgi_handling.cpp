#include "webserv.hpp"
#include "Request.hpp"

void    php_fill_env(Request & request, std::string path, char *env[13])
{
	std::string env_vars[13];
	int         i;
	std::string filename;

	filename = path.substr(path.rfind("/") + 1);
	env_vars[0] = "CONTENT_LENGTH=" + request.headers["Content-Length"];
	env_vars[1] = "CONTENT_TYPE=" + request.headers["Content-Type"];
	env_vars[2] = "REQUEST_METHOD=POST";
	env_vars[3] = "REDIRECT_STATUS=200";
	env_vars[4] = "GATEWAY_INTERFACE=CGI/1.1";
	env_vars[5] = "SCRIPT_NAME=" + request.uri;
	env_vars[6] = "PATH_INFO=" + path;
	env_vars[7] = "PATH_TRANSLATED=" + path;
	env_vars[8] = "REMOTE_ADDR=" + request.headers["Origin"];
	env_vars[9] = "AUTH_TYPE=" + request.headers["Authorization"];
	env_vars[10] = "QUERY_STRING=" + request.query_string; // set only for get requests
	env_vars[11] = "REMOTE_USER=" + request.headers["Authorization"];

	i = 0;
	while (i < 12) {
		env[i] = new char[env_vars[i].size() + 1];
		strcpy(env[i], env_vars[i].c_str());
		++i;
	}
	env[i] = NULL;
}

std::pair<bool, std::string>    internal_server_error()
{
	std::string server_error = "500 Internal Server Error";
	return (std::make_pair(true, generate_error_page(server_error, server_error)));
}

int fork_cgi_process(int tubes[2], int file_fd, char *cgi_args[3], char *env[13])
{
	int         cgi_pid;
	int         exit_status = 0;
	int         status = 0;

	cgi_pid = fork();
	if (cgi_pid == -1)
		return (-1);
	if (cgi_pid == 0)
	{
		close(tubes[1]);
		// input from pipes, output to file
		if (dup2(tubes[0], 0) == -1 || dup2(file_fd, 1) == -1)
			exit(EXIT_FAILURE);
		if (execve(cgi_args[0], cgi_args, env) == -1)
			exit(EXIT_FAILURE);
	}
	if (waitpid(cgi_pid, &status, 0) == -1)
		return (-1);

	if (WIFEXITED(status))
	{
		exit_status = WEXITSTATUS(status);
		if (exit_status == EXIT_FAILURE)
			return (-1);
	}
	return (0);
}

std::pair<bool, std::string>    php_cgi(Request & request, std::string server_directory, std::string path, Location & location)
{
	char                *cgi_args[3];
	int                 tubes[2];
	int                 file_fd;
	std::string         cgi_output_path;
	char                *env[13];
	std::stringstream   ss_content_length(request.headers["Content-Length"]);
	unsigned int        content_length = 0;
	std::string         cgi_path;

	cgi_path = location.location_map["cgi_path"];
	ss_content_length >> content_length;
	cgi_output_path = server_directory + "cgi_output.html";

	cgi_args[0] = new char[cgi_path.size() + 1];
	strcpy(cgi_args[0], cgi_path.c_str());
	cgi_args[1] = new char[path.size() + 1];
	strcpy(cgi_args[1], path.c_str());
	cgi_args[2] = NULL;

	if ((file_fd = open(cgi_output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (internal_server_error());
	if (pipe(tubes) == -1)
		return (internal_server_error());

	// send post data to the cgi process
	write(tubes[1], request.data.c_str(), content_length);
	php_fill_env(request, path, env);

	if (fork_cgi_process(tubes, file_fd, cgi_args, env) == -1)
		return (internal_server_error());

	close(tubes[0]);
	close(tubes[1]);
	close(file_fd);
	delete [] cgi_args[0];
	delete [] cgi_args[1];
	for (int i = 0; i < 13 ; i++)
		delete env[i];
	
	std::string http_response = get_response(cgi_output_path, request.uri, request.protocol, 200, 1);
	return (std::make_pair(false, http_response));
}
