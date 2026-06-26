#pragma once

#include "balancer/Backend.hpp"

#include <vector>
#include <string>

class Config {
public:
    int listenPort = 8080;
    int healthCheckInterval = 3;
    std::vector<Backend> backends;

    explicit Config(const std::string& filename);
};