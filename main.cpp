#include <iostream>
#include <cstdlib>
#include <sstream>
#include <pistache/endpoint.h>
#include <yaml-cpp/yaml.h>
#include <ImageMagick-6/Magick++.h>

#include "config.h"
#include "resizer.h"

using namespace imageresizer;

int main(int argc, char *argv[]) {
    YAML::Node config;
    try {
        config = YAML::LoadFile("config.yaml");
    } catch(std::exception &e) {
        std::cout<<"Error reading config file: "<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }
    Config::instance().initialize(config);
    std::cout<<"Port: "<<Config::instance().getPort()<<std::endl;
    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << Config::instance().getThreads() << " threads" << std::endl;

    Net::Address addr(Net::Ipv4::any(), Net::Port(Config::instance().getPort()));
    Resizer stats(addr,
                Config::instance().getSourceImageDir(),
                Config::instance().getImageCacheDir());

    stats.init(Config::instance().getThreads());
    stats.start();

    stats.shutdown();
    return EXIT_SUCCESS;
}


