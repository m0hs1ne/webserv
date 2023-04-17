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
    this->fileFD = other.fileFD;
    this->fileName = other.fileName;
    return *this;
}

void formPostResponse(Request &request, Response &response, Server &server)
{
    std::map<int, std::string>code = response.initHttpCode();
    size_t fileSize = 0;
    std::ifstream oss(request.fileName, std::ios::binary | std::ios::ate);
    if (oss.is_open())
    {
        fileSize = oss.tellg();
        oss.close();
    }
    std::cout << "fileSize: " << fileSize << std::endl;
    std::cout << "client_max_body_size: " << server.locations[response.location].client_max_body_size << std::endl;
    std::cout << "location: " << response.location << std::endl;
    if (fileSize > server.locations[response.location].client_max_body_size)
    {
        response.code = 413;
        std::cout << "fileName: " << request.fileName << std::endl;
        unlink(request.fileName.c_str());
    }
    if (server.error_pages.find(response.code) != server.error_pages.end())
        response.returnFile = server.error_pages[response.code];
    if (response.body.empty() && response.returnFile.empty())
        response.body = code[response.code];
    else if (response.body.empty() && !response.returnFile.empty())
        response.body = readFile(response.returnFile);
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Content-Length: " + itos(response.body.size()) + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Type: text/html\r\n\r\n";
}

void formDeleteResponse(Response &response, Server &server)
{
    std::map<int, std::string>code = response.initHttpCode();
    size_t fileSize = 0;

    if (server.error_pages.find(response.code) != server.error_pages.end())
    {
        response.returnFile = server.error_pages[response.code];
        std::ifstream oss(response.returnFile, std::ios::binary | std::ios::ate);
        if (oss.is_open())
        {
            fileSize = oss.tellg();
            oss.close();
        }
    }
    else
    {
        response.body = code[response.code];
        fileSize = response.body.size();
    }
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Type: text/html\r\n";
    response.response += "Content-Length: " + itos(fileSize) +"\r\n";
    response.response += "\r\n";
}

void formNotAllowedResponse(Response &response, Server &server)
{
    std::map<int, std::string>code = response.initHttpCode();
    size_t fileSize = 0;
    response.code = 501;

    if (server.error_pages.find(response.code) != server.error_pages.end())
    {
        response.returnFile = server.error_pages[response.code];
        std::ifstream oss(response.returnFile, std::ios::binary | std::ios::ate);
        if (oss.is_open())
        {
            fileSize = oss.tellg();
            oss.close();
        }
    }
    else
    {
        response.body = code[response.code];
        fileSize = response.body.size();
    }
    response.response = "HTTP/1.1 ";
    response.response += code[response.code] + "\r\n";
    response.response += "Server: " + server.names[0] + "\r\n";
    response.response += "Content-Type: text/html\r\n";
    response.response += "Content-Length: " + itos(fileSize) +"\r\n";
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
        (response.returnFile.empty() || response.mNotAllow || access(response.returnFile.c_str(), R_OK)))
    {
        response.body = code[response.code];
        fileSize = response.body.size();
    }
    else if (response.body.empty() && !response.returnFile.empty() && isDir(response.returnFile.c_str()) != -1)
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
    if (response.redirect.empty() && response.cgiheader.empty())
        response.response += "Content-Type: " + type + "\r\n";
    else if (!response.redirect.empty())
    {
        response.response += "Connection: close\r\n";
        response.response += "Location: " + response.redirect + "\r\n";
    }
    if (!response.cgiheader.empty())
        response.response += response.cgiheader;
    response.response += "\r\n";
}

void Response::formResponse(Request &request, Server &server)
{
    if (request.method == "GET")
        formGetResponse(*this, server);
    else if (request.method == "POST")
        formPostResponse(request,*this, server);
    else if (request.method == "DELETE")
        formDeleteResponse(*this, server);
    else if (this->mNotAllow)
        formNotAllowedResponse(*this, server);
}

Response::~Response(){}