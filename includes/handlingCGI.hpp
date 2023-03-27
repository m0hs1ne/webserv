#ifndef CGI_HPP
#define CGI_HPP
#include "handlingRequest.hpp"

char **setEnv(Response response, Request request, Server &server);
void checkCGI(Request request, Response &response, Server &server);
#endif