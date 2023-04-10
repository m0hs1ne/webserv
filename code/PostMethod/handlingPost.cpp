#include "../../includes/handlingPost.hpp"
#include <algorithm>

int skipUntilBody(std::string &str)
{
    int i = 0;
    while (str[i] && (str[i] == '\n' || str[i] == '\r'))
        i++;
    return i--;
}

void parseMultiPart(std::string part, Request &request)
{
    Part Part;
    if (part.find("Content-Disposition: form-data; name=\"") != std::string::npos)
    {
        Part.name = part.substr(part.find("Content-Disposition: form-data; name=\"") + 38);
        Part.name = Part.name.substr(0, Part.name.find("\""));
        part.erase(0, part.find("Content-Disposition: form-data; name=\"") + 38);
    }
    if (part.find("filename=\"") != std::string::npos)
    {
        Part.filename = part.substr(part.find("filename=\"") + 10);
        Part.filename = Part.filename.substr(0, Part.filename.find("\""));
        part.erase(0, part.find("filename=\"") + 10);
    }

    if (part.find("Content-Type: ") != std::string::npos)
    {
        Part.contentType = part.substr(part.find("Content-Type: ") + 14);
        Part.contentType = Part.contentType.substr(0, Part.contentType.find("\n"));
        part.erase(0, part.find("Content-Type: ") + 14);
        part.erase(0, part.find(Part.contentType) + Part.contentType.size());
        part.erase(0, skipUntilBody(part));
    }
    Part.body = part;
    request.parts.push_back(Part);
}

void boundary(SocketConnection &connection)
{
    std::string boundary = "--" + connection.request.contentType.substr(connection.request.contentType.find("boundary=") + 9);
    std::vector<std::string> files;

    // if last line can be a part of boundary concatinate it with the first line of the next body and if it is a boundary parse it
    std::string lastLine = getLastLine(connection.request.body);
    if (!connection.request.lastLine.empty())
    {
        connection.request.body = connection.request.lastLine + connection.request.body;
        connection.request.lastLine.clear();
    }
    if (boundary.find(lastLine) == 0)
    {
        connection.request.lastLine = lastLine;
        connection.request.body.erase(connection.request.body.find(lastLine), lastLine.size());
    }

    // Split the body to diffrent parts with the boundary
    files = splitString(connection.request.body, boundary, connection.request.buffer_size);

    // Body with no boundaries push it directly to the opened file
    if (files.size() == 0)
    {
        write(connection.request.bFd, connection.request.body.c_str(), connection.request.body.size());
        return;
    }

    // Body with boundaries in middle push the part before the boundary to the opened file
    // close the file and remove it from the vector
    if (findByteByByte(connection.request.body, boundary, connection.request.buffer_size, boundary.size()) != 0 &&
        findByteByByte(connection.request.body, boundary, connection.request.buffer_size, boundary.size()) != std::string::npos)
    {
        write(connection.request.bFd, files[0].c_str(), files[0].size());
        close(connection.request.bFd);
        connection.request.bFd = -2;
        files.erase(files.begin());
    }

    // if there is more files parse them
    for (size_t i = 0; i < files.size(); i++)
    {
        if (!files[i].empty())
            parseMultiPart(files[i], connection.request);
    }

    // Open the file and write the body to it
    for (size_t i = 0; i < connection.request.parts.size(); i++)
    {
        if (!connection.request.parts[i].filename.empty())
        {
            if (connection.request.bFd == -2)
            {
                std::string file = connection.request.uploadPath + "/" + connection.request.parts[i].filename;
                connection.request.bFd = open(file.c_str(), O_RDWR | O_CREAT, 0777);
            }
            write(connection.request.bFd, connection.request.parts[i].body.c_str(), connection.request.parts[i].body.size());
        }
        else
            connection.request.data[connection.request.parts[i].name] = connection.request.parts[i].body;
        if ((i + 1) != connection.request.parts.size())
        {
            close(connection.request.bFd);
            connection.request.bFd = -2;
        }
    }

    // if the body ends with the boundary close the opened file
    if (findByteByByte(connection.request.body, boundary + "--", connection.request.buffer_size, (boundary + "--").size()) != std::string::npos)
    {
        close(connection.request.bFd);
        connection.request.bFd = -2;
        connection.ended = true;
    }
    connection.request.parts.clear();
}

int hexToDec(const std::string &hex)
{
    int dec;
    std::stringstream ss(hex);
    ss >> std::hex >> dec;
    return dec;
}

int hexToDec(char hex)
{
    int dec;
    std::stringstream ss(hex);
    ss >> std::hex >> dec;
    if (dec == 0 && hex != '0')
        return -1;
    return dec;
}

