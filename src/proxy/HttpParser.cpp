#include "proxy/HttpParser.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

HttpRequest HttpParser::parse(const std::string& rawRequest)
{
    HttpRequest request;

    const std::size_t headerEnd = rawRequest.find("\r\n\r\n");

    if (headerEnd == std::string::npos) {
        throw std::runtime_error(
            "Malformed HTTP request: missing header terminator"
        );
    }

    const std::string headerSection =
        rawRequest.substr(0, headerEnd);

    request.body =
        rawRequest.substr(headerEnd + 4);

    std::istringstream stream(headerSection);

    std::string requestLine;

    if (!std::getline(stream, requestLine)) {
        throw std::runtime_error(
            "Malformed HTTP request: missing request line"
        );
    }

    // getline removes '\n' but may leave '\r'
    if (!requestLine.empty() &&
        requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestLineStream(requestLine);

    if (!(requestLineStream
          >> request.method
          >> request.path
          >> request.version)) {
        throw std::runtime_error(
            "Malformed HTTP request line"
        );
    }

    if (request.method.empty() ||
        request.path.empty() ||
        request.version.empty()) {
        throw std::runtime_error(
            "Malformed HTTP request line"
        );
    }

    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        const std::size_t colon = line.find(':');

        if (colon == std::string::npos) {
            throw std::runtime_error(
                "Malformed HTTP header: " + line
            );
        }

        std::string key =
            line.substr(0, colon);

        std::string value =
            line.substr(colon + 1);

        // Remove leading spaces from header value.
        const std::size_t firstNonSpace =
            value.find_first_not_of(" \t");

        if (firstNonSpace != std::string::npos) {
            value.erase(0, firstNonSpace);
        } else {
            value.clear();
        }

        if (key.empty()) {
            throw std::runtime_error(
                "Malformed HTTP header: empty name"
            );
        }

        request.headers[key] = value;
    }

    return request;
}