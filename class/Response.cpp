#include "../includes/Response.hpp"

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
    return *this;
}

Response::~Response(){}