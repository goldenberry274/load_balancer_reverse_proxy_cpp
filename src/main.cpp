#include "core/Server.hpp"
#include "balancer/Backend.hpp"
#include "config/config.hpp"

#include <vector>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2){
        std::cout << "Usage: " << argv[0] << " <config_path> " << std::endl;
        return 1;
    }
    
    std::string config_path = argv[1];
    Config config(config_path);

    Server server(
        config.listenPort,
        config.backends
    );

    server.start();

    return 0;
}