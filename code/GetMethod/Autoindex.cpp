#include "../../includes/Autoindex.hpp"

AutoIndex::AutoIndex(void) {}

AutoIndex::AutoIndex(AutoIndex const &src)
{
    (void)src;
}

AutoIndex::~AutoIndex(void) {}

AutoIndex &AutoIndex::operator=(AutoIndex const &src)
{
    (void)src;
    return *this;
}

std::string AutoIndex::getPage(const char *path, std::string reqPath, std::string host, int port)
{
    DIR *dir = opendir(path);
    std::string page =
        "<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" +
        reqPath + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>Index of "+ reqPath +"</h1>\n\
    <p>\n";

    if (dir == NULL)
    {
        std::cerr << "Error: could not open [" << path << "]" << std::endl;
        return "";
    }
    for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir))
    {
        page += AutoIndex::getLink(std::string(dirEntry->d_name), reqPath, host, port);
    }
    page += "\
    </p>\n\
    </body>\n\
    </html>\n";
    closedir(dir);
    return page;
}

std::string AutoIndex::getLink(std::string dirEntry, std::string dirName, std::string host, int port)
{
    if (dirEntry == ".")
        return "";

    std::stringstream ss;
    ss << "\t\t<p><a href=\"http://" + host + ":" << port << dirName + dirEntry + "\">" + dirEntry + "</a></p>\n";
    std::cout << ss.str() << std::endl;
    return ss.str();
}