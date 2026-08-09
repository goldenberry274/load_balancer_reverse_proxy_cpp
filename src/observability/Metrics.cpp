#include "observability/Metrics.hpp"

Metrics::Metrics()
    : totalRequests_(0),
      failedRequests_(0),
      completedRequests_(0),
      activeConnections_(0),
      healthyBackendCount_(0),
      totalBackendCount_(0),
      totalBackendLatencyUs_(0),
      backendLatencySamples_(0),
      startTime_(std::chrono::steady_clock::now())
{
}

void Metrics::requestStarted()
{
    totalRequests_++;
    activeConnections_++;
}

void Metrics::requestFinished()
{
    completedRequests_++;
    activeConnections_--;
}

void Metrics::requestFailed()
{
    failedRequests_++;
}

void Metrics::recordBackendLatency(
    std::chrono::microseconds latency)
{
    totalBackendLatencyUs_ += latency.count();
    backendLatencySamples_++;
}

void Metrics::setHealthyBackendCount(std::size_t count)
{
    healthyBackendCount_ = count;
}

void Metrics::setTotalBackendCount(std::size_t count)
{
    totalBackendCount_ = count;
}

std::size_t Metrics::getTotalRequests() const
{
    return totalRequests_.load();
}

std::size_t Metrics::getFailedRequests() const
{
    return failedRequests_.load();
}

std::size_t Metrics::getCompletedRequests() const
{
    return completedRequests_.load();
}

std::size_t Metrics::getActiveConnections() const
{
    return activeConnections_.load();
}

std::size_t Metrics::getHealthyBackendCount() const
{
    return healthyBackendCount_.load();
}

std::size_t Metrics::getTotalBackendCount() const
{
    return totalBackendCount_.load();
}

double Metrics::getAverageBackendLatencyMs() const
{
    const std::size_t samples =
        backendLatencySamples_.load();

    if (samples == 0) {
        return 0.0;
    }

    const long long totalUs =
        totalBackendLatencyUs_.load();

    return static_cast<double>(totalUs)
        / static_cast<double>(samples)
        / 1000.0;
}

std::chrono::seconds Metrics::getUptime() const
{
    const auto now =
        std::chrono::steady_clock::now();

    return std::chrono::duration_cast<std::chrono::seconds>(
        now - startTime_
    );
}