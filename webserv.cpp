#include <sys/socket.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include "includes/handlingRequest.hpp"
#define MAX 3000

void startServers(std::vector<parsingConfig::server> servers)
{
    int server_fd[30], new_socket, max_sd, sd, activity,valread;
    struct sockaddr_in address;
    Response res;
    int addrlen = sizeof(address);

    fd_set readfds;

    for (int i = 0; i < 30; i++)
        server_fd[i] = 0;

    for (size_t i = 0; i < servers.size(); i++)
    {
        if ((server_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(server_fd[i], SOL_SOCKET, SO_REUSEADDR, &valread, sizeof(valread)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(servers[i].port);
        memset(address.sin_zero, '\0', sizeof address.sin_zero);
        if (bind(server_fd[i], (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd[i], 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        std::cout << "Listening on port " << servers[i].port << std::endl;
    }
    while (true)
    {
        FD_ZERO(&readfds);
        for (size_t i = 0; i < servers.size(); i++)
            FD_SET(server_fd[i], &readfds);
        max_sd = server_fd[0];
        for (size_t i = 0; i < servers.size(); i++)
        {
            sd = server_fd[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            std::cout << "select error" << std::endl;
        }
        for (size_t i = 0; i < servers.size(); i++)
        {
            sd = server_fd[i];
            if (FD_ISSET(sd, &readfds))
            {
                if ((new_socket = accept(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                std::cout << "New connection" << std::endl;
                std::cout << "Accepted socket: " << new_socket << std::endl;
                char buffer[MAX] = {0};
                valread = read(new_socket, buffer, MAX -1);
                // std::cout << buffer << std::endl;
                if(valread > 0)
                    res = handleRequest(buffer, servers[i]);
                std::string hello = "HTTP/1.1 "+ res.response;
                // std::string hello = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world!";
                send(new_socket, hello.c_str(), hello.length(), 0);
                std::cout << "Hello message sent" << std::endl;
            }
        }
    }
}

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
        while (true)
            startServers(servers);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}