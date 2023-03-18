#pragma once

# include <iostream>
# include <string>
# include <vector>
#include <netinet/in.h>

class SocketConnection
{
    public:
        std::string client_request;
        std::string server_response;
        int socket_fd;
        int socket_port;
        struct sockaddr_in addr; // memset it // 0 '
};

