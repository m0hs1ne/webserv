#include "parseConfig.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Usage : ./webServ file.conf" << std::endl;
        return 0;
    }
    try
    {
        parsingConfig a(av[1]);
        a.print();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}