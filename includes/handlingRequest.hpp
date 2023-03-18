#ifndef HREQUEST_HPP
#define HREQUEST_HPP
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <unistd.h>
#include "tools.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "handlingConfig.hpp"

typedef struct parsingConfig::server Server;

Response handleRequest(std::string buffer, Server &server);
Request fillRequest(const std::string &buffer);
bool isRequestWellFormed(Request request, Response &response, Server &server);
void formResponse(Request request, Response &response, Server &server);
bool matchLocation(Request request, Response &response, Server &server);
bool methodAllowed(Request request, Response &response, Server &server);
void checkRedirection(Response &response, Server &server,Request request);
void checkPathFound(Request request, Response &response, Server &server);
std::string setContentType(std::string path);
bool urlDecode(Request *req, Response &response);
#endif