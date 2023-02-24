#include "TcpServer.hpp"

void exitwitherror(const std::string &err)
{
    std::cerr << err << std::endl;
    exit(EXIT_FAILURE);
}

http::TcpServer::TcpServer(int _domain, int _service, int _protocol, unsigned long _address, int _port, unsigned int _backlog)
{
    address.sin_family = _domain;
    address.sin_port = _port;
    address.sin_addr.s_addr = htonl(_address);
    backlog = _backlog;
    //Establish Socket
    sock = socket(_domain, _service, _protocol);
    if (sock < 0)
        exitwitherror("Failed to socker...");
    connection = bind(sock, (struct sockaddr *)&address, sizeof(address));
    if (connection < 0)
        exitwitherror("Failed to bind...");
}

void http::TcpServer::startListen()
{
    listening = listen(connection, backlog);
    if (listening < 0)
        exitwitherror("Failed to listen...");
}

int http::TcpServer::get_sock()
{
    return sock;
}

int http::TcpServer::get_connection()
{
    return connection;
}
struct sockaddr_in http::TcpServer::get_address()
{
    return address;
}
int http::TcpServer::get_listening()
{
    return listening;
}