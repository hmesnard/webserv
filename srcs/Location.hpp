#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <map>
#include <string>

class Location
{
	private :
	public :
		std::string prefix;
		std::map<std::string, std::string> location_map;


		Location();
		Location(std::string prefix);
		~Location(){};

		Location &operator=(Location const &cpy);
};

#endif
