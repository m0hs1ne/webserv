
#include "../../includes/handlingDelete.hpp"
#include <dirent.h>

typedef struct parsingConfig::server Server;
void handleFile(Response &response,std::string path)
{
    int result = unlink(path.c_str());
    if (result == 0)
        response.code = 204;
    else
    {
        if (access(path.c_str(), W_OK))
                response.code = 403;
            else
                response.code = 500;
    }
}

void handleDir(Response &response)
{
    DIR *dir = opendir(response.fullPath.c_str());

    if(!dir)
    {
        response.code = 403;
        return ;
    }

    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.code = 409;
        return ;
    }
    else
    {
        for(struct dirent *dirEntry = readdir(dir);dirEntry;dirEntry= readdir(dir))
        {
            handleFile(response,response.fullPath + dirEntry->d_name);
        }
        int result = rmdir(response.fullPath.c_str());

        if (result == 0)
            response.code = 204;
        else
        {
            if (access(response.fullPath.c_str(), W_OK))
                response.code = 403;
            else
                response.code = 500;
        }
    }
}

void handlingDelete(Response &response)
{
    if (isDir(response.fullPath.c_str()) == -1)
        handleDir(response);
    else if (!isDir(response.fullPath.c_str()))
        handleFile(response, response.fullPath);
}