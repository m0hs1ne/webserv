

#include "WebServ.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return 0;
    }
    try
    {
        WebServ Myserver(av[1]);
        Myserver.RunServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}