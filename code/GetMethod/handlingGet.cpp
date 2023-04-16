#include "../../includes/handlingGet.hpp"
#include <dirent.h>
#include "../../includes/Autoindex.hpp"
#include "../../includes/handlingCGI.hpp"


typedef parsingConfig::server Server;


void handleDir(Request &request, Response &response, Server &server)
{
    AutoIndex autoindex;

    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.fullPath += "/";
        response.redirect = request.path + "/";
        response.code = 301;
        return;
    }
    if(request.path == "/")
    {
        if(server.locations[response.location].index.empty() && !server.locations[response.location].autoindex && server.locations[response.location].return_pages.empty())
        {
            response.body = "Welcome to my server!";
            response.code = 200;
        }
    }

    if(!server.locations[response.location].return_pages.empty())
    {
        response.fullPath += server.locations[response.location].return_pages;
        response.returnFile = response.fullPath;
        response.redirect = request.path + server.locations[response.location].return_pages;
        response.code = 301;
        if (access(response.fullPath.c_str(), R_OK))
        {
            response.code = 403;
        }
    }
        
    else if(server.locations[response.location].index.empty())
    {
        response.code = 403;
    }
    else
    {
        response.fullPath += server.locations[response.location].index;
        if (access(response.fullPath.c_str(), R_OK))
        {
            response.code = 403;
        }
    }

    if (response.fullPath.find_last_of('.') != std::string::npos)
        request.extension = response.fullPath.substr(response.fullPath.find_last_of('.') + 1);

    if (!server.locations[response.location].cgi_extension.empty() &&
        find(server.locations[response.location].cgi_extension.begin(),\
             server.locations[response.location].cgi_extension.end(),\
             request.extension) !=  \
             server.locations[response.location].cgi_extension.end())
    {
        checkCGI(request, response, server);
        return;
    }
    if (server.locations[response.location].autoindex)
    {
        if(!access(response.fullPath.c_str(), R_OK))
        {
            response.code = 200;
            response.body = autoindex.getPage(response.fullPath.c_str(), request.path, server.host, server.port);
            response.bodySize = response.body.size();
        }
        else
            response.code = 403;
    }
}

bool isLocation(SocketConnection &connection)
{
    if (connection.request.path != "/" && \
        connection.server->locations[connection.response.location].name == connection.request.path)
        return true;
    return false;
}

void handleLocation(SocketConnection &connection)
{
    connection.response.fullPath += "/";
    connection.response.code = 301;
    connection.response.redirect = connection.request.path + "/";
    return;
}

void handlingGet(SocketConnection &connection)
{
    connection.ended = true;
    if (isLocation(connection))
        handleLocation(connection);
    else if (isDir(connection.response.fullPath.c_str()) == -1)
        handleDir(connection.request, connection.response, *(connection.server));
    else if (!connection.server->locations[connection.response.location].cgi_extension.empty() && \
             find(connection.server->locations[connection.response.location].cgi_extension.begin(),\
                  connection.server->locations[connection.response.location].cgi_extension.end() ,\
                  connection.request.extension) !=  connection.server->locations[connection.response.location].cgi_extension.end() && \
             !isDir(connection.response.fullPath.c_str()))
    checkCGI(connection.request, connection.response, *(connection.server));
}