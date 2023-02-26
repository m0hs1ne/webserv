#include "inc/tools.hpp"

static const char *methods[] = {"POST", "GET", "DELETE", nullptr};

size_t countLines(std::string src)
{
    size_t i = 0;
    size_t lines = 1;

    while (src[i])
    {
        if (src[i++] == '\n')
            ++lines;
    }
    return lines;
}

std::string readFile(const std::string &file)
{
    std::ifstream ifs(file);
    if (!ifs)
        throw parsingException("Error opening file");
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

std::string itos(size_t n)
{
    std::istringstream convert(n);
    std::string res;

    convert >> res;
    return res;
}

std::string getLine(std::string src, size_t n)
{
    size_t i = 0;
    size_t j = 0;
    size_t lineCount = 0;

    if (n >= countLines(src))
        return std::string();
    while (lineCount < n)
    {
        if (src[i++] == '\n')
            ++lineCount;
    }
    while (std::isspace(src[i]) && src[i] != '\n')
        i++;
    while (src[i + j] && src[i + j] != '\n')
        j++;
    while (j > 0 && std::isspace(src[i + j - 1]))
        --j;
    return (std::string(src, i, j));
}

std::vector<std::string> split(std::string str, char c, int stop)
{
    std::vector<std::string> res;
    size_t i = 0;
    size_t j = 0;

    while (str[i])
    {
        if (str[i] == c)
        {
            if (i == j)
                ++j;
            else
            {
                res.push_back(std::string(str, j, i - j));
                j = i + 1;
            }
            if (stop)
                break;
        }
        ++i;
    }
    if (i != j)
        res.push_back(std::string(str, j, i - j));
    return res;
}

bool isMethodValid(const std::string &method)
{
    size_t i = 0;

    while (methods[i])
    {
        if (method == methods[i])
            return true;
        i++;
    }
    return false;
}