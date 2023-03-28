#include "ParsingConfig.hpp"

std::string parsingConfig::readFile(const std::string &file)
{
    std::ifstream ifs(file);
    if (!ifs)
        throw parsingException("Error opening file");
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

size_t parsingConfig::countLines(std::string src)
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

std::string parsingConfig::getLine(std::string src, size_t n)
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

std::vector<std::string> parsingConfig::splitWS(std::string str)
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

bool parsingConfig::isSkippable(const std::string &src, size_t line)
{
    std::string l;

    l = getLine(src, line);
    return (splitWS(l).empty() || l.empty() || l[0] == '#');
}

bool parsingConfig::endsWithOB(const std::string &src, size_t line)
{
    std::vector<std::string> splits;

    splits = splitWS(getLine(src, line));
    if (!splits.empty())
    {
        if (splits[splits.size() - 1] == "{")
            return true;
    }
    return false;
}

size_t parsingConfig::getCBracket(const std::string &src, size_t line)
{
    size_t n;
    size_t size;
    size_t oBrackets = 0;

    if (getLine(src, line)[getLine(src, line).size() - 1] != '{')
        throw parsingException("Expected '{'");
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
    throw parsingException("Expected '}'");
    return -1;
}

parsingConfig::server parsingConfig::initServer()
{
    server s;
    s.port = 80;
    s.host = "127.0.0.1";
    s.root = "";
    return (s);
}

parsingConfig::location parsingConfig::initLoc()
{
    location l;
    l.name = "/";
    l.root = "";
    l.index = "";
    l.autoindex = false;
    l.upload_path = "";
    l.client_max_body_size = 1048576; // bytes
    return l;
}

bool parsingConfig::isPropValid(const std::string &name, const char **vNames)
{
    size_t i = 0;

    while (vNames[i])
    {
        if (name == vNames[i])
            return true;
        i++;
    }
    return false;
}

std::vector<std::string> parsingConfig::parseProp(const std::string &src, size_t line, const std::string &obj)
{
    std::vector<std::string> res;
    std::string l;

    l = getLine(src, line);
    if (l[l.size() - 1] != ';')
        throw parsingException("Expected ';'");
    l = std::string(l, 0, l.size() - 1);
    res = splitWS(l);
    if (res.size() <= 1)
        throw parsingException("Expected property parameters");
    if (!isPropValid(res[0], obj == "location" ? locProp : serverProp))
        throw parsingException("Invalid property name");
    return res;
}

bool parsingConfig::isMethodValid(const std::string &method)
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

size_t parsingConfig::sToI(const std::string &str)
{
    size_t value;
    std::istringstream convert(str);

    if (!(convert >> value))
        throw parsingException("not positive integer");
    if (value < 0)
        throw parsingException("not positive integer");
    return value;
}

void parsingConfig::parseLocProp(const std::string &src, size_t n, location &l)
{
    std::vector<std::string> line;
    char last;

    line = parseProp(src, n, "location");
    if (line[0] == locProp[0])
    {
        for (size_t i = 1; i < line.size(); ++i)
        {
            if (!isMethodValid(line[i]))
                throw parsingException("Invalid method");
            else
                l.methods.push_back(line[i]);
        }
    }
    if (line[0] == locProp[1])
        l.root = line[1];
    if (line[0] == locProp[2])
    {
        if (line[1] == "on")
            l.autoindex = true;
        else if (line[1] == "off")
            l.autoindex = false;
        else
            throw parsingException("Autoindex param should be 'on' or 'off'");
    }
    if (line[0] == locProp[3])
        l.index = line[1];
    if (line[0] == locProp[4])
    {
        for (size_t i = 1; i < line.size(); ++i)
            l.cgi_extension.push_back(line[i]);
    }
    if (line[0] == locProp[5])
        l.cgi_path = line[1];
    if (line[0] == locProp[6])
    {
        if (line[1] == "on")
            l.upload_enable = true;
        else if (line[1] == "off")
            l.upload_enable = false;
        else
            throw parsingException("Upload_enable param should be 'on' or 'off'");
    }
    if (line[0] == locProp[7])
        l.upload_path = line[1];
    if (line[0] == locProp[8])
    {
        if (line.size() != 2)
            throw parsingException("Invalid client_max_body_size param");
        l.client_max_body_size = sToI(line[1]);
        last = line[1][line[1].size() - 1];
        if (last == 'K' || last == 'k')
            l.client_max_body_size *= 1024;
        else if (last == 'M' || last == 'm')
            l.client_max_body_size *= 1024 * 1024;
        else if (last == 'G' || last == 'g')
            l.client_max_body_size *= 1024 * 1024 * 1024;
        else if (!std::isdigit(last))
            throw parsingException("Invalid client_max_body_size param");
        
    }
    if(line[0] == locProp[9])
    {
        if(line.size() != 3)
            throw parsingException("Invalid redirect param");
        if(line[1] == "301")
            l.return_pages = line[2];
        else
            throw parsingException("Invalid redirect param");
    }
}

parsingConfig::location parsingConfig::parseLocation(const std::string &src, size_t lineS, size_t lineE)
{
    location l;
    std::vector<std::string> line;

    l = initLoc();
    line = splitWS(getLine(src, lineS));
    if (line.size() != 3)
        throw parsingException("Invalid location name");
    l.name = line[1];
    for (size_t n = lineS + 1; n < lineE; ++n)
    {
        if (!isSkippable(src, n))
            parseLocProp(src, n, l);
    }
    return l;
}

void parsingConfig::parseServerProp(const std::string &src, size_t n, server &s)
{
    std::vector<std::string> line;

    line = parseProp(src, n, "server");
    if (line[0] == serverProp[0])
    {
        if (line.size() != 3)
            throw parsingException("<port> <host>");
        s.port = sToI(line[1]);
        s.host = line[2];
    }
    if (line[0] == serverProp[1])
    {
        for (size_t i = 1; i < line.size(); ++i)
            s.names.push_back(line[i]);
    }
    if (line[0] == serverProp[2])
    {
        if (line.size() != 3)
            throw parsingException("<code> <path>");
        s.error_pages[sToI(line[1])] = line[2];
    }
    if (line[0] == serverProp[3])
        s.root = line[1];
}

void parsingConfig::parseServer(const std::string &src, size_t lineS, size_t lineE)
{
    server s;
    std::vector<std::string> line;

    s = initServer();
    for (size_t n = lineS + 1; n < lineE; ++n)
    {
        if (!isSkippable(src, n))
        {
            std::vector<std::string> words = splitWS(getLine(src, n));
            if (!words.empty() && words[0] == "location")
            {
                s.locations.push_back(parseLocation(src, n, getCBracket(src, n)));
                n = getCBracket(src, n);
            }
            else
                parseServerProp(src, n, s);
        }
    }
    servers.push_back(s);
}

void parsingConfig::validateConfig()
{
    if (servers.empty())
        throw parsingException("No server found");
    for (size_t i = 0; i < servers.size(); ++i)
    {
        for (size_t j = 0; j < servers.size(); ++j)
        {
            if (i != j)
                if (servers[i].host == servers[j].host && servers[i].port == servers[j].port)
                    throw parsingException("Duplicate server");
        }
    }
}

parsingConfig::parsingConfig(const std::string &file)
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
            if (!line.empty() && line[0] == "server")
            {
                parseServer(fileContent, i, getCBracket(fileContent, i));
                i = getCBracket(fileContent, i);
            }
            else
                throw parsingException("Invalid config");
        }
        ++i;
    }
    validateConfig();
}

