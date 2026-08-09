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

    // Backend health summary
    void setHealthyBackendCount(std::size_t count);
    void setTotalBackendCount(std::size_t count);

    // Getters
    std::size_t getTotalRequests() const;
    std::size_t getFailedRequests() const;
    std::size_t getCompletedRequests() const;
    std::size_t getActiveConnections() const;

    std::size_t getHealthyBackendCount() const;
    std::size_t getTotalBackendCount() const;

    double getAverageBackendLatencyMs() const;

    std::chrono::seconds getUptime() const;

private:
    std::atomic<std::size_t> totalRequests_;
    std::atomic<std::size_t> failedRequests_;
    std::atomic<std::size_t> completedRequests_;
    std::atomic<std::size_t> activeConnections_;

    std::atomic<std::size_t> healthyBackendCount_;
    std::atomic<std::size_t> totalBackendCount_;

    // Stored in microseconds
    std::atomic<long long> totalBackendLatencyUs_;
    std::atomic<std::size_t> backendLatencySamples_;

    std::chrono::steady_clock::time_point startTime_;
};