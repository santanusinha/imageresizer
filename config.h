#pragma once

#include <cstdint>
#include <yaml-cpp/yaml.h>

namespace imageresizer {

class Config {
    public:
        static Config &instance() {
            static Config config;
            return config;
        }

        int32_t getPort() const {
            return port;
        }

        int32_t getThreads() const {
            return threads;
        }

        const std::string &getImageDir() const {
            return imageDir;
        }

        const std::string &getSourceImageDir() const {
            return sourceImageDir;
        }

        const std::string &getImageCacheDir() const {
            return imageCacheDir;
        }

        void initialize(const YAML::Node &config) {
            port = config["port"].as<int32_t>();
            threads = config["threads"].as<int32_t>();
            imageDir = config["imageDir"].as<std::string>();
            sourceImageDir = config["sourceImageDir"].as<std::string>();
            imageCacheDir = config["imageCacheDir"].as<std::string>();
        }

    private:
        
        int32_t port;
        int32_t threads;
        std::string imageDir;
        std::string sourceImageDir;
        std::string imageCacheDir;

        Config()
            :port(0),
            threads(0),
            imageDir() {
        }

        Config(const Config &) = delete;
        Config& operator=(const Config &) = delete;
        
};
} //namespace imageresizer
