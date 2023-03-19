#include <sys/socket.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "includes/handlingRequest.hpp"
#define MAX 4445851

void startServers(std::vector<parsingConfig::server> servers)
{
    int server_fd[1024], new_socket = 0, max_sd, sd, activity, valread, rd = 0, s = 0, valsent = 0;
    struct sockaddr_in address;
    Response res;
    int addrlen = sizeof(address);

    fd_set readfds;
    fd_set writefds;

    for (int i = 0; i < 30; i++)
        server_fd[i] = 0;

    signal(SIGPIPE, SIG_IGN);

    for (size_t i = 0; i < servers.size(); i++)
    {
        if ((server_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        fcntl(server_fd[i], F_SETFL, O_NONBLOCK);
        if (setsockopt(server_fd[i], SOL_SOCKET, SO_REUSEPORT, &valread, sizeof(valread)))
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
    FD_ZERO(&writefds);
    while (true)
    {
        FD_ZERO(&readfds);
        max_sd = server_fd[servers.size() - 1];
        for (size_t i = 0; i < servers.size(); i++)
        {
            sd = server_fd[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }
        for (int i = 0; i < 1024; i++)
        {
            if (FD_ISSET(i, &writefds))
            {
                if (i > max_sd)
                    max_sd = i;
            }
        }
        if ((activity = select(max_sd + 1, &readfds, &writefds, NULL,NULL)) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        if (activity == 0)
        {
            continue;
        }
        std::cout << "Activity " << activity << std::endl;
        for (size_t i = 0; i < servers.size(); i++)
        {
            sd = server_fd[i];
            if (FD_ISSET(sd, &readfds) || FD_ISSET(new_socket, &writefds))
            {
                if (sd == server_fd[i] && !FD_ISSET(new_socket, &writefds))
                {
                    if ((new_socket = accept(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    fcntl(new_socket, F_SETFL, O_NONBLOCK);
                    FD_SET(new_socket, &writefds);
                }
                if (FD_ISSET(new_socket, &writefds))
                {
                    char buffer[MAX] = {0};
                    if (rd == 0)
                    {
                        valread = read(new_socket, buffer, MAX - 1);
                        std::cout << "valread: " << valread << std::endl;
                    }
                    if (valread <= 0)
                    {
                        perror("read");
                        close(new_socket);
                        FD_CLR(new_socket, &writefds);
                        rd = 0;
                        continue;
                    }
                    else
                    {
                        if (rd == 0)
                        {
                            res = handleRequest(buffer, servers[i]);
                            rd = 1;
                        }
                        std::string header = res.response;
                        std::string body = res.body;
                        if (!header.empty() && s == 0)
                        {
                            send(new_socket, header.c_str(), header.size(), 0);
                            header.clear();
                            res.response.clear();
                            s = 1;
                            // std::cout << "header sent" << std::endl;
                        }
                        else if (body.size() <= 5000 && !body.empty())
                        {
                            std::string resp = dToh(body.size()) + "\r\n" + body + "\r\n";
                            valsent += send(new_socket, resp.c_str(), resp.size(), 0);
                            body.clear();
                            res.body.clear();
                        }
                        else if (!body.empty())
                        {
                            std::string resp = dToh(5000) + "\r\n" + body.substr(0, 5000) + "\r\n";
                            valsent += send(new_socket, resp.c_str(), resp.size(), 0);
                            body.erase(0, 5000);
                            res.body.erase(0, 5000);
                        }
                        else
                        {
                            send(new_socket,
                                            "0\r\n\r\n",6 , 0);
                            valsent = 0;
                            close(new_socket);
                            FD_CLR(new_socket, &writefds);
                            rd = 0;
                            s = 0;
                        }
                        // close(new_socket);
                        // FD_CLR(new_socket, &readfds);
                    }
                }
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
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
