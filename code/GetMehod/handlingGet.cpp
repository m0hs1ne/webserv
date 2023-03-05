#include "../../includes/handlingGet.hpp"
#include <dirent.h>
#include "../../includes/Autoindex.hpp"

// void checkCGI(Request request, Response &response, Server &server)
// {}

void handleDir(Request request, Response &response, Server &server)
{
    (void)(request);
    AutoIndex autoindex;
    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.fullPath += "/";
        response.redirect = request.path + "/";
        response.code = 301;
    }
    // if (!access((request.fullPath + "index.html").c_str(), R_OK) ||\
    //     !access((request.fullPath + "index.php").c_str(), R_OK))
    // {
    //     checkCGI(request, response, server);
    // }
    else if(server.locations[response.location].autoindex)
    {
        response.code = 200;
        response.body = autoindex.getPage(response.fullPath.c_str(),request.path, server.host, server.port);
    }
}

void handlingGet(Request request, Response &response, Server &server)
{
    if (isDir(response.fullPath.c_str()) == -1)
    {
        handleDir(request, response, server);
    }
    else if (!isDir(response.fullPath.c_str()))
    {
        // is a file
        // checkCGI(request, response, server);
    }
    else
    {
        // doesn't exist
    }
}

