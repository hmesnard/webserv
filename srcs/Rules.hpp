#ifndef RULES_HPP
#define RULES_HPP

#include <string>
#include <vector>
#include <map>
#include "Location.hpp"

class Rules
{
	private:

	public:
		//general set of rules
		std::map<std::string, std::string> directives;
		
		//vector of location directives
		std::vector<Location> locations;
		
		Rules();
		~Rules(){};
		Rules(Rules const &cpy);

		Rules &operator=(Rules const &cpy);


		//returns our map of general rules
		std::map<std::string, std::string> &get_directives(void);
		std::vector<Location> &get_locations(void);
};

#endif
