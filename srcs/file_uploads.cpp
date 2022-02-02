#include "Server.hpp"
#include "webserv.hpp"

std::pair<bool, std::string>    forbidden_page()
{
    std::string page_message = "403 Forbidden";
    return (std::make_pair(false, generate_error_page(page_message, page_message)));
}

std::pair<bool, std::string>    request_entity_too_large()
{
    std::string page_message = "413 Request Entity Too Large";
    return (std::make_pair(false, generate_error_page(page_message, page_message)));
}

long    get_max_body_size(Location & location)
{
    std::string         str_body_size = location.location_map["client_max_body_size"];
    std::stringstream   ss(str_body_size);
    long                max_body_size = 0;
    std::string         units;

    ss >> max_body_size;
    ss >> units;
    transform(units.begin(), units.end(), units.begin(), tolower);
    if (str_body_size.empty())
        return (0);
    if (units == "ko")
        return (max_body_size * 1000);
    if (units == "mo")
        return (max_body_size * 1000000);
    return (max_body_size);
}

bool    directory_exists(std::string directory_path)
{
    struct stat info;

    if (stat(directory_path.c_str(), &info) != 0)
        return (false);
    else if (info.st_mode & S_IFDIR)
        return (true);
    else
        return (false);
}

std::string gen_random_string(const int len)
{
    static const char   alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    std::string rand_str;
    
    rand_str.reserve(len);
    for (int i = 0; i < len; ++i) {
        rand_str += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return (rand_str);
}

std::pair<bool, std::string>    upload_file(Request & request, std::string upload_path, std::string path)
{
    request.data.erase(0, request.data.find("\n\r") + 3);
    request.data.erase(request.data.rfind(request.headers["boundary"]) - 3);
    // if directory doesn't exist, create it
    if (!directory_exists(upload_path)) {
        if (mkdir(upload_path.c_str(), 0744) == -1)
            return (internal_server_error());
    }
    // make sure path ends with /
    if (upload_path[upload_path.size() - 1] != '/')
        upload_path += '/';

    std::string full_path = upload_path + request.headers["Filename"];
    std::string file_extension = get_file_extension(request.headers["Filename"]);

    struct stat buffer;
    while (stat(full_path.c_str(), &buffer) == 0)   // file already exists, we need a new one
        full_path = upload_path + gen_random_string(10) + '.' + get_file_extension(request.headers["Filename"]);
    // create file with full path
    std::ofstream   outfile(full_path);

    // write data to new file
    int file = open(full_path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
	write(file, request.data.c_str(), request.data.size());
    return (std::make_pair(true, get_response(path, request.uri, request.protocol, 200, 0)));
}

std::pair<bool, std::string>    check_upload_file(Request & request, Location & location, std::string path)
{
    std::string         upload_path = location.location_map["upload_path"];
    long                content_len = request.data.size();
    long                max_body_size;

    max_body_size = get_max_body_size(location);

    if (upload_path.empty()) {
        return (forbidden_page());
    }
    else if (content_len > max_body_size) {
        return (request_entity_too_large());
    }
    else {
        return (upload_file(request, upload_path, path));
    }
}
