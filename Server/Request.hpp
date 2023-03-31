#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <string>
#include <map>
#include <vector>
# include <unistd.h> 
#include "Response.hpp"
#include "../includes/tools.hpp"
#include "ParsingConfig.hpp"

typedef struct Part
{
    std::string name;
    std::string filename;
    std::string contentType;
    std::string body;
} Part;

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

        typedef struct parsingConfig::server Server;
        void fillRequest(const std::string &buffer);
        bool isRequestWellFormed(Response &response, Server &server);
        bool matchLocation(Response &response, Server &server);
        bool methodAllowed(Response &response, Server &server);
        void checkRedirection(Response &response, Server &serve);
        void checkPathFound(Response &response, Server &server);
        bool urlDecode(Response &response);

        Response handleRequest(std::string buffer, Server &server);

        Request(){};
        // Request(const Request &other){
        //     (void)other;
        // };
        // Request &operator=(const Request &other){};
        ~Request(){};
};
#endif