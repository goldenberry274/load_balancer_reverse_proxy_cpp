#include "doctest.h"

#include "config/config.hpp"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace {

class TemporaryConfigFile {
private:
    std::string filename_;

public:
    TemporaryConfigFile(
        const std::string& filename,
        const std::string& contents
    )
        : filename_(filename)
    {
        std::ofstream output(filename_);

        if (!output) {
            throw std::runtime_error(
                "Failed to create temporary config file"
            );
        }

        output << contents;
    }

    ~TemporaryConfigFile()
    {
        std::remove(filename_.c_str());
    }

    const std::string& filename() const
    {
        return filename_;
    }
};

} // namespace

TEST_CASE("Config loads listen port and health-check interval")
{
    TemporaryConfigFile file(
        "test_basic_config.yaml",
        R"(
            listen_port: 8080

            health_check:
            interval: 5

            backends:
                - host: 127.0.0.1
                port: 9001
        )"
    );

    Config config(file.filename());

    CHECK(config.listenPort == 8080);
    CHECK(config.healthCheckInterval == 5);
}

TEST_CASE("Config loads all backends")
{
    TemporaryConfigFile file(
        "test_backends_config.yaml",
        R"(
            listen_port: 8080

            health_check:
            interval: 3

            backends:
                - host: 127.0.0.1
                port: 9001

                - host: 127.0.0.1
                port: 9002

                - host: 192.168.1.10
                port: 9003
        )"
    );

    Config config(file.filename());

    REQUIRE(config.backends.size() == 3);

    CHECK(config.backends.at(0).host == "127.0.0.1");
    CHECK(config.backends.at(0).port == 9001);
    CHECK(config.backends.at(0).healthy);

    CHECK(config.backends.at(1).host == "127.0.0.1");
    CHECK(config.backends.at(1).port == 9002);
    CHECK(config.backends.at(1).healthy);

    CHECK(config.backends.at(2).host == "192.168.1.10");
    CHECK(config.backends.at(2).port == 9003);
    CHECK(config.backends.at(2).healthy);
}

TEST_CASE("Config rejects a missing file")
{
    CHECK_THROWS_AS(
        Config("this_file_does_not_exist.yaml"),
        YAML::BadFile
    );
}

TEST_CASE("Config rejects invalid integer values")
{
    TemporaryConfigFile file(
        "test_invalid_integer.yaml",
        R"(
            listen_port: not-a-number

            health_check:
                interval: 3

            backends:
                - host: 127.0.0.1
                port: 9001
        )"
    );

    CHECK_THROWS_AS(
        Config(file.filename()),
        YAML::TypedBadConversion<int>
    );
}

TEST_CASE("Config rejects an empty backend list")
{
    TemporaryConfigFile file(
        "test_empty_backends.yaml",
        R"(
            listen_port: 8080

            health_check:
            interval: 3

            backends: []
        )"
    );

    CHECK_THROWS_AS(
        Config(file.filename()),
        std::runtime_error
    );
}

TEST_CASE("Config rejects backend without a port")
{
    TemporaryConfigFile file(
        "test_missing_backend_port.yaml",
        R"(
        listen_port: 8080

        health_check:
        interval: 3

        backends:
            - host: 127.0.0.1
        )"
    );

    CHECK_THROWS(
        Config(file.filename())
    );
}