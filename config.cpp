#include <iostream>
#include <boost/program_options.hpp>
#include <yaml-cpp/yaml.h>
#include "config.h"

namespace po = boost::program_options;

namespace imageresizer {

bool
Config::initialize(int argc, char *argv[]) {

    po::options_description desc("Allowed options");

    desc.add_options()
        ("help", "produce help message")
        ("config,f", po::value<std::string>(), "set config file name")
        ("threads,t", po::value<int>(), "set thread count")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc <<std::endl;
        return false;
    }

    if(!vm.count("config")) {
        std::cerr<<"[--config/-f <config>] must be provided"<<std::endl;
        return false;
    }

    YAML::Node config;
    const char *configPath = vm["config"].as<std::string>().c_str();
    std::cout<<"Reading config: "<< configPath;
    std::cout <<std::endl;
    try {
        config = YAML::LoadFile(configPath);
    } catch(std::exception &e) {
        std::cout<<"Error reading config file: "<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("threads")) {
        threads = vm["threads"].as<int32_t>();
    }
    else {
        threads = config["threads"].as<int32_t>();
    }

    port = config["port"].as<int32_t>();
    cacheTimeSeconds = config["cacheTimeSeconds"].as<int32_t>();
    imageDir = config["imageDir"].as<std::string>();
    sourceImageDir = config["sourceImageDir"].as<std::string>();
    imageCacheDir = config["imageCacheDir"].as<std::string>();
    return true;
}


} //namespace imageresizer
