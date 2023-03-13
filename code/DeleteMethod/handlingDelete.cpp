
#include "../../includes/handlingDelete.hpp"

typedef struct parsingConfig::server Server;

void handleDir(Response &response)
{

    if (response.fullPath[response.fullPath.size() - 1] != '/')
    {
        response.code = 409;
        return ;
    }
    else
    {
        int result = std::remove(response.fullPath.c_str());

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

void handleFile(Response &response)
{
    int result = std::remove(response.fullPath.c_str());
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
void handlingDelete(Response &response)
{
    if (isDir(response.fullPath.c_str()) == -1)
        handleDir(response);
    else if (!isDir(response.fullPath.c_str()))
        handleFile(response);
}