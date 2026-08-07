#include "core/Server.hpp"
#include "observability/Logger.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(
    int port,
    std::shared_ptr<std::vector<Backend>> backends,
    std::size_t worker_count
)
    : port_(port),
      server_fd_(-1),
      is_running_(false),
      backends_(std::move(backends)),
      balancer_(backends_, backends_mutex_),
      health_checker_(backends_, backends_mutex_, 3),
      thread_pool_(worker_count)
{
}

Server::Server(
    int port,
    const std::vector<Backend>& backends,
    std::size_t worker_count
)
    : Server(
          port,
          std::make_shared<std::vector<Backend>>(backends),
          worker_count
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
        Logger::error("Bind failed on port " + std::to_string(port_));
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        Logger::error("Listen failed\n");
        return;
    }

    is_running_ = true;
    Logger::info("Server started on port " + std::to_string(port_));
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
                Logger::error("Accept connection failed");
            }
            continue;
        }

        try {
            thread_pool_.enqueue(
                [this, client_fd]()
                {
                    handleClient(client_fd);
                }
            );
        }
        catch (const std::exception& e) {
            Logger::error(
                "Failed to enqueue client: " +
                std::string(e.what())
            );

            close(client_fd);
        }
    }
}

int Server::connectToBackend(const Backend& backend) {
    int backend_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (backend_fd < 0) {
        Logger::error("Backend socket creation failed");
        return -1;
    }

    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend.port);

    if (inet_pton(AF_INET, backend.host.c_str(), &backend_addr.sin_addr) <= 0) {
        Logger::error("Invalid backend address: " + backend.host);
        close(backend_fd);
        return -1;
    }

    if (connect(
            backend_fd,
            (sockaddr*)&backend_addr,
            sizeof(backend_addr)) < 0)
    {
        Logger::error("Failed to connect to backend " + backend.host + ":"
                        + std::to_string(backend.port));


        close(backend_fd);
        return -1;
    }

    return backend_fd;
}

void Server::handleClient(int client_fd)
{
    char buffer[4096] = {0};

    const ssize_t bytes_read =
        read(client_fd, buffer, sizeof(buffer));

    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    const std::string request(buffer, bytes_read);

    // Internal route handled by the load balancer itself.
    if (request.rfind("GET /status ", 0) == 0) {
        sendStatusResponse(client_fd);
        close(client_fd);
        return;
    }

    Backend backend;

    try {
        backend = balancer_.getNextBackend();
    } catch (const std::exception& e) {
        const std::string response =
            "HTTP/1.1 503 Service Unavailable\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 29\r\n"
            "Connection: close\r\n"
            "\r\n"
            "No healthy backends available";

        send(client_fd, response.data(), response.size(), 0);
        close(client_fd);
        return;
    }

    Logger::info(
        "Forwarding request to backend " +
        backend.host + ":" +
        std::to_string(backend.port)
    );

    const int backend_fd = connectToBackend(backend);

    if (backend_fd < 0) {
        const std::string error_response =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 19\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Backend unavailable";

        send(
            client_fd,
            error_response.data(),
            error_response.size(),
            0
        );

        close(client_fd);
        return;
    }

    // Forward the complete client request.
    std::size_t request_sent = 0;

    while (request_sent < static_cast<std::size_t>(bytes_read)) {
        const ssize_t sent = send(
            backend_fd,
            buffer + request_sent,
            static_cast<std::size_t>(bytes_read) - request_sent,
            0
        );

        if (sent <= 0) {
            Logger::error("Failed to forward request to backend");
            close(backend_fd);
            close(client_fd);
            return;
        }

        request_sent += static_cast<std::size_t>(sent);
    }

    char response_buffer[4096];

    // Forward every response chunk until the backend closes its connection.
    while (true) {
        const ssize_t backend_bytes = recv(
            backend_fd,
            response_buffer,
            sizeof(response_buffer),
            0
        );

        if (backend_bytes == 0) {
            break;
        }

        if (backend_bytes < 0) {
            Logger::error(
                "Failed to receive response from backend " +
                backend.host + ":" +
                std::to_string(backend.port)
            );
            break;
        }

        std::size_t response_sent = 0;

        while (
            response_sent <
            static_cast<std::size_t>(backend_bytes)
        ) {
            const ssize_t sent = send(
                client_fd,
                response_buffer + response_sent,
                static_cast<std::size_t>(backend_bytes) -
                    response_sent,
                0
            );

            if (sent <= 0) {
                Logger::error(
                    "Failed to forward backend response to client"
                );

                close(backend_fd);
                close(client_fd);
                return;
            }

            response_sent += static_cast<std::size_t>(sent);
        }
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
void Server::sendStatusResponse(int client_fd)
{
    std::string body = "Load Balancer Status\n\n";

    {
        std::lock_guard<std::mutex> lock(backends_mutex_);

        for (const Backend& backend : *backends_) {
            body += backend.host + ":" + std::to_string(backend.port) + "\n";
            body += "Healthy: ";
            body += backend.healthy ? "yes\n" : "no\n";
            body += "Requests served: "
                 + std::to_string(backend.requestsServed)
                 + "\n\n";
        }
    }

    const std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n"
        + body;

    std::size_t total_sent = 0;

    while (total_sent < response.size()) {
        const ssize_t sent = send(
            client_fd,
            response.data() + total_sent,
            response.size() - total_sent,
            0
        );

        if (sent <= 0) {
            Logger::error("Failed to send status response");
            return;
        }

        total_sent += static_cast<std::size_t>(sent);
    }

    Logger::info("Status response sent");
}