#pragma once

#include <string>

struct Backend{
    std::string host;
    int port;
    bool healthy = true;
};