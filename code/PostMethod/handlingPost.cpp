#include "../../includes/handlingPost.hpp"

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
        return ;
    }

    // Body with boundaries in middle push the part before the boundary to the opened file
    // close the file and remove it from the vector
    if (findByteByByte(connection.request.body, boundary, connection.request.buffer_size, boundary.size()) != 0 &&\
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

void fillFile(SocketConnection &connection)
{
    std::string chunk;
    std::vector<std::string> splitArr;
    std::cout << connection.request.body << std::endl;
    if (connection.request.attr.find("Transfer-Encoding") != connection.request.attr.end() &&
        connection.request.attr["Transfer-Encoding"] == " chunked")
    {
        splitArr = split(connection.request.body, '\n', 1);
        chunk = splitArr[1];
        if (chunk.find("0\n\n") != std::string::npos)
        {
            splitArr = splitString(chunk, "0\n\n", chunk.size());
            chunk = splitArr[0];
            connection.request.ended = true;
        }
        write(connection.request.openedFd, chunk.c_str(), chunk.size());
    }
    else
        write(connection.request.openedFd, connection.request.body.c_str(), connection.request.body.size());
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

    std::cout << "bfd -----> " << connection.request.bFd << std::endl;
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
    else if (connection.request.bFd != -2)
    {
        boundary(connection);
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
    else if (connection.request.contentType.find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = connection.request.contentType.substr(connection.request.contentType.find("boundary=") + 9);
        std::vector<std::string> files;
        files = splitString(connection.request.body, boundary, connection.request.buffer_size);
        for (size_t i = 0; i < files.size(); i++)
        {
            if (!files[i].empty())
                parseMultiPart(files[i], connection.request);
        }
        for (size_t i = 0; i < connection.request.parts.size(); i++)
        {
            if (!connection.request.parts[i].filename.empty())
            {
                std::string file = uploadPath + "/" + connection.request.parts[i].filename;
                connection.request.bFd = open(file.c_str(), O_RDWR | O_CREAT, 0777);
                write(connection.request.bFd, connection.request.parts[i].body.c_str(), connection.request.parts[i].body.size());
            }
            else
                connection.request.data[connection.request.parts[i].name] = connection.request.parts[i].body;
            if (i + 1 != connection.request.parts.size())
            {
                close(connection.request.bFd);
                connection.request.bFd = -2;
            }
        }
        connection.request.parts.clear();
    }
    else
    {
        connection.request.openedFd = open((uploadPath + "/" + connection.request.fileName).c_str(), O_CREAT | O_RDWR | O_APPEND, 0777);
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