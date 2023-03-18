#include "../includes/tools.hpp"

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

std::string urlDecodeStr(std::string str)
{
    std::ostringstream decoded;
    std::string rslt;

    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if (*it == '%')
        {
            std::istringstream hex_string(std::string(it + 1, it + 3));
            int hex_value = 0;
            hex_string >> std::hex >> hex_value;

            decoded << static_cast<char>(hex_value);
            it += 2;
        }
        else if (*it == '+')
        {
            decoded << ' ';
        }
        else
        {
            decoded << *it;
        }
    }
    rslt = decoded.str();
    return rslt;
}

bool locationFind(std::string path, std::string location)
{
    size_t i = 0;
    size_t notFound = 0;

    while (path[i] && location[i])
    {
        if (path[i] != location[i])
            notFound = 1;
        ++i;
    }
    if (location[i] == '\0' && !notFound)
        return true;
    return false;
}

std::string dToh(size_t d)
{
    std::stringstream ss;
    ss << std::hex << d;
    return ss.str();
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
    std::stringstream convert;
    std::string res;
    convert << n;
    res = convert.str();
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

std::vector<std::string> splitString( std::string str,  std::string delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
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

std::string toUpper(std::string str)
{
    for(size_t i = 0; i < str.size(); i++)
    {
        str[i] = std::toupper(str[i]);
    }
    return str;
}

int isDir(const char *pathname)
{
    struct stat info;
    if (stat(pathname, &info) != 0)
        return 1;
    else if (info.st_mode & S_IFDIR)
        return -1;
    else
        return 0;
}