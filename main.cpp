#include <cstdlib>
#include <sstream>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/next_prior.hpp>
#include <ImageMagick-6/Magick++.h>
#include <pistache/endpoint.h>
#include <yaml-cpp/yaml.h>

#include "config.h"
#include "logger.h"
#include "resizer.h"
#include "util.h"

using namespace imageresizer;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    spdlog::stdout_logger_mt("console");
    Config::instance().initialize(argc, argv);
    if(!ensurePath(Config::instance().getSourceImageDir(), false)) {
        return EXIT_FAILURE;
    }
    INFO("Port: {}", Config::instance().getPort());
    INFO("Cores: {} ", std::thread::hardware_concurrency());
    INFO("Threads: {}", Config::instance().getThreads());

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(Config::instance().getPort()));
    Resizer stats(addr,
                Config::instance().getSourceImageDir(),
                Config::instance().getImageCacheDir());

    stats.init(Config::instance().getThreads());
    stats.start();

    stats.shutdown();
    return EXIT_SUCCESS;
}


