#include "../../includes/handlingPost.hpp"

void parseMultiPart(std::string part, Request &request)
{
    Part Part;
    if (part.find("Content-Disposition: form-data; name=\"") != std::string::npos)
    {
        Part.name = part.substr(part.find("Content-Disposition: form-data; name=\"") + 38);
        Part.name = Part.name.substr(0, Part.name.find("\""));
    }
    if (part.find("filename=\"") != std::string::npos)
    {
        Part.filename = part.substr(part.find("filename=\"") + 10);
        Part.filename = Part.filename.substr(0, Part.filename.find("\""));
    }

    if (part.find("Content-Type: ") != std::string::npos)
    {
        Part.contentType = part.substr(part.find("Content-Type: ") + 14);
        Part.contentType = Part.contentType.substr(0, Part.contentType.find("\n"));
    }

    if (part.find("\n\n") != std::string::npos)
        Part.body = part.substr(part.find("\n\n") + 2);
    request.parts.push_back(Part);
}

void handlingPost(Request request, Response &response, Server &server)
{
    std::string contentType;
    std::string contentLength;
    std::string body;
    std::string uploadPath;

    if (server.locations[response.location].upload_enable)
    {
        if (!server.locations[response.location].upload_path.empty())
        {
            uploadPath = server.locations[response.location].root + server.locations[response.location].upload_path;
            if (uploadPath[uploadPath.size() - 1] == '/')
                uploadPath.erase(uploadPath.size() - 1, 1);
        }
        else
        {
            uploadPath = server.locations[response.location].root + "/uploads";
        }
        if (access(uploadPath.c_str(), F_OK) == -1)
        {
            int status = mkdir(uploadPath.c_str(), 0777);
            if (status == -1)
            {
                response.code = 500;
                return;
            }
        }
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
    }

    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
        std::vector<std::string> files;
        files = splitString(request.body, "--" + boundary);

        for (size_t i = 0; i < files.size(); i++)
        {
            if (!files[i].empty())
                parseMultiPart(files[i], request);
        }
        std::cout << "request.parts.size(): " << request.parts.size() << std::endl;
        for (size_t i = 0; i < request.parts.size(); i++)
        {
            if (!request.parts[i].filename.empty())
            {
                std::string file = uploadPath + "/" + request.parts[i].filename;
                std::ofstream ofs(file);
                ofs << request.parts[i].body;
                ofs.close();
            }
            else
            {
                request.data[request.parts[i].name] = request.parts[i].body;
                std::cout << "request.parts[i].name: " << request.parts[i].name << std::endl;
                std::cout << "request.parts[i].body: " << request.parts[i].body << std::endl;
            }
        }
    }
}