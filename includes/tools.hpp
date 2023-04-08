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
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>


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
// static std::map<int, std::string> code;
char *ft_strdup(const char *str, size_t size);
std::string getLastLine(const std::string& str);
std::string setContentType(std::string path);
std::vector<std::string> splitString(std::string str, std::string delimiter, size_t size);
void copyByteByByte(std::string &str1, std::string &str2);
std::string readFile(const std::string &file);
std::string dToh(size_t d);
size_t countLines(std::string &src);
std::string *getLine(std::string &src, size_t n, size_t size);
std::string *getLine(std::string &src, size_t n, size_t size, size_t *len);
std::vector<std::string> split(std::string str, char c, int stop);
bool isMethodValid(const std::string &method);
size_t sToi(const std::string &str);
std::string itos(size_t n);
std::string toUpper(std::string str);
int isDir(const char *pathname);
std::string urlDecodeStr(std::string &str);
void freeCharArray(char** charArray);
void initHttpCode(std::map<int, std::string> &code);
std::string generateRandomString();
size_t findByteByByte(const std::string& str1, const std::string& str2, size_t str1Size, size_t str2Size);

#endif