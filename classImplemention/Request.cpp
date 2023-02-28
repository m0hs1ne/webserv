#ifndef REQUEST_HPP
#define REQUEST_HPP
#include "../includes/Request.hpp"

Request::Request(){}
Request::Request(const Request& other)
{
    *this = other;
}

Request& Request::operator=(const Request& other)
{
    this->method = other.methos;
    this->path = other.path;
    this->pathFound = other.pathFound;
    this->attr = other.attr;
    this->size = other.size;
}
Request::~Resquest(){}
#endif