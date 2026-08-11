#pragma once

#include <string>

struct Backend {
    std::string host;
    int port;
    bool healthy = false;
    std::size_t requestsServed = 0;
};