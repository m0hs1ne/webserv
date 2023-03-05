#include "../../includes/handlingGet.hpp"
#include <dirent.h>
#include "../../includes/Autoindex.hpp"

void checkCGI(Request request, Response &response, Server &server)
{
    std::string cgiExts = server.locations[response.location].cgi_extension[0];
    std::string cgiPaths = server.locations[response.location].cgi_path;
    int pipefd[2];
    pid_t pid;
    char buffer[2048];
    char *const args[] = { (char *)cgiPaths.c_str(), (char *)response.fullPath.c_str(), NULL };

    // Create the pipe
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe\n";
        return ;
    }

    // Fork a child process
    pid = fork();

    if (pid == -1) {
        std::cerr << "Error forking process\n";
        return ;
    }

    if (pid == 0) {
        // Child process - write to pipe
        close(pipefd[0]); // Close the read end of the pipe

        // Redirect standard output to the write end of the pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            std::cerr << "Error redirecting standard output\n";
            return ;
        }

        // Execute the command
        if (execve(cgiPaths.c_str(), args, NULL) == -1) {
            std::cerr << "Error executing command\n";
            return ;
        }
    } else {
        // Parent process - read from pipe
        close(pipefd[1]); // Close the write end of the pipe

        // Read data from the pipe
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer));

        response.body = buffer;

        // Wait for the child process to complete
        wait(NULL);
    }
}

void handleDir(Request request, Response &response, Server &server)
{
    (void)(request);
    AutoIndex autoindex;
    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.fullPath += "/";
        response.redirect = request.path + "/";
        response.code = 301;
    }
    if (!access((response.fullPath + "index.html").c_str(), R_OK) ||\
        !access((response.fullPath + "index.php").c_str(), R_OK))
    {
        checkCGI(request, response, server);
    }
    else if(server.locations[response.location].autoindex)
    {
        response.code = 200;
        response.body = autoindex.getPage(response.fullPath.c_str(),request.path, server.host, server.port);
    }
}

void handlingGet(Request request, Response &response, Server &server)
{
    if (isDir(response.fullPath.c_str()) == -1)
    {
        handleDir(request, response, server);
    }
    else if (!isDir(response.fullPath.c_str()))
    {
        // is a file
        // checkCGI(request, response, server);
    }
    else
    {
        // doesn't exist
    }
}

