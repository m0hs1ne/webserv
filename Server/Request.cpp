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

void fillPostBody(std::string &buffer, Request &req)
{
    std::string buf_tmp = "";
    size_t size = req.buffer_size - (req.headerSize);

    buf_tmp.append(buffer.c_str(), req.buffer_size);
    buf_tmp.erase(0, req.headerSize);
    req.body.clear();
    req.body.append(buf_tmp.c_str(), size);
    req.bodySize = size;
}

void ParseFirstLine(std::string &buffer, Request &req)
{
    std::vector<std::string> splitArr;
    std::string *line;
    size_t l_size = 0;

    line = getLine(buffer, 0, req.buffer_size, &l_size, "size");
    req.headerSize += l_size + 1;
    splitArr = split(*line, ' ', 0);
    delete line;
    req.method = splitArr[0];
    if (!splitArr[1].empty())
        req.path = splitArr[1];
    splitArr = split(req.path, '?', 1);
    if (splitArr.size() > 1)
    {
        req.path = splitArr[0];
        req.query = splitArr[1];
    }
    if (req.path.find_last_of('.') != std::string::npos)
        req.extension = req.path.substr(req.path.find_last_of('.') + 1);
}

void ParseHeaderAttr(std::string &buffer, Request &req)
{
    std::vector<std::string> splitArr;
    std::string *line;
    size_t l_size = 0;

    line = getLine(buffer, 1, req.buffer_size, &l_size, "size");
    req.headerSize += l_size + 1;
    for (int i = 2; !line->empty(); i++)
    {
        if(line->size() == 1 && (*line)[0] == '\r')
            break;
        splitArr = split(*line, ':', 1);
        delete line;
        req.attr[splitArr[0]] = splitArr[1];
        line = getLine(buffer, i, req.buffer_size, &l_size, "size");
        req.headerSize += l_size + 1;
    }
    delete line;
}

void Request::fillRequest(const std::string &buffer)
{
    std::vector<std::string> splitArr;
    std::string buf_tmp = "";

    this->headerSize = 0;
    buf_tmp.append(buffer.c_str(), this->buffer_size);

    ParseFirstLine(buf_tmp, *this);
    ParseHeaderAttr(buf_tmp, *this);
    if (this->method == "POST")
        fillPostBody(buf_tmp, *this);
}

bool Request::isRequestWellFormed(Response &response)
{
    if (this->attr.find("Transfer-Encoding") != this->attr.end() && this->attr["Transfer-Encoding"] != " chunked\r")
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
            response.fullPath[response.fullPath.size() - 1] == '/' && this->method == "GET" && !access(response.fullPath.c_str(), R_OK))
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
    response.mNotAllow = true;
    return false;
}

Response Request::handleRequest(char *buffer, Server &server)
{
    if (this->openedFd == -2 && this->bFd == -2)
    {
        Response *response = new Response();
        Response resp;
        std::string *buff = new std::string();
        
        buff->append(buffer, this->buffer_size);
        fillRequest(*buff);
        this->ok = false;
        if (isRequestWellFormed(*response) &&
            urlDecode(*response) &&
            matchLocation(*response, server) &&
            methodAllowed(*response, server))
        {

            this->ok = true;
        }
        resp = *response;
        delete response;
        delete buff;
        return resp;
    }
    else
    {
        this->method = "POST";
        if(!this->body.empty())
        {
            write(1, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n1------------------------------------------------------------------------------------------\n", 113);
            write(1, this->body.c_str(), this->chunkSize_chunked);
            write(1, "\n2------------------------------------------------------------------------------------------\n\n\n", 96);
            this->body.append(buffer, this->buffer_size);
            write(1, "\n\n\n1------------------------------------------------------------------------------------------\n", 96);
            write(1, this->body.c_str(), this->chunkSize_chunked + this->buffer_size);
            write(1, "\n2------------------------------------------------------------------------------------------\n\n\n", 96);
            // exit(0);
        }
        else
            this->body.append(buffer, this->buffer_size);
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