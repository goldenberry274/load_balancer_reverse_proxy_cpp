#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#include "balancer/Backend.hpp"

class HealthChecker {
private:
    std::shared_ptr<std::vector<Backend>> backends_;
    //std::mutex& backends_mutex_;

    std::thread worker_;
    std::atomic<bool> running_;

    int interval_seconds_;

    bool checkBackend(const Backend& backend);
    void loop();

public:
    HealthChecker(
        std::shared_ptr<std::vector<Backend>> backends,
        //std::mutex& backends_mutex,
        int interval_seconds = 3
    );

    ~HealthChecker();

    void start();
    void stop();
};