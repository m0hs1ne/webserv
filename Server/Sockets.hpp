#pragma once

# include <iostream>
# include <string>
# include <vector>
#include <netinet/in.h>
# include <sys/time.h>

# define ANSWER 0

class Connections
{
    public:
        std::string request;
        bool is_read;
        bool is_write;
        uint64_t Connec_fd;
};

class SocketConnection
{
    public:
        std::string client_request;
        std::string server_response;
        uint64_t socket_fd;
        int socket_port;
        struct sockaddr_in addr; // memset it // 0 '
        std::vector<Connections> connections;
};

