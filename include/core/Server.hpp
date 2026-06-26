#pragma once

#include "balancer/Backend.hpp"
#include "balancer/RoundRobinBalancer.hpp"
#include "health/HealthChecker.hpp"

#include <string>
#include <vector>
#include <memory>

class Server {
public:
    // Constructor to initialize server port
    Server(int port, std::shared_ptr<std::vector<Backend>> backends);
    Server(int port, const std::vector<Backend>& backends);
    
    // Destructor to clean up sockets
    ~Server();

    // Starts the server and begins listening for clients
    void start();

    // Stops the server loop and closes connections
    void stop();

private:
    int port_;
    int server_fd_;
    bool is_running_;

    std::shared_ptr<std::vector<Backend>> backends_;
    
    RoundRobinBalancer balancer_;
    HealthChecker health_checker_;

    // Internal helper to handle incoming client data
    void handleClient(int client_fd);
    int connectToBackend(const Backend& backend);
};

