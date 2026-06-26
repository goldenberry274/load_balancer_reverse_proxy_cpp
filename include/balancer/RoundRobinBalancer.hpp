#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <stdexcept>
#include <mutex>
#include <memory>

#include "LoadBalancer.hpp"

using namespace std;

class RoundRobinBalancer : public LoadBalancer{
    private:
        shared_ptr<vector<Backend>> backends;
        mutex& mutex_;
        size_t current = 0;
    public:
        //For debugging purposes
        explicit RoundRobinBalancer(initializer_list<Backend> backends, mutex& mutex);
        explicit RoundRobinBalancer(shared_ptr<vector<Backend>> backends, mutex& mutex);
        Backend getNextBackend() override;
};