#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>

class Request
{
    private:


    public:

        std::string                         type;
        std::string                         uri;
        std::string                         protocol;
        std::map<std::string, std::string>  headers;
        std::string                         data;
		std::string							query_string;

        Request();
        Request(std::string request);
        ~Request();
        void    print();
        void    fill_object(std::string full_request);

};

#endif
