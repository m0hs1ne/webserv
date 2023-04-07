#include "../../includes/handlingCGI.hpp"

char** convertToCharArray(std::vector<std::string> strings) {
    char** charArray = new char*[strings.size() + 1];

    for (size_t i = 0; i < strings.size(); i++) {
        charArray[i] = new char[strings[i].length() + 1];
        strcpy(charArray[i], strings[i].c_str());
    }
    charArray[strings.size()] = NULL;
    return charArray;
}

char **setEnv(Response response, Request &request, Server &server)
{
    std::vector<std::string> stdEnv;

    if (!request.query.empty() && request.method == "GET")
        stdEnv.push_back("QUERY_STRING=" + request.query);
    else if (request.method == "POST")
    {
        stdEnv.push_back("CONTENT_LENGTH=" + request.attr["CONTENT_LENGTH"]);
        stdEnv.push_back("CONTENT_TYPE=" + request.attr["CONTENT_TYPE"]);
    }
    stdEnv.push_back("GATEWAY_INTERFACE=CGI/1.1");
    stdEnv.push_back("REQUEST_METHOD=" + request.method);
    stdEnv.push_back("SERVER_PORT=" + itos(server.port));
    stdEnv.push_back("REDIRECT_STATUS=" + itos(200));
    stdEnv.push_back("PATH_INFO=" + request.path);
    stdEnv.push_back("PATH_TRANSLATED=" + response.fullPath);
    return convertToCharArray(stdEnv);
}

// void checkCGI(Request request, Response &response, Server &server)
// {
//     std::string cgiExts = server.locations[response.location].cgi_extension[0];
//     std::string cgiPaths = server.locations[response.location].cgi_path;
//     int pipefd[2];
//     pid_t pid;
//     char buffer[2048];
//     char *const args[] = {(char *)cgiPaths.c_str(), (char *)(response.fullPath).c_str(), NULL};
//     char **envp;

//     if (pipe(pipefd) == -1)
//     {
//         std::cerr << "Error creating pipe\n";
//         return;
//     }
//     envp = setEnv(response, request, server);
//     pid = fork();

//     if (pid == -1)
//     {
//         std::cerr << "Error forking process\n";
//         return;
//     }

//     if (pid == 0)
//     {
//         close(pipefd[0]);
//         if (dup2(pipefd[1], STDOUT_FILENO) == -1)
//         {
//             std::cerr << "Error redirecting standard output\n";
//             return;
//         }
//         if (execve(cgiPaths.c_str(), args, envp) == -1)
//         {
//             std::cerr << "Error executing command\n";
//             return;
//         }
//     }
//     else
//     {
//         freeCharArray(envp);
//         close(pipefd[1]);
//         std::string header;
//         std::string line;
//         std::string buf;
//         size_t n = 1;
//         while (n != 0)
//         {
//             n = read(pipefd[0], buffer, sizeof(buffer) - sizeof(char));
//             buffer[n] = '\0';
//             buf += buffer;
//         }
//         line = getLine(buf, 0);
//         int i = 1;
//         while (!line.empty())
//         {
//             header += line + "\r\n";
//             line = getLine(buf, i);
//             i++;
//         }
//         buf.erase(0, header.size());
//         response.cgiheader = header;
//         response.body = buf;
//         close(pipefd[0]);
//         wait(NULL);
//     }
// }

void checkCGI(Request &request, Response &response, Server &server)
{
    std::string cgiExts = server.locations[response.location].cgi_extension[0];
    std::string cgiPaths = server.locations[response.location].cgi_path;
    int pipefd[2];
    int postBodyfd[2];
    pid_t pid;
    char buffer[2048];
    char *const args[] = {(char *)cgiPaths.c_str(), (char *)(response.fullPath).c_str(), NULL};
    char **envp;


    if (pipe(pipefd) == -1)
    {
        std::cerr << "Error creating pipe\n";
        return;
    }
    if (request.method == "POST")
    {
        if (pipe(postBodyfd) == -1)
        {
            std::cerr << "Error creating pipe\n";
            return;
        }
        write(postBodyfd[1], request.body.c_str(), request.body.size());
        close(postBodyfd[1]);
    }
    envp = setEnv(response, request, server);
    pid = fork();

    if (pid == -1)
    {
        std::cerr << "Error forking process\n";
        return;
    }

    if (pid == 0)
    {
        close(pipefd[0]);
        if (request.method == "POST")
        {
            if (dup2(postBodyfd[0], STDIN_FILENO) == -1)
            {
                std::cerr << "Error redirecting standard input\n";
                return;
            }
        }
        std::cerr << "cgi done\n" << std::endl;
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "Error redirecting standard output\n";
            return;
        }
        if (execve(cgiPaths.c_str(), args, envp) == -1)
        {
            std::cerr << "Error executing command\n";
            return;
        }
    }
    else
    {
        freeCharArray(envp);
        close(pipefd[1]);
        std::string header;
        std::string line;
        std::string buf;
        size_t n = 1;
        while (n != 0)
        {
            n = read(pipefd[0], buffer, sizeof(buffer) - sizeof(char));
            buffer[n] = '\0';
            for (size_t i = 0; i < n; i++)
                buf.push_back(buffer[i]);
        }
        line = getLine(buf, 0, sizeof(buf.c_str()));
        int i = 1;
        while (!line.empty())
        {
            header += line + "\r\n";
            line = getLine(buf, i, sizeof(buf.c_str()));
            i++;
        }
        buf.erase(0, header.size());
        response.cgiheader = header;
        for (size_t i = 0; i < sizeof(buf.c_str()); i++)
            request.body.push_back(buf[i]);
        copyByteByByte(response.body, buf);
        close(pipefd[0]);
        wait(NULL);
    }
}