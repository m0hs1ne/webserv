#include "../includes/Request.hpp"

Request::Request(){}
Request::Request(const Request& other)
{
    *this = other;
}

Request& Request::operator=(const Request& other)
{
    this->method = other.method;
    this->path = other.path;
    this->pathFound = other.pathFound;
    this->attr = other.attr;
    this->size = other.size;
    this->query = other.query;
    this->body = other.body;
    this->formUrlEncoded = other.formUrlEncoded;
    return *this;
}
Request::~Request(){}
