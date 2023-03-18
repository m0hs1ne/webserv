#include "../../includes/handlingPost.hpp"

void parseMultiPart(std::string part, std::string path, Request request, Response &res)
{
    std::string name;
    std::string filename;
    std::string contentType;
    std::string body;
    (void)request;
    (void)res;
    (void)path;

    if (part.find("Content-Disposition: form-data; name=\"") != std::string::npos)
    {
        name = part.substr(part.find("Content-Disposition: form-data; name=\"") + 38);
        name = name.substr(0, name.find("\""));
    }
    if(part.find("filename=\"") != std::string::npos)
    {
        filename = part.substr(part.find("filename=\"") + 10);
        filename = filename.substr(0, filename.find("\""));
    }

    if (part.find("Content-Type: ") != std::string::npos)
    {
        contentType = part.substr(part.find("Content-Type: ") + 14);
        contentType = contentType.substr(0, contentType.find("\n"));
    }
    // std::cout << "name: " << name << std::endl;
    // std::cout << "filename: " << filename << std::endl;
    std::cout << "contentType: " << contentType << std::endl;
}

void handlingPost(Request request, Response &response, Server &server)
{
    std::string contentType;
    std::string contentLength;
    std::string body;
    std::string path;

    if (server.locations[response.location].upload_enable)
    {
        if (!server.locations[response.location].upload_path.empty())
            path = server.locations[response.location].root + server.locations[response.location].upload_path;
        else
            path = server.locations[response.location].root + "/uploads";
    }
    else
    {
        response.code = 405;
        response.body = "upload_enable is false";
        return;
    }

    if (request.attr.find("Content-Type") != request.attr.end())
        contentType = request.attr["Content-Type"];
    if (request.attr.find("Content-Length") != request.attr.end())
        contentLength = request.attr["Content-Length"];

    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string key = urlDecodeStr(request.body.substr(0, request.body.find("=")));
        std::string value = urlDecodeStr(request.body.substr(request.body.find("=") + 1));
        request.formUrlEncoded[key] = value;
        std::cout << "---->" << key << "value " << value << std::endl;
    }

    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
        std::vector<std::string> files;
        files = splitString(request.body, "--" + boundary);

        for (size_t i = 0; i < files.size(); i++)
        {
            if(!files[i].empty())
                parseMultiPart(files[i], path, request,response);
        }
    }
}