parsingConfig::parsingConfig() {}

std::vector<parsingConfig::server> parsingConfig::getServers()
{
    return servers;
}

void parsingConfig::print()
{
    std::map<int, std::string>::iterator it;
	std::vector<location>::iterator it2;

	for (size_t i = 0; i < servers.size(); i++)
	{
		std::cout << "- Server" << std::endl;
		std::cout << "   * server_name: ";
		for (size_t j = 0; j < servers[i].names.size(); ++j)
			std::cout << servers[i].names[j] << " ";
		std::cout << std::endl;
		std::cout << "   * host: " + servers[i].host << std::endl;
		std::cout << "   * port: " << servers[i].port << std::endl;
		std::cout << "   * root: " + servers[i].root << std::endl;
		it = servers[i].error_pages.begin();
		while (it != servers[i].error_pages.end())
		{
			std::cout << "   * error_page for " << it->first <<  ": " + it->second << std::endl;
			++it;
		}
		it2 = servers[i].locations.begin();
		while (it2 != servers[i].locations.end())
		{
			std::cout << "   - Location " + it2->name << std::endl;
			std::cout << "     * methods: ";
			for (size_t j = 0; j < it2->methods.size(); ++j)
				std::cout << it2->methods[j] + " ";
			std::cout << std::endl;
			std::cout << "     * root: " << it2->root << std::endl;
			std::cout << "     * cgi_extension: ";
			for (size_t j = 0; j < it2->cgi_extension.size(); ++j)
				std::cout << it2->cgi_extension[j] << " ";
			std::cout << std::endl;
			std::cout << "     * cgi_path: " << it2->cgi_path << std::endl;
			std::cout << "     * autoindex: " << it2->autoindex << std::endl;
			std::cout << "     * upload_enable: " << it2->upload_enable << std::endl;
			std::cout << "     * upload_path: " << it2->upload_path << std::endl;
			std::cout << "     * client_max_body_size: " <<  it2->client_max_body_size << std::endl;
			++it2;
		}
	}
}