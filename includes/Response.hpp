#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include <string>

class Response
{
    public:
        int code;
        std::string codeMessage;
        std::string returnFile;
        std::string response;

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        ~Response();
};
#endif