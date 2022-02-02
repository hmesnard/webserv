#include "webserv.hpp"
#include "Rules.hpp"
#include "Server.hpp"

//TODO ON PARSING
//================

void	display_map(std::map<std::string, std::string> map)
{
	std::map<std::string, std::string>::iterator iter = map.begin();
	std::cout << "Rules of server : " << std::endl;
	std::cout << "-----------------" << std::endl;
	unsigned int longest_key = 0;
	while (iter != map.end())
	{
		if (iter->first.length() > longest_key)
			longest_key = iter->first.length();
		iter++;
	}
	iter = map.begin();
	while (iter != map.end())
	{
		unsigned int a = iter->first.length();
		std::string b = " --> ";
		while (a < longest_key + 4)
		{
			b += " ";
			a++;
		}
		std::cout << iter->first << b << iter->second << std::endl;
		iter++;
	}
	std::cout << std::endl;
}

//get a string from open curly brace to closing
std::pair<std::string, int> get_location_block(std::string file, int i)
{
	int begin = i;
	while (file[i] != '}' && file[i])
		i++;
	//returns the location block string as well as the position of the end of it
	return (std::make_pair(file.substr(begin, i - begin), i)); 
}

std::pair<std::string, int> next_word(std::string str, size_t i, bool a, const char *skip_chars)
{
	size_t k;
	size_t size;
	//skip whitespaces until a word is found
	while (i < str.length() && isspace(str[i]))
		i++;
	//get starting position of word
	k = i;

	//get ending position of word
	while (i < str.length() && str[i] != skip_chars[0] && str[i] != skip_chars[1] && !(a && isspace(str[i])))
		i++;

	size = i - k;

	//check if the word matches one of the directives that we allow 
	//if it does add it to our map of location rules
	std::string keyword = str.substr(k, size);
	return (make_pair(keyword, i));
}

//function has to return the number of characters we treated from the server block
int treat_location(Rules &new_rules, std::string server_block, int start)
{
	unsigned int i = 0;

	//string that only holds the current location directive
	std::pair<std::string, int> a = get_location_block(server_block, start);
	std::string location_string = a.first;
	unsigned int block_end = a.second;

	//sets the url while constructing our empty map of location rules. We need to fill our map
	Location new_location(next_word(location_string, 0, true, "{;").first);

	//if no url was specified, skip that block
	if (new_location.prefix == "")
		return (block_end);

	while (i < location_string.length())
	{
		std::pair<std::string, int> a = next_word(location_string, i, true, "\0\0");
		std::string keyword = a.first;
		i = a.second;

		//go through our map of location rules, if a key matches the word we found, assign a new value to it
		std::map<std::string, std::string>::iterator iter = new_location.location_map.begin();
		while (iter != new_location.location_map.end())
		{
			if (keyword == iter->first)
			{
				if (keyword == "return")
					iter->second = next_word(location_string, i, false, ";\0").first;
				else
					iter->second = next_word(location_string, i, true, "{;").first;
			}
			iter++;
		}
	}
	if (new_location.prefix.back() == '/')
		new_location.prefix.resize(new_location.prefix.size() - 1);	
	new_rules.locations.push_back(new_location);
	return (block_end);
}


Rules parse_server(std::string &server_block)
{
	unsigned int i = 0;
	Rules new_rules;

	while (server_block[i])
	{
		std::pair<std::string, int> a = next_word(server_block, i, true, "\0\0");
		std::string directives_keyword = a.first;
		i = a.second;

		//check if the word matches one of the directives that we allow 
		//if it does and it's not location, store the next word in the second item of the pair
		//from our directives map
		if (directives_keyword == "location")
		{
			//get a new Location set of rules from of location block and push it in our vector of locations rules for this server block
			i = treat_location(new_rules, server_block, i);
			continue ;
		}
		//go through our map of general server rules, if a key matches the word we found, assign a new value to it
		std::map<std::string, std::string>::iterator iter = new_rules.directives.begin();
		while (iter != new_rules.directives.end())
		{
			if (directives_keyword == iter->first)
				iter->second = next_word(server_block, i, true, "{;").first;
			iter++;
		}
	}
	return (new_rules);
}

