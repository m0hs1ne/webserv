#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <string>
#include <map>
#include <vector>

class Request
{
public:
    std::string method;
    std::string path;
    bool pathFound;
    std::map<std::string, std::string> attr;
    size_t size;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);
    ~Request();
};
#endif