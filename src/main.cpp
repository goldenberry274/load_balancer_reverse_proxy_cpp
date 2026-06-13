#include <iostream>
#include <vector>


#include "core/Server.hpp"


int main()
{
    Server server(8080);
    server.start();

    return 0;
}