#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cstdlib>

std::string readFile(std::string file)
{
    std::ifstream ifs(file);
    if (!ifs)
        std::cout << "Error while reading" << file << std::endl;
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

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

std::vector<std::string> splitWS(std::string str)
{
    std::vector<std::string> res;
    size_t i = 0;
    size_t j = 0;

    while (str[i])
    {
        if (std::isspace(str[i]))
        {
            if (i == j)
                ++j;
            else
            {
                res.push_back(std::string(str, j, i - j));
                j = i + 1;
            }
        }
        ++i;
    }
    if (i != j)
        res.push_back(std::string(str, j, i - j));
    return res;
}

bool isSkippable(std::string src, size_t line)
{
    std::string l;

    l = getLine(src, line);
    return (splitWS(l).size() == 0 || l.size() == 0 || l[0] == '#');
}

bool endsWithOB(std::string src, size_t line)
{
    std::vector<std::string> splits;

    splits = splitWS(getLine(src, line));
    if (splits.size() > 0)
    {
        if (splits[splits.size() - 1] == "{")
            return true;
    }
    return false;
}

size_t getCBracket(std::string src, size_t line)
{
    size_t n;
    size_t size;
    size_t oBrackets = 0;

    if (getLine(src, line)[getLine(src, line).size() - 1] != '{')
        std::cout << line << " " << "Expected '{'" << std::endl;
    n = line + 1;
    size = countLines(src);
    while (n < size)
    {
        if (!isSkippable(src, n) && endsWithOB(src, n))
            ++oBrackets;
        if (!isSkippable(src, n) && getLine(src, n) == "}")
        {
            if (oBrackets == 0)
                return n;
            --oBrackets;
        }
        ++n;
    }
    std::cout << line << " " << "Expected '}'" << std::endl;
    return -1;
}

void parseConfig(std::string file)
{
    std::string fileContent;
    size_t i;
    size_t size;
    std::vector<std::string> line;

    i = 0;
    fileContent = readFile(file);
    size = countLines(fileContent);
    while (i < size)
    {
        if (!isSkippable(fileContent, i))
        {
            line = splitWS(getLine(fileContent, i));
            if (line.size() > 0 && line[0] == "server")
            {
                // parseServer(fileContent, i, getCBracket(fileContent,i));
                std::cout << getCBracket(fileContent, i ) << std::endl;
            }
        }
        ++i;
    }
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Usage: ./webServ <file>" << std::endl;
        return 1;
    }
    parseConfig(av[1]);
}