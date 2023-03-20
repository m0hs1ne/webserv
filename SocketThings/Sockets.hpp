#pragma once

# include <iostream>
# include <string>
# include <vector>
#include <netinet/in.h>

class Connections
{
    public:
        std::string request;
        bool is_read;
        bool is_write;
        int Connec_fd;
};

class SocketConnection
{
    public:
        std::string client_request;
        std::string server_response;
        int socket_fd;
        int socket_port;
        struct sockaddr_in addr; // memset it // 0 '
        std::vector<Connections> connections;
};

