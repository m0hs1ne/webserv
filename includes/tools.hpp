#ifndef TOOLS_HPP
#define TOOLS_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cstdlib>

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
std::string readFile(const std::string &file);
size_t countLines(std::string src);
std::string getLine(std::string src, size_t n);
std::vector<std::string> split(std::string str, char c, int stop);
bool isMethodValid(const std::string &method);
size_t sToI(const std::string &str);
std::string itos(size_t n);
std::string toUpper(std::string str);
#endif