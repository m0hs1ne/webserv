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

    (void)server;
    if (!request.query.empty() && request.method == "GET")
        stdEnv.push_back("QUERY_STRING=" + request.query);
    else if (request.method == "POST")
    {
        std::stringstream out;
		out << request.body.length();
        stdEnv.push_back("CONTENT_LENGTH=" + out.str());
        if (request.attr.find("Content-Type") != request.attr.end())
            stdEnv.push_back("CONTENT_TYPE=" + request.attr["Content-Type"].erase(request.attr["Content-Type"].size() - 1, request.attr["Content-Type"].size()).erase(0, 1));
        // std::cout << "CONTENT_tyPE=" << request.attr["Content-Type"] << std::endl;
    }
    if (request.attr.find("Cookie") != request.attr.end())
        stdEnv.push_back("HTTP_COOKIE=" + request.attr["Cookie"].erase(0,1));

    stdEnv.push_back("REQUEST_METHOD=" + request.method);
    stdEnv.push_back("SCRIPT_FILENAME=" + response.fullPath);
    stdEnv.push_back("PATH_TRANSLATED=" + request.path);
    stdEnv.push_back("REDIRECT_STATUS=200");
    stdEnv.push_back("PATH_INFO=" + request.path);
    return convertToCharArray(stdEnv);
}

size_t find_cgi_path(std::vector<std::string>& cgiExtension, std::string cgiExts)
{
    size_t i = 0;
    while (i < cgiExtension.size())
    {
        if (cgiExtension[i].find(cgiExts) != std::string::npos)
            return i;
        i++;
    }
    return i;
}

void checkCGI(Request &request, Response &response, Server &server)
{
    std::string cgiExts = request.extension;
    std::string cgiPaths;
    size_t cgiPathIndex;

    cgiPathIndex = find_cgi_path(server.locations[response.location].cgi_extension, cgiExts);
    if (cgiPathIndex == server.locations[response.location].cgi_path.size())
        cgiPathIndex = 0;
    cgiPaths = server.locations[response.location].cgi_path[cgiPathIndex];
    int pipefd[2];
    int postBodyfd[2];
    pid_t pid;
    char buffer[2048];
    char *const args[] = {(char *)cgiPaths.c_str(), (char *)(response.fullPath).c_str(), NULL};
    char **envp;

    int			saveStdin;
	int			saveStdout;
    saveStdin = dup(STDIN_FILENO);
	saveStdout = dup(STDOUT_FILENO);

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
    for (size_t i = 0; envp[i] != NULL; i++)
    {
        std::cout << envp[i] << std::endl;
    }
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
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "Error redirecting standard output\n";
            return;
        }
        // char buffer[1000];
        // read(STDIN_FILENO, buffer, 1000);
        // std::cerr << buffer << std::endl;
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
    
        std::string header = "";
        std::string *line = NULL;
        std::string buf;
        size_t n = 1;
        std::map<int, std::string> codeMsg = response.initHttpCode();

        while (n != 0)
        {
            n = read(pipefd[0], buffer, sizeof(buffer) - sizeof(char));
            response.bodySize += n;
            buffer[n] = '\0';
            buf.append(buffer, n);
        }
        if(response.bodySize == 0)
        {
            buf.append("Content-Type: text/html\r\n");
            buf.append("\r\n");
            buf.append("<h1>500 Internal Error</h1>\r\n");
            buf.append("Error executing CGI\n");
            response.bodySize = buf.size();
        }
        size_t headerSize = 0;
        size_t lineSize = 0;
        line = getLine(buf, 0, response.bodySize, &lineSize, "size");
        for (int i = 1; !line->empty() && *line != "\r"; i++)
        {
            header.append(*line + "\n");
            delete line;
            headerSize += lineSize + 1;
            line = getLine(buf, i, response.bodySize, &lineSize, "size");
        }
        buf.erase(0, header.size() + 2);
        response.bodySize -= header.size() + 2;
        response.cgiheader = header;
        if (!buf.empty())
            response.body.append(buf.c_str(), response.bodySize);
        else
            response.body.append(" ");
        dup2(saveStdin, STDIN_FILENO);
	    dup2(saveStdout, STDOUT_FILENO);
        close(pipefd[0]);
        wait(NULL);
        }
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
