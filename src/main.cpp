#include "core/Server.hpp"
#include "balancer/Backend.hpp"

#include <vector>


int main() {

    std::vector<Backend> backends = {
        {"127.0.0.1", 9001},
        {"127.0.0.1", 9002},
        {"127.0.0.1", 9003}
    };

    Server server(8080, backends);
    server.start();

    return 0;
}