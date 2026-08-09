#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>

class Metrics {
public:
    Metrics();

    // Request lifecycle
    void requestStarted();
    void requestFinished();
    void requestFailed();

    // Backend latency
    void recordBackendLatency(
        std::chrono::microseconds latency
    );

    // Getters
    std::size_t getTotalRequests() const;
    std::size_t getFailedRequests() const;
    std::size_t getActiveConnections() const;

    std::size_t getCompletedRequests() const;

    double getAverageBackendLatencyMs() const;

private:
    std::atomic<std::size_t> totalRequests_;
    std::atomic<std::size_t> failedRequests_;
    std::atomic<std::size_t> completedRequests_;
    std::atomic<std::size_t> activeConnections_;

    // Stored in microseconds
    std::atomic<long long> totalBackendLatencyUs_;
};