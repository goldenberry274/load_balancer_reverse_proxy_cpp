#pragma once

#include "Backend.hpp"

class LoadBalancer{

    public:
        virtual Backend getNextBackend() = 0;
        virtual ~LoadBalancer() = default;
};