//return the index of the start of the server block (right after the opening curly brace, so we're sure it's there)
int server_keyword(std::string const &file)
{
	unsigned int i = 0;

	while (i < file.length())
	{
		std::pair<std::string, int> a = next_word(file, i, true, "\0\0");
		std::string keyword = a.first;
		i = a.second;
		
		//get the substring from the begining of the word to the end and compare it to "server"
		//if the first word is server, this means we might have a server block
		if (keyword == "server")
		{
			while (file[i] && isspace(file[i]))
				i++;
			if (file[i] && file[i] == '{')
				return (++i);
		}
		i++;
	}
	return (0);
}

std::string get_server_block(std::string &file)
{
	int server_begin;
	int i;
	unsigned char brace_flag;

	brace_flag = 0;
	server_begin = server_keyword(file);
	if (!server_begin)
		return (std::string());
	try
	{
		file = file.erase(0, server_begin);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	i = 0;
	while (file[i])
	{
		if (file[i] == '{')
			brace_flag = 1;
		if (file[i] == '}')
		{
			if (brace_flag)
				brace_flag = 0;
			else
				break;
		}
		i++;
	}
	return (file.substr(0, i));
}

bool	only_digits(std::string str)
{
	int i = 0;
	while (str[i])
	{
		if (!isdigit(str[i]))
			return (false);
		i++;
	}
	return (true);
}

std::pair<std::string, std::string> split_listen(Rules &rules)
{
	if (only_digits(rules.directives["listen"]))
		return (std::make_pair("", rules.directives["listen"]));
	else
	{
		//position of delimiter
		unsigned int pos = rules.directives["listen"].find(":");
		return (std::make_pair(rules.directives["listen"].substr(0, pos), rules.directives["listen"].substr(pos + 1, rules.directives["listen"].length())));
	}
}

//add a server to our vector of listening Servers, or a virtual server to a Listening server's vector
void add_server(std::vector<Server *> &servers, Rules &rules)
{
	unsigned int i = 0;

	//split the listen directive of our rule to get the port and IP separetely
	std::pair<std::string, std::string> IP_port = split_listen(rules);

	//if the IP is empty, this means that we'll listen to all the available interfaces on this port. 
	//An empty IP means listening to 0.0.0.0
	if (!IP_port.first.compare(""))
		rules.directives["listen"] = "0.0.0.0:" + IP_port.second;
	while (i < servers.size())
	{
		if (!servers[i]->get_rules().directives["listen"].compare(rules.directives["listen"]))
		{
			servers[i]->push_v_server(Virtual_server(rules));
			return ;
		}
		i++;
	}
	//get the IP and the PORT of the server
	//need to replace all correct values of the IP:port, and server_name here.
	if (!IP_port.first.compare(""))
		servers.push_back(new Server(IP_port.second.c_str(), "0.0.0.0"));
	else
		servers.push_back(new Server(IP_port.second.c_str(), IP_port.first.c_str()));
	servers[servers.size() - 1]->set_rules(rules);
}

//beginning of parsing

//fills a vector of Servers up with pointer to new Listening Servers
size_t conf_file(std::string path, std::vector<Server *> &servers)
{
	//our whole configuration file
	std::string file_string;

	Rules new_rules;

	//check if file exists, if it doesn't we can't launch the server and need to print the 
	//appropriate message;
	FILE *file_fd = fopen(path.c_str(), "r");
	if (!file_fd)
		return (0);
	fclose(file_fd);
	//get the content of the file as a string
	file_string = file_content(path, 0);
	if (file_string.empty())
		return (-1);
	while (1)
	{
		//get the next "server" text block from file. Deletes the content in the string before it
		std::string server_string = get_server_block(file_string);

		//this means no more server blocks where found
		if (server_string.empty())
			return (1);
		
		//parse the "server" block and get the rules from it
		new_rules = parse_server(server_string);
		add_server(servers, new_rules);
	}
	return (0);
}
