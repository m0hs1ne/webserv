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
    std::string fullPath;
    std::map<std::string, std::string> attr;
    int size;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);
    ~Request();
};
#endif