#include "../../includes/handlingPost.hpp"

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
    
    

    // if (request.attr["Content-Type"] == "application/x-www-form-urlencoded")
    // {
    //     std::string key = request.body.substr(0, request.body.find("="));
    //     std::string value = request.body.substr(request.body.find("=") + 1);
    //     request.formUrlEncoded[key] = value;
    // }
    // if(contentType.find("multipart/form-data") != std::string::npos)
    // {
    //     std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
    //     std::string filename = request.body.substr(request.body.find("filename=") + 10);
    //     filename = filename.substr(0, filename.find("\r\n"));
    //     std::string file = request.body.substr(request.body.find("\r\n\r\n") + 4);
    //     file = file.substr(0, file.find("\r\n--" + boundary + "--"));
    //     std::string filepath = path + "/" + filename;
    //     std::ofstream filestream(filepath);
    //     filestream << file;
    //     filestream.close();
    //     response.code = 200;
    //     response.body = "File uploaded";
    // }
    // else
    // {
    //     response.code = 400;
    //     response.body = "Bad request";
    // }
}