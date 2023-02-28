#ifndef HREQUEST_HPP
#define HREQUEST_HPP
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "tools.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "handlingConfig.hpp"

typedef struct parsingConfig::server Server;

std::map<int, std::string> code;
Response &handleRequest(const std::string &buffer, Server &server);
Request &fillRequest(const std::string &buffer);
bool isRequestWellFormed(Request request, Response &response, Server &server);
void formResponse(Response &response);
bool matchLocation(Request request, Response &response, Server &server);
#endif