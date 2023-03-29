# ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include "ParsingConfig.hpp"
#include "../includes/tools.hpp"
#include <unistd.h>
#include "Response.hpp"

class Request;
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
        std::string redirect;
        std::string cgiheader;

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        void formResponse(std::string method, Server &server);
        ~Response();
};
#endif