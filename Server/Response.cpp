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

void Response::formResponse(std::string method, Server &server)
{
    if (method == "GET")
        formGetResponse(*this, server);
}

Response::~Response(){}