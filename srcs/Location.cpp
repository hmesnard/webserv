#include "Location.hpp"

Location::Location()
{
	//defaults to off
	location_map.insert(std::make_pair("autoindex", "off"));
	//defaults to the default directory
	location_map.insert(std::make_pair("root", "."));
	//if this is something else than true, it's automatically false
	location_map.insert(std::make_pair("GET", "true"));
	location_map.insert(std::make_pair("POST", "true"));
	location_map.insert(std::make_pair("DELETE", "false"));
	//defaults to index.html
	location_map.insert(std::make_pair("index", "index.html"));
	location_map.insert(std::make_pair("return", ""));
	//defaults to 404.html
	location_map.insert(std::make_pair("error_page", "404.html"));
	location_map.insert(std::make_pair("cgi_path", ""));
	location_map.insert(std::make_pair("upload_path", ""));
	location_map.insert(std::make_pair("client_max_body_size", ""));
}

Location::Location(std::string new_url)
{
	prefix = new_url;
	
	//defaults to off
	location_map.insert(std::make_pair("autoindex", "off"));
	//defaults to the default directory
	location_map.insert(std::make_pair("root", "."));
	//if this is something else than true, it's automatically false
	location_map.insert(std::make_pair("GET", "true"));
	location_map.insert(std::make_pair("POST", "true"));
	location_map.insert(std::make_pair("DELETE", "false"));
	//defaults to index.html
	location_map.insert(std::make_pair("index", "index.html"));
	location_map.insert(std::make_pair("return", ""));
	//defaults to 404.html
	location_map.insert(std::make_pair("error_page", "404.html"));
	location_map.insert(std::make_pair("cgi_path", ""));
	location_map.insert(std::make_pair("upload_path", ""));
	location_map.insert(std::make_pair("client_max_body_size", ""));
}


Location &Location::operator=(Location const &cpy)
{
	if (this != &cpy)
	{
		location_map = cpy.location_map;
		prefix = cpy.prefix;
	}
	return (*this);
}
