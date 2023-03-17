#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>

//#include <unistd.h>
//#include <signal.h>
#include <sys/select.h>
#include "includes/handlingRequest.hpp"
# include "SocketThings/Sockets.hpp" //my header
#define MAX 5000

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return 0;
    }
    try
    {
        parsingConfig a(av[1]);
        std::vector<parsingConfig::server> servers = a.getServers();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Server(std::vector<parsingConfig::server> servers)
{
    std::vector<SocketConnection> Sockets;
    
    for (size_t i = 0; i < servers.size(); i++)
    {
        SocketConnection Socket;
        if (!( Socket.socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        // if (setsockopt(Socket.socket_fd, SOL_SOCKET, SO_REUSEPORT, &valread, sizeof(valread)))
        // {
        //     perror("setsockopt");
        //     exit(EXIT_FAILURE);
        // }
        Socket.addr.sin_family = AF_INET;
        Socket.addr.sin_addr.s_addr = INADDR_ANY;
        Socket.addr.sin_port = htons(servers[i].port);
        Socket.socket_port = servers[i].port;
        //memset(address.sin_zero, '\0', sizeof address.sin_zero);
        if (bind(Socket.socket_fd, (struct sockaddr *)&Socket.addr, sizeof(Socket.addr)))
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(Socket.socket_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        std::cout << "Listening on port " << servers[i].port << std::endl;
        Sockets.push_back(Socket);
    }
}