#include "doctest.h"
#include "proxy/HttpParser.hpp"

#include <stdexcept>
#include <string>


TEST_CASE("Parse basic GET request")
{
    const std::string raw =
        "GET /hello HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: curl\r\n"
        "\r\n";

    HttpRequest request = HttpParser::parse(raw);

    CHECK(request.method == "GET");
    CHECK(request.path == "/hello");
    CHECK(request.version == "HTTP/1.1");

    CHECK(request.headers.at("Host") == "localhost:8080");
    CHECK(request.headers.at("User-Agent") == "curl");

    CHECK(request.body.empty());
}


TEST_CASE("Parse POST request with body")
{
    const std::string raw =
        "POST /login HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 18\r\n"
        "\r\n"
        "{\"username\":\"bob\"}";

    HttpRequest request = HttpParser::parse(raw);

    CHECK(request.method == "POST");
    CHECK(request.path == "/login");
    CHECK(request.version == "HTTP/1.1");

    CHECK(request.headers.at("Content-Type") == "application/json");
    CHECK(request.headers.at("Content-Length") == "18");

    CHECK(request.body == "{\"username\":\"bob\"}");
}


TEST_CASE("Parse HTTP 1.0 request")
{
    const std::string raw =
        "GET /legacy HTTP/1.0\r\n"
        "Host: localhost\r\n"
        "\r\n";

    HttpRequest request = HttpParser::parse(raw);

    CHECK(request.method == "GET");
    CHECK(request.path == "/legacy");
    CHECK(request.version == "HTTP/1.0");
}


TEST_CASE("Parse HEAD request")
{
    const std::string raw =
        "HEAD /status HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    HttpRequest request = HttpParser::parse(raw);

    CHECK(request.method == "HEAD");
    CHECK(request.path == "/status");
    CHECK(request.version == "HTTP/1.1");
}


TEST_CASE("Trim leading spaces from header values")
{
    const std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host:     localhost:8080\r\n"
        "\r\n";

    HttpRequest request = HttpParser::parse(raw);

    CHECK(request.headers.at("Host") == "localhost:8080");
}


TEST_CASE("Reject request without header terminator")
{
    const std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject malformed request line")
{
    const std::string raw =
        "GET\r\n"
        "Host: localhost\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject request line with extra data")
{
    const std::string raw =
        "GET / HTTP/1.1 EXTRA\r\n"
        "Host: localhost\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject unsupported HTTP version")
{
    const std::string raw =
        "GET / HTTP/2.0\r\n"
        "Host: localhost\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject unsupported HTTP method")
{
    const std::string raw =
        "DELETE /resource HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject path without leading slash")
{
    const std::string raw =
        "GET hello HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject malformed header")
{
    const std::string raw =
        "GET / HTTP/1.1\r\n"
        "This-is-not-a-valid-header\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject header with empty name")
{
    const std::string raw =
        "GET / HTTP/1.1\r\n"
        ": some-value\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}


TEST_CASE("Reject obviously invalid request")
{
    const std::string raw =
        "THIS IS NOT HTTP\r\n"
        "\r\n";

    CHECK_THROWS_AS(
        HttpParser::parse(raw),
        std::runtime_error
    );
}