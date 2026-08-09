#pragma once

#include "balancer/Backend.hpp"
#include "balancer/RoundRobinBalancer.hpp"
#include "health/HealthChecker.hpp"
#include "ThreadPool.hpp"
#include "observability/Metrics.hpp"

#include <memory>
#include <mutex>
#include <vector>

class Server {
public:
    Server(int port, std::shared_ptr<std::vector<Backend>> backends, std::size_t workerCount = 4);
    Server(int port, const std::vector<Backend>& backends, std::size_t workerCount = 4);

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
    ThreadPool thread_pool_;
    Metrics metrics_;

    void handleClient(int client_fd);
    int connectToBackend(const Backend& backend);
    void sendStatusResponse(int client_fd);
    void sendMetricsResponse(int client_fd);
};