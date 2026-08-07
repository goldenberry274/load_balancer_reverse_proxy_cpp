#include "core/ThreadPool.hpp"

#include <stdexcept>
#include <utility>

ThreadPool::ThreadPool(std::size_t threadCount)
    : stopping_(false)
{
    if (threadCount == 0) {
        throw std::invalid_argument(
            "ThreadPool requires at least one worker thread"
        );
    }

    workers_.reserve(threadCount);

    for (std::size_t i = 0; i < threadCount; ++i) {
        workers_.emplace_back(
            [this]()
            {
                workerLoop();
            }
        );
    }
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stopping_) {
            throw std::runtime_error(
                "Cannot enqueue task into a stopped ThreadPool"
            );
        }

        tasks_.push(std::move(task));
    }

    condition_.notify_one();
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stopping_) {
            return;
        }

        stopping_ = true;
    }

    condition_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    workers_.clear();
}

void ThreadPool::workerLoop()
{
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            condition_.wait(
                lock,
                [this]()
                {
                    return stopping_ || !tasks_.empty();
                }
            );

            if (stopping_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        try {
            task();
        }
        catch (const std::exception& e) {
            Logger::error(
                "ThreadPool task threw exception: " +
                std::string(e.what())
            );
        }
        catch (...) {
            Logger::error(
                "ThreadPool task threw unknown exception"
            );
        }
    }
}