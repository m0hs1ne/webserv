#include "Response.hpp"


Response::Response()
{
    this->bodySize = 0;
    this->code = 200;
    this->returnFile = "";
    this->mNotAllow = false;
    this->fileFD = -2;
}

Response::Response(const Response& other)
{
    *this = other;
}

std::map<int, std::string> Response::initHttpCode()
{
    std::map<int, std::string> code;

    code[501] = "501 Not Implemented";
    code[500] = "500 Internal Server Error";
    code[414] = "414 Request-URI Too Long";
    code[413] = "413 Request Entity Too Large";
    code[411] = "411 Length Required";
    code[409] = "409 Conflict";
    code[405] = "405 Method Not Allowed";
    code[404] = "404 Not Found";
    code[403] = "403 Forbidden";
    code[400] = "400 Bad Request";
    code[301] = "301 Moved Permanently";
    code[204] = "204 No Content";
    code[201] = "201 Created";
    code[200] = "200 OK";
    return code;
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
    this->body = "";
    this->body.append(other.body.c_str(), other.bodySize);
    this->bodySize = other.bodySize;
    this->type = other.type;
    this->cgiheader = other.cgiheader;
    this->mNotAllow = other.mNotAllow;
    return *this;
}

void formPostResponse(Response &response, Server &server)
{
    std::map<int, std::string>code = response.initHttpCode();

    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    if (response.body.empty() && response.returnFile.empty())
        response.body = code[response.code];
    else if (response.body.empty() && !response.returnFile.empty())
        response.body = readFile(response.returnFile);
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Type: text/html\r\n\r\n";
}

void formDeleteResponse(Response &response, Server &server)
{
    std::map<int, std::string>code = response.initHttpCode();

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


void formGetResponse(Response &response, Server &server)
{
    size_t fileSize = 0;
    std::map<int, std::string>code = response.initHttpCode();

    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    std::string ext = "." + response.returnFile.substr(response.returnFile.find_last_of(".") + 1);
    std::string type = server.extToType[ext];
    if (type.empty() || type == "application/x-httpd-php")
        type = "text/html";
    if (response.body.empty() &&
        response.redirect.empty() &&
        (response.returnFile.empty() || response.mNotAllow || response.code == 404 || access(response.returnFile.c_str(), R_OK)))
    {
        response.body = code[response.code];
        fileSize = response.body.size();
    }
    else if (response.body.empty() && !response.returnFile.empty() && isDir(response.returnFile.c_str()) != -1 && response.code == 200)
    {
        response.fileFD = open(response.returnFile.c_str(), O_RDWR);
        if(response.fileFD < 0)
            response.code = 500;
        else
        {
            std::ifstream oss(response.returnFile, std::ios::binary | std::ios::ate);
            fileSize = oss.tellg();
            oss.close();
        }
    }
    else if (!response.body.empty())
    {
        response.returnFile.clear();
        fileSize = response.body.size();
    }


    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Content-Length: " + itos(fileSize) + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    if (response.redirect.empty())
        response.response += "Content-Type: " + type + "\r\n";
    else
    {
        response.response += "Connection: close\r\n";
        response.response += "Location: " + response.redirect + "\r\n";
    }
    response.response += "\r\n";
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