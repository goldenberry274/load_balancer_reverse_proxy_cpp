#include "observability/Metrics.hpp"

Metrics::Metrics()
    : totalRequests_(0),
      failedRequests_(0),
      completedRequests_(0),
      activeConnections_(0),
      totalBackendLatencyUs_(0)
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

double Metrics::getAverageBackendLatencyMs() const
{
    std::size_t completed = completedRequests_.load();

    if (completed == 0) {
        return 0.0;
    }

    long long totalUs = totalBackendLatencyUs_.load();

    return static_cast<double>(totalUs) /
           completed /
           1000.0;
}