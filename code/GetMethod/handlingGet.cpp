// #include "../../includes/handlingGet.hpp"
#include <dirent.h>
// #include "../../includes/Autoindex.hpp"
#include "../../includes/tools.hpp"

void setEnv(Response response, Request request, Server &server)
{
    (void)response;
    if (!request.query.empty())
        setenv("QUERY_STRING", request.query.c_str(), 1);
    setenv("REQUEST_METHOD", "GET", 1);
    setenv("SERVER_PORT", itos(server.port).c_str(), 1);
    setenv("REDIRECT_STATUS", itos(200).c_str(), 1);
    setenv("PATH_INFO", request.path.c_str(), 1);
    setenv("PATH_TRANSLATED", response.fullPath.c_str(), 1);
}

void checkCGI(Request request, Response &response, Server &server)
{
    std::string cgiExts = server.locations[response.location].cgi_extension[0];
    std::string cgiPaths = server.locations[response.location].cgi_path;
    int pipefd[2];
    pid_t pid;
    char buffer[2048];
    char *const args[] = {(char *)cgiPaths.c_str(), (char *)(response.fullPath).c_str(), NULL};

    if (pipe(pipefd) == -1)
    {
        std::cerr << "Error creating pipe\n";
        return;
    }
    setEnv(response, request, server);
    pid = fork();

    if (pid == -1)
    {
        std::cerr << "Error forking process\n";
        return;
    }

    if (pid == 0)
    {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "Error redirecting standard output\n";
            return;
        }
        if (execve(cgiPaths.c_str(), args, NULL) == -1)
        {
            std::cerr << "Error executing command\n";
            return;
        }
    }
    else
    {
        close(pipefd[1]);
        std::string header;
        std::string line;
        std::string buf;
        size_t n = 1;
        while (n != 0)
        {
            n = read(pipefd[0], buffer, sizeof(buffer) - sizeof(char));
            buffer[n] = '\0';
            buf += buffer;
        }
        line = getLine(buf, 0);
        int i = 1;
        while (!line.empty())
        {
            header += line + "\r\n";
            line = getLine(buf, i);
            i++;
        }
        buf.erase(0, header.size());
        response.cgiheader = header;
        response.body = buf;
        close(pipefd[0]);
        wait(NULL);
    }
}

void handleDir(Request request, Response &response, Server &server)
{
    AutoIndex autoindex;

    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.fullPath += "/";
        response.redirect = request.path + "/";
        response.code = 301;
        return;
    }
    if (!server.locations[response.location].cgi_extension.empty() &&
        request.path.substr(request.path.find_last_of(".") + 1) == server.locations[response.location].cgi_extension[0] &&
        !access((response.fullPath + server.locations[response.location].index).c_str(), R_OK))
    {
        response.fullPath += server.locations[response.location].index;
        checkCGI(request, response, server);
    }
    if (server.locations[response.location].autoindex)
    {
        if(!access(response.fullPath.c_str(), R_OK))
        {
            response.code = 200;
            response.body = autoindex.getPage(response.fullPath.c_str(), request.path, server.host, server.port);
        }
        else
        {
            response.code = 403;
        }
    }
}

void handlingGet(Request request, Response &response, Server &server)
{
    if (isDir(response.fullPath.c_str()) == -1)
        handleDir(request, response, server);
    else if (!server.locations[response.location].cgi_extension.empty() &&
             request.path.substr(request.path.find_last_of(".") + 1) == server.locations[response.location].cgi_extension[0] &&
             !isDir(response.fullPath.c_str()))
        checkCGI(request, response, server);
}