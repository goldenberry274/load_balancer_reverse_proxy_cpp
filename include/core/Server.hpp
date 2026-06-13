#pragma once

#include <string>
#include <vector>

class Server {
public:
    // Constructor to initialize server port
    Server(int port);
    
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

    // Internal helper to handle incoming client data
    void handleClient(int client_fd);
};

