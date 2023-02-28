#include "../includes/handlingRequest.hpp"

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

Response &handleRequest(const std::string &buffer, Server &server)
{
    Request request;
    Response *response = new Response();

    initHttpCode();
    request = fillRequest(buffer);
    if (!isRequestWellFormed(request, *response, server))
        formResponse(*response);
    else if (matchLocation(request, *response, server))
        formReponse(*response);
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
    if (request.attr["Transfer-Encoding"] != "chunked")
        response.code = 501;
    else if (request.attr.find("Transfer-Encoding") == request.attr.end() &&\
                request.attr.find("Content-Length") == request.attr.end() &&\
                request.method == "POST")
        response.code = 400;
    else if (request.size > 2048)
        response.code = 414;
    else if (request.size > server.client_max_body_size)
        response.code = 413;
    return (response.code == 200);
}

void formReponse(Response &response)
{}

bool matchLocation(Request request, Response &response, Server &server)
{
    int pos;
    int i;

    for (i = 0;  i < server.locations.size(); i++)
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
    if (!server.locations[i].index.empty())
    {
        response.code == 301;
        response.returnFile = server.locations[i].index;
    }
    response.location = i;
    return true;
}