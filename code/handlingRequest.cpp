#include "../includes/handlingRequest.hpp"

std::map<int, std::string> code;
std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!*'():;%@+$,/?#[]";

void initHttpCode()
{
    if (!code.size())
    {
        code[501] = "501 Not Implemented";
        code[500] = "500 Internal Server Error";
        code[414] = "414 Request-URI Too Long";
        code[413] = "413 Request Entity Too Large";
        code[409] = "409 Conflict";
        code[405] = "405 Method Not Allowed";
        code[404] = "404 Not Found";
        code[403] = "403 Forbidden";
        code[400] = "400 Bad Request";
        code[301] = "301 Moved Permanently";
        code[204] = "204 No Content";
        code[201] = "201 Created";
        code[200] = "200 OK";
    }
}

Response &handleRequest(std::string buffer, Server &server)
{
    Request request;
    Response *response = new Response();

    initHttpCode();
    request = fillRequest(buffer);
    if (!isRequestWellFormed(request, *response, server))
        formResponse(*response);
    else if (!matchLocation(request, *response, server))
        formResponse(*response);
    else if (!methodAllowed(request, *response, server))
        formResponse(*response);
    return *response;
}

Request &fillRequest(const std::string &buffer)
{
    Request *req = new Request;
    std::vector<std::string> splitArr;

    req->size = buffer.size();
    std::string line = getLine(buffer, 0);
    req->pathFound = false;
    splitArr = split(line, ' ', 0);
    req->method = splitArr[0];
    req->path = splitArr[1];
    line = getLine(buffer, 1);
    for (int i = 1; !line.empty(); i++)
    {
        splitArr = split(line, ':', 1);
        req->attr[splitArr[0]] = splitArr[1];
        line = getLine(buffer, i);
    }
    return *req;
}

bool isRequestWellFormed(Request request, Response &response, Server &server)
{
    if (request.attr.find("Transfer-Encoding") != request.attr.end() && request.attr["Transfer-Encoding"] != "chunked")
        response.code = 501;
    else if (request.attr.find("Transfer-Encoding") == request.attr.end() &&
             request.attr.find("Content-Length") == request.attr.end() &&
             request.method == "POST")
        response.code = 400;
    else if (request.path.size() > 2048)
        response.code = 414;
    else if (!request.path.empty())
    {
        for (size_t i = 0; i < request.path.size(); i++)
        {
            if (allowedChar.find(request.path[i]) == std::string::npos)
                response.code = 400;
        }
    }
    // we need to check if the request body size is bigger than the client_max_body_size
    else if (request.size > server.client_max_body_size)
        response.code = 413;
    return (response.code == 200);
}

void formResponse(Response &response)
{
    response.response = code[response.code] + "\r\n";
    std::cout << response.response << std::endl;
}

bool matchLocation(Request request, Response &response, Server &server)
{
    int pos;
    size_t i;

    for (i = 0; i < server.locations.size(); i++)
    {
        pos = request.path.find(server.locations[i].name);
        if (pos == 1)
        {
            response.code = 200;
            break;
        }
        else
            response.code = 404;
    }
    if (response.code == 404)
        return false;
    response.location = i;
    if (!server.locations[i].index.empty())
    {
        response.code = 200;
        response.returnFile = server.locations[i].index;
    }
    else if (!server.locations[i].return_pages.empty())
    {
        response.code = 301;
        response.returnFile = server.locations[i].return_pages;
    }
    return true;
}

bool methodAllowed(Request request, Response &response, Server &server)
{
    std::vector<std::string>::iterator it = server.locations[response.location].methods.begin();

    for (; it != server.locations[response.location].methods.end(); it++)
    {
        if (toUpper(*it) == toUpper(request.method))
            return true;
    }
    return false;
}