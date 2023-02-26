#ifndef REQUEST_HPP
#define REQUEST_HPP
#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include "parseConfig.hpp"
typedef struct parsingConfig::server server;
typedef struct Request
{
    std::string method;
    std::string path;
    bool pathFound;
    std::string fullPath;
    std::map<std::string, std::string> other;
} Request;

typedef struct Response
{
    int code;
    std::string response;
} Response;

Response &handleRequest(const std::string &buffer, server &Server);
std::string getLine(std::string src, size_t n);

#endif