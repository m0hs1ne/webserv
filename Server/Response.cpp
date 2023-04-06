#include "Response.hpp"


Response::Response()
{
    this->code = 200;
    this->returnFile = "";
}

Response::Response(const Response& other)
{
    *this = other;
}

Response& Response::operator=(const Response& other)
{
    this->code = other.code;
    this->codeMessage = other.codeMessage;
    this->response = other.response;
    this->returnFile = other.returnFile;
    this->location = other.location;
    this->root = other.root;
    this->fullPath = other.fullPath;
    this->body = other.body;
    this->type = other.type;
    this->cgiheader = other.cgiheader;
    return *this;
}

void formPostResponse(Response &response, Server &server)
{
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    if (response.body.empty() && response.returnFile.empty())
        response.body = (*response.codeMsg)[response.code];
    else if (response.body.empty() && !response.returnFile.empty())
        response.body = readFile(response.returnFile);
    response.response = "HTTP/1.1 ";
    response.response += (*response.codeMsg)[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Type: text/html\r\n";
}

void formDeleteResponse(Response &response, Server &server)
{
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    else
        response.body = (*response.codeMsg)[response.code];
    response.response = "HTTP/1.1 ";
    response.response += (*response.codeMsg)[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Length: 0\r\n";
    response.response += "\r\n";
}


void formGetResponse(Response &response, Server &server)
{
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    std::string type = setContentType(response.returnFile);
    if (response.body.empty() &&
        response.redirect.empty() &&
        (response.returnFile.empty()|| response.code == 404 || access(response.returnFile.c_str(), R_OK)))
        response.body = (*response.codeMsg)[response.code];
    else if (response.body.empty() && !response.returnFile.empty() && isDir(response.returnFile.c_str()) != -1)
    {
        response.fileFD = open(response.returnFile.c_str(), O_RDWR);
        if(response.fileFD < 0)
            response.code = 500;
    }
    response.response = "HTTP/1.1 ";
    response.response += (*response.codeMsg)[response.code] + "\r\n";
    // response.response += "Content-Length: " + itos(response.body.size()) + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    if (response.redirect.empty())
    {
        response.response += "Content-Type: " + type + "\r\n";
    }
    else
    {
        response.response += "Connection: close\r\n";
        response.response += "Location: " + response.redirect + "\r\n";
    }
}

void Response::formResponse(std::string method, Server &server)
{
    if (method == "GET")
        formGetResponse(*this, server);
    else if (method == "POST")
        formPostResponse(*this, server);
    else if (method == "DELETE")
        formDeleteResponse(*this, server);
}

Response::~Response(){}