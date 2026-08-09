#pragma once

#include "HttpRequest.hpp"

#include <string>

class HttpParser {
public:
    static HttpRequest parse(const std::string& rawRequest);
};