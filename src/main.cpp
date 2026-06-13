#include <iostream>
#include <vector>

#include "balancer/Backend.hpp"
#include "balancer/RoundRobinBalancer.hpp"

int main()
{
    std::vector<Backend> backends = {
        {"127.0.0.1", 9001},
        {"127.0.0.1", 9002},
        {"127.0.0.1", 9003}
    };

    RoundRobinBalancer balancer(backends);

    std::cout << "Testing Round Robin Load Balancer\n\n";

    for (int i = 0; i < 10; i++) {
        Backend selected = balancer.getNextBackend();

        std::cout << "Request " << i + 1
                  << " -> "
                  << selected.host << ":"
                  << selected.port
                  << '\n';
    }

    return 0;
}