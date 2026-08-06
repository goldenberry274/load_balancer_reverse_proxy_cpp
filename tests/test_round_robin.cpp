#include "doctest.h"

#include "balancer/Backend.hpp"
#include "balancer/RoundRobinBalancer.hpp"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

TEST_CASE("Round robin cycles through healthy backends")
{
    auto backends = std::make_shared<std::vector<Backend>>(
        std::initializer_list<Backend>{
            {"127.0.0.1", 9001, true, 0},
            {"127.0.0.1", 9002, true, 0},
            {"127.0.0.1", 9003, true, 0}
        }
    );

    std::mutex mutex;
    RoundRobinBalancer balancer(backends, mutex);

    CHECK(balancer.getNextBackend().port == 9001);
    CHECK(balancer.getNextBackend().port == 9002);
    CHECK(balancer.getNextBackend().port == 9003);
    CHECK(balancer.getNextBackend().port == 9001);
}

TEST_CASE("Round robin skips unhealthy backends")
{
    auto backends = std::make_shared<std::vector<Backend>>(
        std::initializer_list<Backend>{
            {"127.0.0.1", 9001, true, 0},
            {"127.0.0.1", 9002, false, 0},
            {"127.0.0.1", 9003, true, 0}
        }
    );

    std::mutex mutex;
    RoundRobinBalancer balancer(backends, mutex);

    CHECK(balancer.getNextBackend().port == 9001);
    CHECK(balancer.getNextBackend().port == 9003);
    CHECK(balancer.getNextBackend().port == 9001);
    CHECK(balancer.getNextBackend().port == 9003);
}

TEST_CASE("Round robin throws when all backends are unhealthy")
{
    auto backends = std::make_shared<std::vector<Backend>>(
        std::initializer_list<Backend>{
            {"127.0.0.1", 9001, false, 0},
            {"127.0.0.1", 9002, false, 0}
        }
    );

    std::mutex mutex;
    RoundRobinBalancer balancer(backends, mutex);

    CHECK_THROWS_AS(
        balancer.getNextBackend(),
        std::runtime_error
    );
}

TEST_CASE("Round robin increments request counters")
{
    auto backends = std::make_shared<std::vector<Backend>>(
        std::initializer_list<Backend>{
            {"127.0.0.1", 9001, true, 0},
            {"127.0.0.1", 9002, true, 0}
        }
    );

    std::mutex mutex;
    RoundRobinBalancer balancer(backends, mutex);

    balancer.getNextBackend();
    balancer.getNextBackend();
    balancer.getNextBackend();

    std::lock_guard<std::mutex> lock(mutex);

    CHECK(backends->at(0).requestsServed == 2);
    CHECK(backends->at(1).requestsServed == 1);
}

TEST_CASE("Round robin constructor rejects null backend pointer")
{
    std::shared_ptr<std::vector<Backend>> backends;
    std::mutex mutex;

    CHECK_THROWS_AS(
        RoundRobinBalancer(backends, mutex),
        std::invalid_argument
    );
}

TEST_CASE("Round robin constructor rejects empty backend list")
{
    auto backends =
        std::make_shared<std::vector<Backend>>();

    std::mutex mutex;

    CHECK_THROWS_AS(
        RoundRobinBalancer(backends, mutex),
        std::invalid_argument
    );
}