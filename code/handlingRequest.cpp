#include "../includes/handlingRequest.hpp"
#include "../includes/handlingGet.hpp"
#include "../includes/handlingDelete.hpp"

std::map<int, std::string> code;
std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-=._~!*'():;%@+$,/?#[]' '";

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

bool urlDecode(Request *req, Response &response)
{
    std::ostringstream decoded;

    for (std::string::const_iterator it = req->path.begin(); it != req->path.end(); ++it)
    {
        if (*it == '%')
        {
            std::istringstream hex_string(std::string(it + 1, it + 3));
            int hex_value = 0;
            hex_string >> std::hex >> hex_value;

            decoded << static_cast<char>(hex_value);
            it += 2;
        }
        else if (*it == '+')
        {
            decoded << ' ';
        }
        else
        {
            decoded << *it;
        }
    }
    for (size_t i = 0; i < decoded.str().size(); i++)
    {
        if (allowedChar.find(decoded.str()[i]) == std::string::npos)
        {
            response.code = 400;
            return false;
        }
    }
    req->path = decoded.str();
    std::cout << "decoded path: " << req->path << std::endl;
    return true;
}


std::string setContentType(std::string path)
{
    std::string type;

    if (path.empty())
    {
        type = "text/html";
        return type;
    }

    std::string ext = path.substr(path.find_last_of(".") + 1);
    if (ext == "html" || ext == "php")
        type = "text/html";
    else if (ext == "jpg" || ext == "jpeg")
        type = "image/jpeg";
    else if (ext == "png")
        type = "image/png";
    else if (ext == "gif")
        type = "image/gif";
    else if (ext == "css")
        type = "text/css";
    else if (ext == "js")
        type = "application/javascript";
    else if (ext == "json")
        type = "application/json";
    else if (ext == "txt")
        type = "text/plain";
    else if (ext == "xml")
        type = "text/xml";
    else if (ext == "pdf")
        type = "application/pdf";
    else if (ext == "zip")
        type = "application/zip";
    else if (ext == "gz")
        type = "application/gzip";
    else if (ext == "mp3")
        type = "audio/mpeg";
    else if (ext == "mp4")
        type = "video/mp4";
    else if (ext == "avi")
        type = "video/x-msvideo";
    else if (ext == "svg")
        type = "image/svg+xml";
    else if (ext == "ico")
        type = "image/x-icon";
    else
        type = "text/plain";
    return type;
}

Request fillRequest(const std::string &buffer) // this is your part a zakaria
{
    Request *req = new Request;
    Request request;
    std::vector<std::string> splitArr;

    req->size = buffer.size();
    std::string line = getLine(buffer, 0);
    req->pathFound = false;
    splitArr = split(line, ' ', 0);
    req->method = splitArr[0];
    req->path = splitArr[1];
    if (req->method == "GET")
    {
        splitArr = split(req->path, '?', 1);
        if (splitArr.size() > 1)
        {
            req->path = splitArr[0];
            req->query = splitArr[1];
        }
        line = getLine(buffer, 1);
        for (int i = 1; !line.empty(); i++)
        {
            splitArr = split(line, ':', 1);
            req->attr[splitArr[0]] = splitArr[1];
            line = getLine(buffer, i);
        }
    }
    else if (req->method == "POST")
    {
        // post parse
    }
    request = *req;
    delete req;
    return request;
}

bool isRequestWellFormed(Request request, Response &response, Server &server)
{
    if (request.attr.find("Transfer-Encoding") != request.attr.end() && request.attr["Transfer-Encoding"] != " chunked")
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

bool matchLocation(Request request, Response &response, Server &server)
{
    int pos;
    size_t i;
    for (i = 0; i < server.locations.size(); i++)
    {
        pos = request.path.find(server.locations[i].name);
        if (pos == 0)
        {
            response.code = 200;
            break;
        }
        else
            response.code = 404;
    }
    if (response.code == 404)
        return false;
    std::cout << request.path << "     " << server.locations[i].name << std::endl;
    response.location = i;
    checkPathFound(request, response, server);
    checkRedirection(response, server, request);
    return true;
}

void checkPathFound(Request request, Response &response, Server &server)
{
    if (!server.locations[response.location].root.empty())
        response.root = server.locations[response.location].root;
    else
        response.root = server.root;
    response.fullPath = response.root + request.path;
    if (response.fullPath[response.fullPath.size() - 1] != '/' && !access(response.fullPath.c_str(), R_OK))
        response.returnFile = response.fullPath;
    else
        response.code = 404;
}

void checkRedirection(Response &response, Server &server, Request request)
{
    if (!server.locations[response.location].return_pages.empty())
    {
        response.code = 301;
        response.returnFile = server.locations[response.location].return_pages;
    }
    else if (!server.locations[response.location].index.empty() && request.path == server.locations[response.location].name)
    {
        response.code = 200;
        response.returnFile = server.locations[response.location].index;
    }
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

void formGetResponse(Response &response, Server &server)
{
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    std::string type = setContentType(response.returnFile);
    if (response.body.empty() &&
        response.redirect.empty() &&
        (response.returnFile.empty() || access(response.returnFile.c_str(), R_OK)))
        response.body = code[response.code];
    else if (response.body.empty() && !response.returnFile.empty())
    {
        response.body = readFile(response.returnFile);
    }
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Length: " + itos(response.body.size()) + "\r\n";
    if (response.redirect.empty())
    {
        response.response += "Transfer-Encoding: chunked\r\n";
        response.response += "Content-Type: " + type + "\r\n";
        response.response += "\r\n";
    }
    else
    {
        response.response += "Connection: close\r\n";
        response.response += "Location: " + response.redirect + "\r\n";
        response.response += "\r\n";
    }
}

void formDeleteResponse(Response &response, Server &server)
{
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    else
        response.body = code[response.code];
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Length: 0\r\n";
    response.response += "\r\n";
}

void formResponse(Request request, Response &response, Server &server)
{
    if (request.method == "GET")
        formGetResponse(response, server);
    else if (request.method == "DELETE")
        formDeleteResponse(response, server);
    // do if condition of POST method here
}

Response handleRequest(std::string buffer, Server &server)
{
    Request request;
    Response *response = new Response();
    Response resp;
    initHttpCode();
    request = fillRequest(buffer);
    if (isRequestWellFormed(request, *response, server) &&
        urlDecode(&request, *response) &&
        matchLocation(request, *response, server) &&
        methodAllowed(request, *response, server))
    {
        if (request.method == "GET")
            handlingGet(request, *response, server);
        else if (request.method == "DELETE")
            handlingDelete(*response);
        // do if condition of POST;
    }
    formResponse(request, *response, server);
    resp = *response;
    delete response;
    return resp;
}