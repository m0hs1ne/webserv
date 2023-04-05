#ifndef POST_HPP
#define POST_HPP
#include "../Server/Sockets.hpp"
#include "../includes/handlingCGI.hpp"
#include <sys/stat.h>

void handlingPost(SocketConnection &connection);

#endif