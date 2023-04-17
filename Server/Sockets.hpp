#pragma once

# include <iostream>
# include <string>
# include <vector>
#include <netinet/in.h>
# include <sys/time.h>
#include "Request.hpp"
# include <fstream>

# define ANSWER 0


class SocketConnection
{
    public:
        bool ended;
        Response response;
        Request request; 
        std::string client_request;
        std::string server_response;
        parsingConfig::server *server;
        uint64_t socket_fd;
        struct sockaddr_in addr;
        int socket_port;
        long long  content_size;
        bool drop_connection;
        size_t data_s;
        //uint64_t Connec_fd; 
        int IsPortSocket;    
};
