#include "config/config.hpp"
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <stdexcept>


Config::Config(const std::string& filename){
    try{
        YAML::Node config = YAML::LoadFile(filename);
        if (config["listen_port"]){
            listenPort = config["listen_port"].as<int>();
        }
        if (config["health_check"]){
            healthCheckInterval = config["health_check"]["interval"].as<int>();
        }
        if (config["backends"] && config["backends"].IsSequence())
        {
            for (const auto& node : config["backends"])
            {
                Backend backend;

                backend.host =
                    node["host"].as<std::string>();

                backend.port =
                    node["port"].as<int>();

                backend.healthy = true;

                backends.push_back(backend);
            }
        }

        if (backends.empty())
        {
            throw std::runtime_error(
                "Configuration contains no backends.");
        }

    }catch (const YAML::BadFile& e) {
        std::cerr << "Error: Could not find or open config.yaml\n";
        throw;
    } catch (const YAML::Exception& e) {
        std::cerr << "Parser error: " << e.what() << "\n";
        throw;
    }

}