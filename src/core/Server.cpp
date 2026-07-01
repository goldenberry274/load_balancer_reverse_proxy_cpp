#include "core/Server.hpp"
#include "observability/Logger.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(int port, std::shared_ptr<std::vector<Backend>> backends)
    : port_(port),
      server_fd_(-1),
      is_running_(false),
      backends_(std::move(backends)),
      balancer_(backends_, backends_mutex_),
      health_checker_(backends_, backends_mutex_, 3)
{
}

Server::Server(int port, const std::vector<Backend>& backends)
    : Server(
          port,
          std::make_shared<std::vector<Backend>>(backends)
      )
{
}
Server::~Server() {
    stop();
}

void Server::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        Logger::error("Socket creation failed\n");
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
        Logger::error("Bind failed on port " + std::to_string(port_) + "\n");
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        Logger::error("Listen failed\n");
        return;
    }

    is_running_ = true;
    Logger::info("Server started on port " + std::to_string(port_) + "\n");
    health_checker_.start();

    while (is_running_) {
        sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);

        int client_fd = accept(
            server_fd_,
            (sockaddr*)&client_addr,
            &addrlen
        );

        if (client_fd < 0) {
            if (is_running_) {
                Logger::error("Accept connection failed\n");
            }
            continue;
        }

        handleClient(client_fd);
    }
}

int Server::connectToBackend(const Backend& backend) {
    int backend_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (backend_fd < 0) {
        Logger::error("Backend socket creation failed\n");
        return -1;
    }

    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend.port);

    if (inet_pton(AF_INET, backend.host.c_str(), &backend_addr.sin_addr) <= 0) {
        Logger::error("Invalid backend address: " + backend.host + "\n");
        close(backend_fd);
        return -1;
    }

    if (connect(
            backend_fd,
            (sockaddr*)&backend_addr,
            sizeof(backend_addr)) < 0)
    {
        Logger::error("Failed to connect to backend " + backend.host + ":"
                        + std::to_string(backend.port) + "\n");


        close(backend_fd);
        return -1;
    }

    return backend_fd;
}

void Server::handleClient(int client_fd) {
    char buffer[4096] = {0};

    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));

    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    Backend backend;

    try {
        backend = balancer_.getNextBackend();
    } catch (const std::exception& e) {
        std::string response =
            "HTTP/1.1 503 Service Unavailable\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "No healthy backends available";

        send(client_fd, response.c_str(), response.size(), 0);
        close(client_fd);
        return;
    }
    Logger::info("Forwarding request to backend " + backend.host + ":" + std::to_string(backend.port) +"\n");

    int backend_fd = connectToBackend(backend);

    if (backend_fd < 0) {
        std::string error_response =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Backend unavailable";

        write(client_fd, error_response.c_str(), error_response.length());
        close(client_fd);
        return;
    }

    send(backend_fd, buffer, bytes_read, 0);

    char response_buffer[4096] = {0};

    ssize_t backend_bytes =
        recv(backend_fd, response_buffer, sizeof(response_buffer), 0);

    if (backend_bytes > 0) {
        send(client_fd, response_buffer, backend_bytes, 0);
    }

    close(backend_fd);
    close(client_fd);
}

void Server::stop() {
    if (is_running_) {
        is_running_ = false;
        health_checker_.stop();
    }

    if (server_fd_ != -1) {
        close(server_fd_);
        server_fd_ = -1;
    }
}