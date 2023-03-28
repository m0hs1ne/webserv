#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

//#include "handlingRequest.hpp"
#include <dirent.h>

class AutoIndex
{
public:
    AutoIndex(void);
    AutoIndex(AutoIndex const &src);
    ~AutoIndex(void);

    AutoIndex &operator=(AutoIndex const &src);

    std::string getPage(const char *path, std::string reqPath, std::string host, int port);

    std::string getLink(std::string dirEntry, std::string dirName, std::string host, int port);
};

#endif