#include "health/HealthChecker.hpp"

#include <chrono>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

HealthChecker::HealthChecker(
    std::shared_ptr<std::vector<Backend>> backends,
    std::mutex& backends_mutex,
    int interval_seconds
)
    : backends_(std::move(backends)),
      backends_mutex_(backends_mutex),
      running_(false),
      interval_seconds_(interval_seconds)
{
}

HealthChecker::~HealthChecker()
{
    stop();
}

void HealthChecker::start()
{
    if (running_) {
        return;
    }

    running_ = true;
    worker_ = std::thread(&HealthChecker::loop, this);
}

void HealthChecker::stop()
{
    running_ = false;

    if (worker_.joinable()) {
        worker_.join();
    }
}

void HealthChecker::loop()
{
    while (running_) {
        {
            std::lock_guard<std::mutex> lock(backends_mutex_);

            for (Backend& backend : *backends_) {
                bool was_healthy = backend.healthy;
                bool is_healthy = checkBackend(backend);

                backend.healthy = is_healthy;

                if (was_healthy != is_healthy) {
                    std::cout
                        << "[HealthChecker] Backend "
                        << backend.host << ":" << backend.port
                        << " is now "
                        << (is_healthy ? "healthy" : "unhealthy")
                        << "\n";
                    
                }
                
            }
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(interval_seconds_)
        );
    }
}

bool HealthChecker::checkBackend(const Backend& backend)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(backend.port);

    if (inet_pton(AF_INET, backend.host.c_str(), &address.sin_addr) <= 0) {
        close(fd);
        return false;
    }

    bool success =
        connect(fd, (sockaddr*)&address, sizeof(address)) == 0;

    close(fd);
    return success;
}