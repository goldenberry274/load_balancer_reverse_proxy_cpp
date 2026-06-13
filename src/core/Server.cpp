#include "core/Server.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server(int port) : port_(port), server_fd_(-1), is_running_(false) {}

Server::~Server() {
    stop();
}

void Server::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept links from any IP
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port_ << "\n";
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        std::cerr << "Listen failed\n";
        return;
    }

    is_running_ = true;
    std::cout << "Server successfully started on port " << port_ << "...\n";

    while (is_running_) {
        sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);

        int client_fd = accept(
            server_fd_,
            (struct sockaddr*)&client_addr,
            &addrlen
        );
        
        if (client_fd < 0) {
            if (is_running_) std::cerr << "Accept connection failed\n";
            continue;
        }

        handleClient(client_fd);
    }
}

void Server::handleClient(int client_fd) {
    char buffer[1024] = {0};
    
    // Read raw data sent by the client
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        std::cout << "Received from client: " << buffer << "\n";
        
        // Send a simple response back to the client
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from C++ Server!";
        write(client_fd, response.c_str(), response.length());
    }

    // Close client connection
    close(client_fd);
}

void Server::stop() {
    if (is_running_) {
        is_running_ = false;
        if (server_fd_ != -1) {
            close(server_fd_);
            server_fd_ = -1;
        }
        std::cout << "Server stopped.\n";
    }
}
