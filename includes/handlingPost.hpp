#ifndef POST_HPP
#define POST_HPP
#include "handlingRequest.hpp"
#include "../includes/tools.hpp"
#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include <sys/stat.h>





void handlingPost(Request request, Response &response, Server &server);

#endif