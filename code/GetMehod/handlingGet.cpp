#include "../../includes/handlingGet.hpp"
#include <dirent.h>

void checkCGI(Request request, Response &response, Server &server)
{}

void autoindex()
{}

void handleDir(Request request, Response &response, Server &server)
{
    if (request.fullPath[request.fullPath.size() - 1] != '/')
    {
        request.fullPath += "/";
        response.code = 301;
    }
    if (!access((request.fullPath + "index.html").c_str(), R_OK) ||\
        !access((request.fullPath + "index.php").c_str(), R_OK))
    {
        checkCGI(request, response, server);
    }
    else
        autoindex();
}

void handlingGet(Request request, Response &response, Server &server)
{
    if (isDir(request.fullPath.c_str()) == 1)
    {
        // is Directory
        handleDir(request, response, server);
    }
    else if (!isDir(request.fullPath.c_str()))
    {
        // is a file
        checkCGI(request, response, server);
    }
    else
    {
        // doesn't exist
    }
}

