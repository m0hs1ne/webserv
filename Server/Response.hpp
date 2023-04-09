# ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include "ParsingConfig.hpp"
#include "../includes/tools.hpp"
#include <unistd.h>

class Response
{
    public:
        int code;
        int location;
        int fileFD;
        size_t bodySize;
        std::string codeMessage;
        std::string returnFile;
        std::string response;
        std::string root;
        std::string fullPath;
        std::string body;
        std::string type;
        std::string redirect;
        std::string cgiheader;
        std::map<int, std::string> *codeMsg;

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        void formResponse(std::string method, Server &server);
        std::map<int, std::string> initHttpCode();
        ~Response();
};
#endif