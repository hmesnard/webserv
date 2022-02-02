#include "Request.hpp"

Request::Request() {}

Request::Request(std::string request)
{
    std::istringstream  iss(request);
    std::string         line;
    std::istringstream  line_stream;
    std::string         header_key;
    std::string         header_value;

    std::getline(iss, line);
    line_stream.str(line);
    line_stream >> this->type >> this->uri >> this->protocol;

    while (std::getline(iss, line))
    {
        if (line == "\r")
            break;
        line_stream.str(line);
        line_stream >> header_key >> header_value;
        header_key.resize(header_key.size() - 1);
        this->headers[header_key] = header_value;
        if (header_value == "multipart/form-data;") {
            line_stream >> header_value;
            this->headers["boundary"] =  header_value.substr(9);
        }
    }
    if (this->type == "POST")
        this->data = request.substr(request.find("\r\n\r\n") + 4);
	
	//get correct hostname without ip following
	headers["Host"] = headers["Host"].substr(0, headers["Host"].rfind(":"));

	//query strings
	size_t find = uri.rfind("?") + 1;
	if (find != 0)
	{
		query_string = uri.substr(find, std::string::npos);
		uri = uri.substr(0, find - 1);
	}
}

Request::~Request() {}

void    Request::print()
{
    std::map<std::string, std::string>::iterator    it = this->headers.begin();
    std::map<std::string, std::string>::iterator    ite = this->headers.end();

    std::cout << "-----PRINT REQUEST-----" << std::endl;
    std::cout << this->type << " " << this->uri << " " << this->protocol << std::endl;
    while (it != ite)
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    if (this->type == "POST")
        std::cout << std::endl << "DATA: " << this->data << std::endl;
}

void    Request::fill_object(std::string full_request)
{
    Request new_request(full_request);

    *this = new_request;
}
