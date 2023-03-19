#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>

//#include <unistd.h>
//#include <signal.h>
#include <sys/select.h>
#include <sys/poll.h>
#include "includes/handlingRequest.hpp"
# include "SocketThings/Sockets.hpp" //my header

void TheServer(std::vector<parsingConfig::server> servers);
void SetUpSockets(std::vector<SocketConnection> &Sockets, std::vector<parsingConfig::server> servers, struct pollfd **fds);
void NewConnectios_Handler(struct pollfd *fds, std::vector<parsingConfig::server> servers, std::vector<SocketConnection> &Sockets);

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
        TheServer(servers);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void TheServer(std::vector<parsingConfig::server> servers)
{
    struct pollfd *fds;
    fds = new pollfd[servers.size()];

    std::vector<SocketConnection> Sockets;
    SetUpSockets(Sockets, servers, &fds);
    while(true)
    {
        NewConnectios_Handller(fds, servers, Sockets);
    }

}

void SetUpSockets(std::vector<SocketConnection> &Sockets, std::vector<parsingConfig::server> servers, struct pollfd **fds)
{
    int valread;
   
    for (size_t i = 0; i <  servers.size(); i++)
    {
        SocketConnection Socket;
        if (!( Socket.socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(Socket.socket_fd, SOL_SOCKET, SO_REUSEPORT, &valread, sizeof(valread)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        Socket.addr.sin_family = AF_INET;
        Socket.addr.sin_addr.s_addr = INADDR_ANY;
        Socket.addr.sin_port = htons(servers[i].port);
        Socket.socket_port = servers[i].port;
        memset(Socket.addr.sin_zero, '\0', sizeof(Socket.addr.sin_zero));
        if (bind(Socket.socket_fd, (struct sockaddr *)&Socket.addr, sizeof(Socket.addr)) < 0)
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
        
        (*fds)[i].fd = Socket.socket_fd;
        (*fds)[i].events = POLLIN;
    }
}

void NewConnectios_Handler(struct pollfd *fds, std::vector<parsingConfig::server> servers, std::vector<SocketConnection> &Sockets)
{
    int poll_return = 0;
    int new_socket = 0;
    ///char buffer[MAX];
    poll_return = poll(fds, servers.size(), 500);
    std::cout << poll_return << std::endl;
    if (poll_return < 0)
    {
        perror("failed");
        exit(1);
    }
    if (poll_return == 0)
        std::cout << "timed out\n" << std::endl;
    for(size_t i = 0; i < servers.size(); i++)
    {
        if(fds[i].revents == POLLIN)
        {   
            socklen_t len = sizeof(Sockets[i].addr);
            
            new_socket = accept(Sockets[i].socket_fd, (sockaddr *)&Sockets[i].addr, &len);
            Sockets[i].connections.push_back(new_socket);
            // if(recv(new_socket, buffer, sizeof(buffer), 0) == 0)
            // {
            //     std::cout << "Client waiting for response ...." << std::endl;
            // }
            // write(1, buffer, MAX);
            // //close(new_socket);
            // //exit(EXIT_FAILURE);
        }
    }
}

void Established_connections_handler(std::vector<SocketConnection> &Sockets)
{
    char buffer[MAX];
    
    for(size_t i = 0; i < Sockets.size(); i++)
    {
        for(size_t j = 0; j < Sockets[i].connections.size(); j++)
        {
            if(recv(Sockets[i].connections[j], buffer, sizeof(buffer), 0))
            {
                std::cout << "Client waiting for response ...." << std::endl;
            }
        }
    }
}