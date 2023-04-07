// #include "../includes/handlingRequest.hpp"
// #include "../includes/handlingGet.hpp"
// #include "../includes/handlingDelete.hpp"
// #include "../includes/handlingPost.hpp"
#include "Request.hpp"

std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-=._~!*'():;%@+$,/?#[]' '";

bool Request::urlDecode(Response &response)
{
    std::ostringstream decoded;

    for (std::string::const_iterator it = this->path.begin(); it != this->path.end(); ++it)
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
    this->path = decoded.str();
    return true;
}

void fillPostBody(std::string buffer, Request &req, int line)
{
    std::vector<std::string> splitArr;
    std::string bodyLine;

    buffer += "\nEOF";
    bodyLine = getLine(buffer, line, req.buffer_size);
    line++;
    for (int i = line; bodyLine != "EOF"; i++)
    {
        req.body += bodyLine + "\n";
        bodyLine = getLine(buffer, i, req.buffer_size);
    }
}

void Request::fillRequest(const std::string &buffer)
{
    std::vector<std::string> splitArr;
    int i;

    this->size = buffer.size();
    std::cout << "buffer size: " << this->size << std::endl;
    std::string line = getLine(buffer, 0, this->buffer_size);
    this->pathFound = false;
    splitArr = split(line, ' ', 0);
    this->method = splitArr[0];
    this->path = splitArr[1];
    splitArr = split(this->path, '?', 1);
    if (splitArr.size() > 1)
    {
        this->path = splitArr[0];
        this->query = splitArr[1];
    }
    line = getLine(buffer, 1, this->buffer_size);
    for (i = 1; !line.empty(); i++)
    {
        splitArr = split(line, ':', 1);
        this->attr[splitArr[0]] = splitArr[1];
        line = getLine(buffer, i, this->buffer_size);
    }
    if (this->method == "POST")
        fillPostBody(buffer, *this, i);
}

bool Request::isRequestWellFormed(Response &response, Server &server)
{
    if (this->attr.find("Transfer-Encoding") != this->attr.end() && this->attr["Transfer-Encoding"] != " chunked")
        response.code = 501;
    else if (this->attr.find("Transfer-Encoding") == this->attr.end() &&
             this->attr.find("Content-Length") == this->attr.end() &&
             this->method == "POST")
        response.code = 400;
    else if (this->path.size() > 2048)
        response.code = 414;
    else if (!this->path.empty())
    {
        for (size_t i = 0; i < this->path.size(); i++)
        {
            if (allowedChar.find(this->path[i]) == std::string::npos)
                response.code = 400;
        }
    }
    else if (this->attr.find("Content-Length") != this->attr.end() &&
             sToi(this->attr["Content-Length"]) > server.client_max_body_size)
        response.code = 413;
    return (response.code == 200);
}

bool Request::matchLocation(Response &response, Server &server)
{
    size_t i = 0;
    std::vector<std::string> splitPath;
    std::vector<std::string> splitLocation;
    int location = -1;

    splitPath = split(this->path, '/', 0);
    while (!splitPath.empty() && i < server.locations.size())
    {
        splitLocation = split(server.locations[i].name, '/', 0);
        for (size_t j = 0; j < splitLocation.size(); j++)
        {
            if (splitLocation[j] == splitPath[j])
            {
                response.code = 200;
                location = (int)i;
            }
            else
                break;
        }
        splitLocation.clear();
        i++;
    }
    if (location == -1)
    {
        for (size_t i = 0; i < server.locations.size(); i++)
        {
            if (server.locations[i].name == "/")
            {
                response.code = 200;
                location = (int)i;
                break;
            }
        }
        if (location == -1)
        {
            response.code = 404;
            return false;
        }
    }
    else if(this->path != server.locations[location].name)
        this->path = this->path.substr(server.locations[location].name.size());
    response.location = location;
    checkPathFound(response, server);
    checkRedirection(response, server);

    return true;
}

