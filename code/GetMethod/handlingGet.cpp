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
            response.bodySize = response.body.size();
        }
        else
        {
            response.code = 403;
        }
    }
}

bool isLocation(SocketConnection &connection)
{
    // std::cout << "----========== " << connection.server->locations[connection.response.location].name << std::endl;
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
    else if (!connection.server->locations[connection.response.location].cgi_extension.empty() &&
             connection.request.path.substr(connection.request.path.find_last_of(".") + 1) == connection.server->locations[connection.response.location].cgi_extension[0] &&
             !isDir(connection.response.fullPath.c_str()))
    checkCGI(connection.request, connection.response, *(connection.server));
}