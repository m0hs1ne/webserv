#include "inc/parseRequest.hpp"
#include "inc/parseConfig.hpp"
#include "inc/tools.hpp"
#include <cstdio>
#include <unistd.h>

typedef struct parsingConfig::server server;
Request &fillRequest(const std::string &buffer);
Response &createResponse(Request &req,  server &Server);

Response &handleRequest(const std::string &buffer, server &Server)
{
    Request req;
    Response *res = new Response;

    req = fillRequest(buffer);
    *res = createResponse(req, Server);
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

void addHttpCode(Request &req, Response &resp)
{
    std::map<int, std::string> codeMap;
    int code;
    codeMap = initHttpCode();
    if (req.pathFound)
        code = 200;
    else
        code = 404;
    resp.code = code;
    resp.response += codeMap[code];
}

void findPath(Request &req, server &server)
{
    req.fullPath = server.root + req.path;
    if (!access(req.fullPath.c_str(), F_OK))
        req.pathFound = true;
}

void findErrorPage(Request &req, Response &resp, server &server)
{
    if (resp.code != 200 && !server.error_pages[resp.code].empty())
    {
        req.pathFound = true;
        req.fullPath = server.error_pages[resp.code];
    }
}

void addFileContent(Request &req, Response &res)
{
    std::string fileContent;

    if (req.pathFound)
    {
        fileContent = readFile(req.fullPath);
        res.response += "\nContent-length: " + itos(fileContent.size());
        res.response += "\n\r\n" + fileContent + "\n";
    }
    else
        res.response += "\nContent-length: 0\n";
    std::cout << res.response << std::endl;
}

Response &createResponse(Request &req,  server &Server)
{
    Response *resp = new Response;

    findPath(req, Server);
    addHttpCode(req, *resp);
    findErrorPage(req, *resp, Server);
    addFileContent(req, *resp);
    return *resp;
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
    return *req;
}