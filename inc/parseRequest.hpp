#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <iostream>
#include <string.h>
#include <vector>
#include <map>


typedef struct Request
{
    std::string method;
    std::string path;
    std::map<std::string, std::string> other;
} Request;

Request &parseRequest(std::string buffer);
std::string getLine(std::string src, size_t n);

#endif