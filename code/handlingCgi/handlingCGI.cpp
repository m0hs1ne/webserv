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

char **setEnv(Response response, Request &request, Server &server, int input)
{
    std::vector<std::string> stdEnv;

    (void)server;
    if (!request.query.empty() && request.method == "GET")
        stdEnv.push_back("QUERY_STRING=" + request.query);
    else if (request.method == "POST")
    {
        struct stat st;
        if (fstat(input, &st) == -1) {
            std::cerr << "fstat() failed\n";
            return NULL;
        }

        off_t size = st.st_size;
        stdEnv.push_back("CONTENT_LENGTH=" + itos(size));
        if (request.attr.find("Content-Type") != request.attr.end())
            stdEnv.push_back("CONTENT_TYPE=" + request.attr["Content-Type"].erase(request.attr["Content-Type"].size() - 1, request.attr["Content-Type"].size()).erase(0,1));
    }
    if (request.attr.find("Cookie") != request.attr.end())
        stdEnv.push_back("HTTP_COOKIE=" + request.attr["Cookie"].erase(0,1));

    stdEnv.push_back("REQUEST_METHOD=" + request.method);
    stdEnv.push_back("SCRIPT_FILENAME=" + response.fullPath);
    stdEnv.push_back("PATH_TRANSLATED=" + request.path);
    stdEnv.push_back("REDIRECT_STATUS=301");
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
    int input = 0;
    pid_t pid;
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
        input = open(request.fileName.c_str(), O_RDONLY);

    envp = setEnv(response, request, server, input);

    response.cgi_pid = fork();
    pid = response.cgi_pid;
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
            if (dup2(input, STDIN_FILENO) == -1)
            {
                std::cerr << "Error redirecting standard input\n";
                return;
            }
            close(input);
        }
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "Error redirecting standard output\n";
            return;
        }
        close(pipefd[1]);
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
        close(input);
        unlink(request.fileName.c_str());
        response.cgi_pid = pid;
        response.cgi_output = pipefd[0];
        dup2(saveStdin, STDIN_FILENO);
	    dup2(saveStdout, STDOUT_FILENO);
    }
}
