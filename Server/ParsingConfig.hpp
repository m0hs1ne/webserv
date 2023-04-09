#ifndef PARSE_HPP
#define PARSE_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cstdlib>

static const char *serverProp[] = {"listen", "server_name", "error_page", "root","include", nullptr};
static const char *locProp[] = {"method", "root", "autoindex", "index", "cgi_extension", "cgi_path", "upload_enable", "upload_path", "client_max_body_size","return", nullptr};
static const char *methods[] = {"POST", "GET", "DELETE", nullptr};

class parsingConfig
{

public:
    struct location
    {
        std::string name;
        std::string root;
        std::vector<std::string> methods;
        bool autoindex;
        std::string index;
        std::vector<std::string> cgi_extension;
        std::string return_pages;
        std::string cgi_path;
        bool upload_enable;
        std::string upload_path;
        size_t client_max_body_size;
    };

    struct server
    {
        std::vector<std::string> names;
        std::string host;
        std::string root;
        std::map<int, std::string> error_pages;
        std::vector<location> locations;
        size_t port;
        size_t client_max_body_size;
        std::map<std::string, std::string> extToType;
        std::map<std::string, std::string> typeToExt;
    };

    parsingConfig();
    explicit parsingConfig(const std::string &file);
    std::string readFile(const std::string &file);
    size_t countLines(std::string src);
    std::string getLine(std::string src, size_t n);
    std::vector<std::string> splitWS(std::string str);
    bool isSkippable(const std::string &src, size_t line);
    bool endsWithOB(const std::string &src, size_t line);
    size_t getCBracket(const std::string &src, size_t line);
    server initServer();
    location initLoc();
    bool isPropValid(const std::string &name, const char **vNames);
    std::vector<std::string> parseProp(const std::string &src, size_t line, const std::string &obj);
    bool isMethodValid(const std::string &method);
    size_t sToI(const std::string &str);
    void parseLocProp(const std::string &src, size_t n, location &l);
    location parseLocation(const std::string &src, size_t lineS, size_t lineE);
    void parseServerProp(const std::string &src, size_t n, server &s);
    void parseServer(const std::string &src, size_t lineS, size_t lineE);
    void parseMimeTypes(std::string filename,server &s);
    void validateConfig();
    std::vector<server> getServers();
    void print();

    class parsingException : public std::exception
    {
        std::string msg;
    public:
        explicit parsingException(const std::string &msg) : msg(msg) {}
        ~parsingException() throw() {}
        const char *what() const throw()
        {
            return msg.c_str();
        }
    };

private:
    std::vector<server> servers;

};
typedef parsingConfig::server Server;

#endif