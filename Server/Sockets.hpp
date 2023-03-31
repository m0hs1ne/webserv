#pragma once

# include <iostream>
# include <string>
# include <vector>
#include <netinet/in.h>
# include <sys/time.h>
#include "Request.hpp"
# include <fstream>

# define ANSWER 0

class Connections
{
    public:
        Response response;
        Request request; 
        long long  content_size;
        bool drop_connection;
        size_t data_s;
        uint64_t Connec_fd;
        parsingConfig::server *server;
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

