#include "inc/parseRequest.hpp"
#include "inc/parseConfig.hpp"
#include "inc/tools.hpp"
#include <cstdio>
#include <unistd.h>

typedef struct parsingConfig::server server;
Request &fillRequest(const std::string &buffer);
std::string &createResponse(Request &req,  server &Server);

Response &handleRequest(const std::string &buffer, server &Server)
{
    Request req;
    Response *res = new Response;

    req = fillRequest(buffer);
    res->response = createResponse(req, Server);
    return *res;
}

std::map<int, std::string>& initHttpCode()
{
    std::map<int, std::string> *code = new std::map<int, std::string>;
    (*code)[501] = "501 Not Implemented";
    (*code)[500] = "500 Internal Server Error";
    (*code)[414] = "414 Request-URI Too Long";
    (*code)[413] = "413 Request Entity Too Large";
    (*code)[409] = "409 Conflict";
    (*code)[405] = "405 Method Not Allowed";
    (*code)[404] = "404 Not Found";
    (*code)[403] = "403 Forbidden";
    (*code)[400] = "400 Bad Request";
    (*code)[301] = "301 Moved Permanently";
    (*code)[204] = "204 No Content";
    (*code)[201] = "201 Created";
    (*code)[200] = "200 OK";
    return (*code);
}

void addHttpCode(Request &req, std::string &Response)
{
    std::map<int, std::string> code;
    code = initHttpCode();
    if (req.pathFound)
        Response += code[200];
    else
        Response += code[404];
}

void findPath(Request &req,  server &Server)
{
    std::string fullPath;

    fullPath = Server.root + req.path;
    if (!access(fullPath.c_str(), F_OK))
        req.pathFound = true;
}

std::string &createResponse(Request &req,  server &Server)
{
    std::string *Response = new std::string;

    findPath(req, Server);
    addHttpCode(req, *Response);
    return *Response;
}

Request &fillRequest(const std::string &buffer)
{
    Request *req = new Request;
    std::vector<std::string> splitArr;

    std::string line = getLine(buffer, 0);
    req->pathFound = false;
    splitArr = split(line, ' ', 0);
    req->method = splitArr[0];
    req->path = splitArr[1];
    line = getLine(buffer, 1);
    for (int i = 1; !line.empty(); i++)
    {
        splitArr = split(line, ':', 1);
        req->other[splitArr[0]] = splitArr[1];
        line = getLine(buffer, i);
    }
    std::cout << req->other["Cookie"] << std::endl;
    return *req;
}