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

void handlingPost(Connections &connection)
{
    std::string contentType;
    std::string contentLength;
    std::string body;
    std::string uploadPath;

    if (connection.server->locations[connection.response.location].upload_enable)
    {
        if (!connection.server->locations[connection.response.location].upload_path.empty())
        {
            uploadPath = connection.server->locations[connection.response.location].root + connection.server->locations[connection.response.location].upload_path;
            if (uploadPath[uploadPath.size() - 1] == '/')
                uploadPath.erase(uploadPath.size() - 1, 1);
        }
        else
        {
            uploadPath = connection.server->locations[connection.response.location].root + "/uploads";
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
    else
    {
        connection.response.code = 405;
        return;
    }

    if (connection.request.attr.find("Content-Type") != connection.request.attr.end())
        contentType = connection.request.attr["Content-Type"];
    if (connection.request.attr.find("Content-Length") != connection.request.attr.end())
        contentLength = connection.request.attr["Content-Length"];

    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::string key = urlDecodeStr(connection.request.body.substr(0, connection.request.body.find("=")));
        std::string value = urlDecodeStr(connection.request.body.substr(connection.request.body.find("=") + 1));
        connection.request.formUrlEncoded[key] = value;
    }

    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
        std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
        std::vector<std::string> files;
        files = splitString(connection.request.body, "--" + boundary);

        for (size_t i = 0; i < files.size(); i++)
        {
            if (!files[i].empty())
                parseMultiPart(files[i], connection.request);
        }
        std::cout << "request.parts.size(): " << connection.request.parts.size() << std::endl;
        for (size_t i = 0; i < connection.request.parts.size(); i++)
        {
            if (!connection.request.parts[i].filename.empty())
            {
                std::string file = uploadPath + "/" + connection.request.parts[i].filename;
                std::ofstream ofs(file);
                ofs << connection.request.parts[i].body;
                ofs.close();
            }
            else
            {
                connection.request.data[connection.request.parts[i].name] = connection.request.parts[i].body;
                std::cout << "request.parts[i].name: " << connection.request.parts[i].name << std::endl;
                std::cout << "request.parts[i].body: " << connection.request.parts[i].body << std::endl;
            }
        }
    }
    connection.response.formResponse(connection.request.method, *(connection.server));
}