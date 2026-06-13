#include "balancer/RoundRobinBalancer.hpp"

#include <stdexcept>

RoundRobinBalancer::RoundRobinBalancer(
    const std::vector<Backend>& backends)
    : backends(backends),
      current(0)
{
    if (backends.empty()) {
        throw std::invalid_argument(
            "RoundRobinBalancer requires at least one backend");
    }
}

Backend RoundRobinBalancer::getNextBackend()
{
    Backend selected = backends[current];

    current = (current + 1) % backends.size();

    return selected;
}