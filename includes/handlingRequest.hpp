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
Request &fillRequest(const std::string &buffer);
bool isRequestWellFormed(Request request, Response &response);
void formResponse(Response &response);
#endif