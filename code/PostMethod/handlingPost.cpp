#include "../../includes/handlingPost.hpp"
#include <algorithm>

int skipUntilBody(std::string &str)
{
    int i = 0;
    while (str[i] && (str[i] == '\n' || str[i] == '\r'))
        i++;
    return i--;
}

int hexToDec(const std::string &hex)
{
    int dec;
    std::stringstream ss(hex);
    ss >> std::hex >> dec;
    return dec;
}


void fillFile(SocketConnection &connection)
{
    std::string chunk;
    std::string *line;
    std::vector<std::string> splitArr;
    std::string body = "";
    size_t size = 0;

    if (connection.request.attr.find("Transfer-Encoding") != connection.request.attr.end() &&
        connection.request.attr["Transfer-Encoding"] == " chunked\r")
    {
        if (connection.request.bodySize)
        {
            size = connection.request.bodySize;
            line = getLine(connection.request.body, 0, size);
            connection.request.chunkSize = hexToDec(*line);
            if (connection.request.chunkSize != 0)
            {
                connection.request.body.erase(0, line->size() + 1);
                size -= line->size() + 1;
            }
            std::cout << "size: " << size << std::endl;
            delete line;
        }
        else
            size = connection.request.buffer_size;
        size += connection.request.chunkSize_chunked;
        if (connection.request.body[size - 5] == '0')
        {
            std::string tmp = connection.request.body.substr(size - 5, 5);
            if (tmp == "0\r\n\r\n")
            {
                if (connection.request.chunkSize == 0)
                {
                    connection.request.body.erase(size - 5, 5);
                    size -= 5;
                }
                else
                {
                    connection.request.body.erase(size - 7, 7);
                    size -= 7;
                }
                write(connection.request.openedFd, connection.request.body.c_str(), size);
                connection.response.code = 201;
                connection.ended = true;
                return ;
            }
        }
        if (connection.request.chunkSize >= size - 50 && connection.request.chunkSize < size && connection.ended != true)
        {
            connection.request.chunkSize_chunked = size;
            return;
        }
        else if (connection.request.chunkSize < size)
        {
            size_t old_size = connection.request.chunkSize;
            line = getLine(connection.request.body, 0, size, connection.request.chunkSize + 2);
            connection.request.body.erase(connection.request.chunkSize, line->size() + 3);
            size -= line->size() + 3;
            connection.request.chunkSize = hexToDec(*line);
            connection.request.chunkSize -= size - old_size;
            delete line;
        }
        else
            connection.request.chunkSize -= size;
        write(connection.request.openedFd, connection.request.body.c_str(), size);
        connection.request.chunkSize_chunked = 0;
    }
    else
    {
        size_t len = sToi(connection.request.attr["Content-Length"]);
        if (connection.request.bodySize)
            size = connection.request.bodySize;
        else
            size = connection.request.buffer_size;
        write(connection.request.openedFd, connection.request.body.c_str(), size);
        connection.request.received += size;
        if (connection.request.received >= len)
            connection.ended = true;
    }
    if (connection.ended)
        close(connection.request.openedFd);
    connection.request.openedFd = -2;
    connection.request.bodySize = 0;
   
    connection.request.body.clear();
}

void process_for_cgi(SocketConnection &connection, std::string uploadPath)
{
    if (connection.request.openedFd == -2)
    {
        std::string type = "";
        if (!connection.request.contentType.empty())
            type = connection.server->typeToExt[connection.request.contentType.erase(0, 1).erase(connection.request.contentType.size() - 1, 1)];
        connection.request.fileName = uploadPath + "/" + connection.request.fileName + type;
        connection.response.fileName = connection.request.fileName;
        connection.request.openedFd = open((connection.request.fileName).c_str(), O_CREAT | O_RDWR | O_APPEND, 0777);
        if (connection.request.openedFd == -1)
        {
            connection.response.code = 500;
            connection.request.ended = true;
            return;
        }
    }
    std::cout << "openedFd: " << connection.request.openedFd << std::endl;
    fillFile(connection);
    if (connection.ended)
    {
        std::cout << "openFd2: " << connection.request.openedFd << std::endl;
        checkCGI(connection.request, connection.response, *(connection.server));
    }
    connection.response.code = 200;
}

void handlingPost(SocketConnection &connection)
{
    std::string contentLength;
    std::string body;
    std::string uploadPath;
    std::cout << "response.location: " << connection.response.location << std::endl;
    if (connection.request.openedFd != -2 && connection.server->locations[connection.response.location].cgi_path.empty())
    {
        fillFile(connection);
        return;
    }
    else if (connection.request.openedFd != -2 && !connection.server->locations[connection.response.location].cgi_path.empty())
    {
        process_for_cgi(connection, uploadPath);
        return;
    }

    if (connection.server->locations[connection.response.location].upload_enable)
    {
        if (!connection.server->locations[connection.response.location].upload_path.empty())
        {
            uploadPath = connection.response.root + connection.server->locations[connection.response.location].upload_path;
            if (uploadPath[uploadPath.size() - 1] == '/')
                uploadPath.erase(uploadPath.size() - 1, 1);
            connection.request.uploadPath = uploadPath;
        }
        else
        {
            uploadPath = connection.response.root + "/uploads";
            connection.request.uploadPath = uploadPath;
        }
        if (access(uploadPath.c_str(), F_OK) == -1)
        {

            int status = mkdir(uploadPath.c_str(), 0777);
            if (status == -1)
            {
                connection.response.code = 500;
                return;
            }
        }
    }
    if (!connection.server->locations[connection.response.location].cgi_path.empty())
    {
        process_for_cgi(connection, uploadPath);
        return;
    }
    else
    {
        if (!connection.server->locations[connection.response.location].upload_enable)
            connection.response.code = 405;
        else
            connection.response.code = 403;
        connection.ended = true;
        return;
    }

    if (connection.request.attr.find("Content-Type") != connection.request.attr.end())
    {
        connection.request.contentType = connection.request.attr["Content-Type"];
    }
    if (connection.request.attr.find("Content-Length") != connection.request.attr.end())
        contentLength = connection.request.attr["Content-Length"];

    if(contentLength == " 0\r")
    {
        connection.response.code = 411;
        connection.ended = true;
        return;
    }

    if (connection.request.contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string key = urlDecodeStr(connection.request.body.substr(0, connection.request.body.find("=")));
        std::string value = urlDecodeStr(connection.request.body.substr(connection.request.body.find("=") + 1));
        connection.request.formUrlEncoded[key] = value;
    }
    else
    {
        std::string type = "";
        if (!connection.request.contentType.empty())
            type = connection.server->typeToExt[connection.request.contentType.erase(0, 1).erase(connection.request.contentType.size() - 1, 1)];
        connection.request.fileName = uploadPath + "/" + connection.request.fileName + type;
        connection.response.fileName = connection.request.fileName;
        connection.request.openedFd = open((connection.request.fileName).c_str(), O_CREAT | O_RDWR | O_APPEND, 0777);
        if (connection.request.openedFd == -1)
        {
            connection.response.code = 500;
            connection.request.ended = true;
            return;
        }
        fillFile(connection);
        connection.response.code = 201;
        return;
    }
    connection.response.code = 201;
    connection.ended = true;
}