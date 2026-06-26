#pragma once

#include "balancer/Backend.hpp"
#include "balancer/RoundRobinBalancer.hpp"
#include "health/HealthChecker.hpp"

#include <memory>
#include <mutex>
#include <vector>

class Server {
public:
    Server(int port, std::shared_ptr<std::vector<Backend>> backends);
    Server(int port, const std::vector<Backend>& backends);

    ~Server();

    void start();
    void stop();

private:
    int port_;
    int server_fd_;
    bool is_running_;

    std::shared_ptr<std::vector<Backend>> backends_;
    std::mutex backends_mutex_;

    RoundRobinBalancer balancer_;
    HealthChecker health_checker_;

    void handleClient(int client_fd);
    int connectToBackend(const Backend& backend);
};