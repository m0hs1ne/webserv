#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include <string>

class Response
{
    public:
        int code;
        int location;
        std::string codeMessage;
        std::string returnFile;
        std::string response;
        std::string root;
        std::string fullPath;
        std::string body;
        std::string type;

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        ~Response();
};
#endif