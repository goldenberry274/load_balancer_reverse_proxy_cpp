#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <stdexcept>

#include "LoadBalancer.hpp"

using namespace std;

class RoundRobinBalancer : public LoadBalancer{
    private:
        vector<Backend> backends;
        size_t current = 0;
    public:
        explicit RoundRobinBalancer(const vector<Backend>& backends);
        Backend getNextBackend() override;
};