void fillFile(SocketConnection &connection)
{
    std::string chunk;
    std::string *line;
    std::vector<std::string> splitArr;
    std::string body = "";
    size_t size = 0;
    std::string &chunkSize_chunked = connection.request.chunkSize_chunked;

    if (connection.request.attr.find("Transfer-Encoding") != connection.request.attr.end() &&
        connection.request.attr["Transfer-Encoding"] == " chunked\r")
    {
        if (connection.request.bodySize)
        {
            size = connection.request.bodySize;
            line = getLine(connection.request.body, 0, size);
            connection.request.chunkSize = hexToDec(*line);
            write(1, "\nbody : ", 8);
            // write(1, connection.request.body.c_str(), size);
            connection.request.body.erase(0, line->size() + 1);
            std::cout << "linesize -> "<< line->size() << std::endl;
            std::cout << "chunkedSi -> "<< connection.request.chunkSize  << std::endl;
            size -= line->size() + 1;
            write(1, "\nbody : ", 8);
            // write(1, connection.request.body.c_str(), size);
            delete line;
            // exit÷(0);
        }
        else
            size = connection.request.buffer_size;
        if (connection.request.body[size - 5] == '0')
        {
            std::string tmp = connection.request.body.substr(size - 5, 5);
            if (tmp == "0\r\n\r\n")
            {
                connection.request.body.erase(size - 5, 5);
                connection.ended = true;
                size -= 5;
            }
        }
        
        // 2 - chunkSize_chunked is set
        if (!chunkSize_chunked.empty())
        {
            //getfirst line
            line = getLine(connection.request.body, 0, size);
            //add it to the chunkSize_chunked
            chunkSize_chunked += *line;
            //and convert it to dec
            connection.request.chunkSize = hexToDec(chunkSize_chunked);
            //if the chunkSize is converted successfully
            if (connection.request.chunkSize == 0)
            {
                //do something here
            }
            delete line;
        }
        // 1 - if chunksize less than the size of the body with 10 bytes
        if (connection.request.chunkSize >= size - 10 && connection.request.chunkSize < size)
        {
            //loop on the 10 bytes to check for \r
            for (size_t i = size - 10; i < size; i++)
            {
                //if Found \r
                if (connection.request.body[i] == '\r')
                {
                    //check for newline
                    if (i + 1 < size && connection.request.body[i + 1] == '\n')
                    {
                        //increment to bypass the newline
                        i++;
                        size_t j;
                        //loop on the rest of bytes to check for the second \r
                        for (j = i; j < size && connection.request.body[j] != '\r'; j++);
                        //check for second newline
                        if (j + 1 < size && connection.request.body[j + 1] == '\n')
                        {
                            //increment to bypass the second newline
                            j++;
                        }
                        //erase the bytes from the body and return them to chunkSize_chunked
                        chunkSize_chunked = connection.request.body.erase(i, j);
                        size -= j;
                        break;
                    }
                }
            }
        }
        if (connection.request.chunkSize <= size)
        {

            size_t old_size = connection.request.chunkSize;
            // std::cout << "\n-------------------------------------=> size " << size << std::endl;
            write(1, "\n----------------------------------\n", 37);
            write(1, connection.request.body.c_str(), size);
            write(1, "\n----------------------------------\n", 37);
            line = getLine(connection.request.body, 0, size, connection.request.chunkSize + 2);
            std::cout << "\n--------------line : " << *line << std::endl;
            connection.request.body.erase(connection.request.chunkSize, line->size() + 3);
            size -= line->size() + 3;
            write(1, connection.request.body.c_str(), size);
            write(1, "\n----------------------------------\n", 37);

            std::cout <<"=========> Ch " << *line << std::endl;
            connection.request.chunkSize = hexToDec(*line);
            std::cout <<"=========> " << connection.request.chunkSize << std::endl;
            connection.request.chunkSize -= size - old_size;
            // std::cout << "\n-------------------------------------=> size " << size << std::endl;
            delete line;
        }
        else
            connection.request.chunkSize -= size;
        write(connection.request.openedFd, connection.request.body.c_str(), size);
        std::cout << "\nSize-> " << size << std::endl;
        std::cout << "\nconnection.request.chunkSize: " << connection.request.chunkSize << std::endl;
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
        {
            close(connection.request.openedFd);
            connection.ended = true;
        }
    }
    connection.request.bodySize = 0;
}

void handlingPost(SocketConnection &connection)
{
    std::string contentLength;
    std::string body;
    std::string uploadPath;

    if (connection.server->locations[connection.response.location].upload_enable && !connection.server->locations[connection.response.location].cgi_path.empty())
    {
        connection.response.code = 500;
        std::cerr << "Error: CGI and Upload conflict." << std::endl;
        return;
    }

    if (connection.request.ended)
    {
        close(connection.request.openedFd);
        return;
    }
    else if (connection.request.openedFd != -2)
    {
        fillFile(connection);
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
    else if (!connection.server->locations[connection.response.location].cgi_path.empty())
    {
        checkCGI(connection.request, connection.response, *(connection.server));
        return;
    }
    else
    {
        if (!connection.server->locations[connection.response.location].upload_enable)
            connection.response.code = 405;
        else
            connection.response.code = 403;
        return;
    }

    if (connection.request.attr.find("Content-Type") != connection.request.attr.end())
    {
        connection.request.contentType = connection.request.attr["Content-Type"];
    }
    if (connection.request.attr.find("Content-Length") != connection.request.attr.end())
        contentLength = connection.request.attr["Content-Length"];

    if (connection.request.contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string *str = new std::string("");
        str->append(connection.request.body.substr(0, findByteByByte(connection.request.body, "=", connection.request.buffer_size, sizeof("="))));
        std::string key = urlDecodeStr(*str);
        str->clear();
        str->append(connection.request.body.substr(connection.request.body.find("=") + 1));
        std::string value = urlDecodeStr(*str);
        connection.request.formUrlEncoded[key] = value;
    }
    else
    {
        std::string type = connection.server->typeToExt[connection.request.contentType.erase(0, 1).erase(connection.request.contentType.size() - 1, 1)];
        connection.request.openedFd = open((uploadPath + "/" + connection.request.fileName + type).c_str(), O_CREAT | O_RDWR | O_APPEND, 0777);
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
}