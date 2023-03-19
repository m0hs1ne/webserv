#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <string>
#include <map>
#include <vector>

typedef struct Part
{
    std::string name;
    std::string filename;
    std::string contentType;
    std::string body;
}Part;

class Request
{
public:
    std::string method;
    std::string body;
    std::map<std::string, std::string> formUrlEncoded;
    std::string path;
    std::string query;
    bool pathFound;
    std::map<std::string, std::string> attr;
    size_t size;
    std::vector<Part> parts;
    std::map<std::string, std::string> data;

    Request();
    Request(const Request& other);
    Request& operator=(const Request& other);
    ~Request();
};
#endif