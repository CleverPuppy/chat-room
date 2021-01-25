#include <iostream>

#include "server.h"


int main()
{
    std::cout <<"Hello World!\n"<<std::endl;
    Server server{};
    server.run();
    return 0;
}