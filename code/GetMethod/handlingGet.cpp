#include "../../includes/handlingGet.hpp"
#include <dirent.h>
#include "../../includes/Autoindex.hpp"
#include "../../includes/tools.hpp"
#include "../../includes/handlingCGI.hpp"

#include <vector>
#include <cstring>

void handleDir(Request request, Response &response, Server &server)
{
    AutoIndex autoindex;

    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.fullPath += "/";
        response.redirect = request.path + "/";
        response.code = 301;
        return;
    }
    if (!server.locations[response.location].cgi_extension.empty() &&
        request.path.substr(request.path.find_last_of(".") + 1) == server.locations[response.location].cgi_extension[0] &&
        !access((response.fullPath + server.locations[response.location].index).c_str(), R_OK))
    {
        response.fullPath += server.locations[response.location].index;
        checkCGI(request, response, server);
    }
    if (server.locations[response.location].autoindex)
    {
        if(!access(response.fullPath.c_str(), R_OK))
        {
            response.code = 200;
            response.body = autoindex.getPage(response.fullPath.c_str(), request.path, server.host, server.port);
        }
        else
        {
            response.code = 403;
        }
    }
}

void handlingGet(Request request, Response &response, Server &server)
{
    if (isDir(response.fullPath.c_str()) == -1)
        handleDir(request, response, server);
    else if (!server.locations[response.location].cgi_extension.empty() &&
             request.path.substr(request.path.find_last_of(".") + 1) == server.locations[response.location].cgi_extension[0] &&
             !isDir(response.fullPath.c_str()))
        checkCGI(request, response, server);
}