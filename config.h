#pragma once

#include <cstdint>
#include <yaml-cpp/yaml.h>
#include <boost/program_options.hpp>

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

        int32_t getCacheTimeSeconds() const {
            return cacheTimeSeconds;
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

        bool initialize(int argc, char *argv[]);

    private:
        
        int32_t port;
        int32_t threads;
        int32_t cacheTimeSeconds;
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
