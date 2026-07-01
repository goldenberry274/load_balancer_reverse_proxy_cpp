#include "core/Server.hpp"
#include "balancer/Backend.hpp"
#include "config/config.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    std::string configPath = "config.yaml";

    if (argc > 1) {
        configPath = argv[1];
    }

    try {
        Config config(configPath);

        Server server(
            config.listenPort,
            config.backends
        );

        server.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to start load balancer: "
                  << e.what()
                  << "\n";
        return 1;
    }

    return 0;
}