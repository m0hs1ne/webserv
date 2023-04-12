#include "../includes/tools.hpp"

static const char *methods[] = {"POST", "GET", "DELETE", nullptr};

// extern std::map<int, std::string> code;

size_t sToi(const std::string &str)
{
    size_t value;
    std::istringstream convert(str);

    if (!(convert >> value))
        std::cerr << "Error: not a number" << std::endl;
    if (value < 0)
        std::cerr << "Error: negative number" << std::endl;
    return value;
}

std::string generateRandomString()
{
    srand(time(NULL));
    int length = rand() % 10 + 5;

    std::string random_string = "";
    for (int i = 0; i < length; i++)
    {
        char random_char = 'a' + rand() % 26;
        random_string += random_char;
    }

    return random_string;
}

size_t countLines(std::string &src)
{
    size_t lines = 1;

    for (std::string::const_iterator it = src.begin(); it != src.end(); ++it)
    {
        if (*it == '\n')
            ++lines;
    }
    return lines;
}

void initHttpCode(std::map<int, std::string> &code)
{
    if (!code.size())
    {
        code[501] = "501 Not Implemented";
        code[500] = "500 Internal Server Error";
        code[414] = "414 Request-URI Too Long";
        code[413] = "413 Request Entity Too Large";
        code[409] = "409 Conflict";
        code[405] = "405 Method Not Allowed";
        code[404] = "404 Not Found";
        code[403] = "403 Forbidden";
        code[400] = "400 Bad Request";
        code[301] = "301 Moved Permanently";
        code[204] = "204 No Content";
        code[201] = "201 Created";
        code[200] = "200 OK";
    }
}

void freeCharArray(char **charArray)
{
    for (int i = 0; charArray[i]; i++)
    {
        delete[] charArray[i];
    }

    delete[] charArray;
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
        std::cout << "Error: " << file <<" not found" << std::endl;
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

void copyByteByByte(std::string &str1, std::string &str2)
{
    for (size_t i = 0; i < sizeof(str2.c_str()); i++)
        str1[sizeof(str1.c_str()) + i] = str2[i];
}

std::string *getLine(std::string &src, size_t n, size_t size)
{
    size_t i = 0;
    size_t j = 1;
    size_t lineCount = 0;
    std::string *ret = new std::string();

    if (n >= countLines(src))
        return ret;
    while (lineCount < n)
    {
        if (i < size && src[i++] == '\n')
            ++lineCount;
    }
    if (src[i] == '\n')
        i++;
    while (i + j < size && src[i + j] != '\n')
        j++;
    *ret = std::string(src, i, j);
    return (ret);
}

std::string *getLine(std::string &src, size_t n, size_t size, size_t *len, std::string optional)
{
    size_t i = 0;
    size_t j = 1;
    size_t lineCount = 0;
    std::string *ret = new std::string();

    if (n >= countLines(src))
    {
        *len = 0;
        return ret;
    }
    while (lineCount < n)
    {
        if (i < size && src[i++] == '\n')
            ++lineCount;
    }
    if (src[i] == '\n')
        i++;
    // while (i < size && src[i] != '\n')
    //     i++;
    while (i + j < size && src[i + j] != '\n')
        j++;
    if (optional == "size")
        *len = j;
    else if (optional == "pos")
        *len = i;
    *ret = std::string(src, i, j);
    return (ret);
}

std::string *getLine(std::string &src, size_t n, size_t size, size_t pos)
{
    size_t i = pos;
    size_t j = 1;
    std::string *ret = new std::string();
    (void)n;
    // while (i < size && src[i] != '\n')
    //     i++;
    while (i + j < size && src[i + j] != '\n')
        j++;
    *ret = std::string(src, i, j);
    return (ret);
}

// std::string getLine(std::string src, size_t n)
// {
//     size_t i = 0;
//     size_t j = 0;
//     size_t lineCount = 0;

//     if (n >= countLines(src))
//         return std::string();
//     while (lineCount < n)
//     {
//         if (src[i++] == '\n')
//             ++lineCount;
//     }
//     while (src[i] != '\n')
//         i++;
//     while (src[i + j] && src[i + j] != '\n')
//         j++;
//     while (j > 0)
//         --j;
//     return (std::string(src, i, j));
// }

char *ft_strdup(const char *str, size_t size)
{
    char *dup = new char[size];

    for (size_t i = 0; i < size; i++)
        dup[i] = str[i];
    return dup;
}

std::vector<std::string> splitString(std::string str, std::string delimiter, size_t size)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = findByteByByte(str, delimiter, size, delimiter.size());
    if (end == std::string::npos)
        return tokens;
    while (end != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string getLastLine(const std::string &str)
{
    std::string::size_type pos = str.find_last_of("\n\r");
    if (pos == std::string::npos)
    {
        return str;
    }
    else if (pos == str.size() - 1)
    {
        std::string::size_type prevPos = str.find_last_of("\n\r", pos - 1);
        if (prevPos == std::string::npos)
        {
            return str.substr(pos + 1);
        }
        else
        {
            return str.substr(prevPos + 1, pos - prevPos - 1);
        }
    }
    else
    {
        return str.substr(pos + 1);
    }
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
    for (size_t i = 0; i < str.size(); i++)
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

size_t findByteByByte(const std::string &str1, const std::string &str2, size_t str1Size, size_t str2Size)
{
    if (str2Size > str1Size)
        return std::string::npos;

    for (size_t i = 0; i < str1Size - str2Size + 1; i++)
    {
        bool match = true;
        for (size_t j = 0; j < str2Size; j++)
        {
            if (str1[i + j] != str2[j])
            {
                match = false;
                break;
            }
        }
        if (match)
            return i;
    }
    return std::string::npos;
}
