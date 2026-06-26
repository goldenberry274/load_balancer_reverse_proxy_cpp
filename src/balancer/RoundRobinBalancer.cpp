#include "balancer/RoundRobinBalancer.hpp"

#include <stdexcept>

RoundRobinBalancer::RoundRobinBalancer(
    std::initializer_list<Backend> backends)
    : backends(std::make_shared<std::vector<Backend>>(backends)),
      current(0)
{
    if (std::empty(backends)) {
        throw std::invalid_argument(
            "RoundRobinBalancer requires at least one backend");
    }
}

RoundRobinBalancer::RoundRobinBalancer(
    std::shared_ptr<std::vector<Backend>> backends)
    : backends(std::move(backends)),
      current(0)
{
    if (!this->backends || this->backends->empty()) {
        throw std::invalid_argument(
            "RoundRobinBalancer requires at least one backend");
    }
}

Backend RoundRobinBalancer::getNextBackend()
{
    size_t attempts = 0;

    while (attempts < backends->size()) {
        Backend selected = backends->at(current);
        current = (current + 1) % backends->size();

        if (selected.healthy) {
            return selected;
        }

        attempts++;
    }

    throw std::runtime_error("No healthy backends available");
}