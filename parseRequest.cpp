#include "inc/parseRequest.hpp"
#include "inc/tools.hpp"
#include <cstdio>

Request &parseRequest(std::string buffer)
{
    Request *req = new Request;
    std::vector<std::string> splitArr;

    std::string line = getLine(buffer, 0);
    splitArr = split(line, ' ');
    req->method = splitArr[0];
    req->path = splitArr[1];
    line = getLine(buffer, 1);
    for (int i = 1; !line.empty(); i++)
    {
        splitArr = split(line, ':');
        req->other[splitArr[0]] = splitArr[1];
        line = getLine(buffer, i);
    }
    std::cout << req->other["Cookie"] << std::endl;
    return *req;
}