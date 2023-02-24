#include <sys/socket.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include "../inc/parseConfig.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return 0;
    }
    std::string file = av[1];
    parsingConfig p(file);
    std::vector<parsingConfig::server> servers = p.getServers();
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(servers[0].port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(fd, 10) == -1)
    {
        perror("listen");
    }
    int len = sizeof addr;
    int new_socket;
    while (true)
    {
        std::cout << "Waiting for connection" << std::endl;
        if ((new_socket = accept(fd, (struct sockaddr *)&addr, (socklen_t *)&len)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char buffer[1024] = {0};
        int vread = read(new_socket, buffer, 1024);
        std::cout << buffer << std::endl;
        if (vread < 0)
            std::cout << "No Bytes to read" << std::endl;
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><head></head><body>Hello World!</body></html>";
        write(new_socket, response.c_str(), response.length());
        std::cout << "Message sent" << std::endl;
        close(new_socket);
    }
}