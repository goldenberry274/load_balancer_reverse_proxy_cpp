#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <stdexcept>
#include <memory>

#include "LoadBalancer.hpp"

using namespace std;

class RoundRobinBalancer : public LoadBalancer{
    private:
        shared_ptr<vector<Backend>> backends;
        size_t current = 0;
    public:
        //For debugging purposes
        explicit RoundRobinBalancer(initializer_list<Backend> backends);
        explicit RoundRobinBalancer(shared_ptr<vector<Backend>> backends);
        Backend getNextBackend() override;
};