#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace http
{
    class TcpServer
    {
    private:
        struct sockaddr_in address;
        int sock;
        int connection;
        int listening;
        unsigned int backlog;

    public:
        TcpServer(int _domain, int _service, int _protocol, unsigned long _address, int _port, unsigned int _backlog);
        ~TcpServer();
        int get_sock();
        int get_connection();
        struct sockaddr_in get_address();
        int get_listening();
        void startListen();
    };
}
#endif