void Request::checkPathFound(Response &response, Server &server)
{
    if (!server.locations[response.location].root.empty())
        response.root = server.locations[response.location].root;
    else
        response.root = server.root;
    response.fullPath = response.root + this->path;
    if (response.fullPath[response.fullPath.size() - 1] != '/' && !access(response.fullPath.c_str(), R_OK))
        response.returnFile = response.fullPath;
    else
        response.code = 404;
    
}

void Request::checkRedirection(Response &response, Server &server)
{
    if (!server.locations[response.location].return_pages.empty())
    {
        response.code = 301;
        response.returnFile = server.locations[response.location].return_pages;
    }
    else if (!server.locations[response.location].index.empty() &&\
             response.returnFile.empty() &&\
            response.fullPath[response.fullPath.size() - 1] == '/')
    {
        response.code = 200;
        response.returnFile = response.root + server.locations[response.location].index;

    }
}

bool Request::methodAllowed(Response &response, Server &server)
{
    std::vector<std::string>::iterator it = server.locations[response.location].methods.begin();

    for (; it != server.locations[response.location].methods.end(); it++)
    {
        if (toUpper(*it) == toUpper(this->method))
            return true;
    }
    response.code = 405;
    return false;
}

Response Request::handleRequest(std::string buffer, Server &server)
{
    if (this->openedFd == -2 && this->bFd == -2)
    {
        Response *response = new Response();
        Response resp;
        fillRequest(buffer);
        this->ok = false;
        if (isRequestWellFormed(*response, server) &&
            urlDecode(*response) &&
            matchLocation(*response, server) &&
            methodAllowed(*response, server))
        {
            this->ok = true;
        }
        std::cout << this->method << std::endl;
        resp = *response;
        delete response;
        return resp;
    }
    else
    {
        std::cout << "openedFd : " << this->openedFd  << std::endl;
        this->method = "POST";
        Response response;
        response.code = 200;
        return response;
    }
}

// void formGetResponse(Response &response, Server &server)
// {
//     if (server.error_pages.find(response.code) != server.error_pages.end())
//         response.returnFile = server.error_pages[response.code];
//     std::string type = setContentType(response.returnFile);
//     if (response.body.empty() &&
//         response.redirect.empty() &&
//         (response.returnFile.empty() || access(response.returnFile.c_str(), R_OK)))
//         response.body = code[response.code];
//     else if (response.body.empty() && !response.returnFile.empty())
//     {
//         response.body = readFile(response.returnFile);
//     }
//     response.response = "HTTP/1.1 ";
//     response.response += code[response.code] + "\r\n";
//     response.response += "Server: " + server.names[0] + "\r\n";
//     response.response += "Content-Length: " + itos(response.body.size()) + "\r\n";
//     if (response.redirect.empty())
//     {
//         response.response += "Transfer-Encoding: chunked\r\n";
//         response.response += "Content-Type: " + type + "\r\n";
//         response.response += "\r\n";
//     }
//     else
//     {
//         response.response += "Connection: close\r\n";
//         response.response += "Location: " + response.redirect + "\r\n";
//         response.response += "\r\n";
//     }
// }

// void formPostResponse(Response &response, Server &server)
// {
//     if (server.error_pages.find(response.code) != server.error_pages.end())
//         response.returnFile = server.error_pages[response.code];
//     if (response.body.empty() && response.returnFile.empty())
//     {
//         response.body = code[response.code];
//     }
//     else if (response.body.empty() && !response.returnFile.empty())
//     {
//         std::cout << "return file: " << response.returnFile << std::endl;
//         response.body = readFile(response.returnFile);
//     }
//     response.response = "HTTP/1.1 ";
//     response.response += code[response.code] + "\r\n";
//     response.response += "Server: " + server.names[0] + "\r\n";
//     response.response += "Content-Length: " + itos(response.body.size()) + "\r\n";
//     response.response += "Transfer-Encoding: chunked\r\n";
//     response.response += "Content-Type: text/html\r\n";
//     response.response += "\r\n";
// }

// void Request::formResponse(Request request, Response &response, Server &server)
// {
//     if (request.method == "GET")
//         formGetResponse(response, server);
//     else if (request.method == "DELETE")
//         formDeleteResponse(response, server);
//     else if (request.method == "POST")
//         formPostResponse(response, server);
//     do if condition of POST method here
// }