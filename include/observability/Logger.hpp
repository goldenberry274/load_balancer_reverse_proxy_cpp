#pragma once

#include <mutex>
#include <string>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger {
public:
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void debug(const std::string& message);

private:
    static std::mutex mutex_;

    static void log(LogLevel level, const std::string& message);
    static std::string levelToString(LogLevel level);
};