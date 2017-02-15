#include <cstdlib>
#include <sstream>
#include <pistache/endpoint.h>
#include <yaml-cpp/yaml.h>
#include <ImageMagick-6/Magick++.h>
#include <boost/program_options.hpp>

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
    INFO("Cores: {} ", hardware_concurrency());
    INFO("Threads: {}", Config::instance().getThreads());

    Net::Address addr(Net::Ipv4::any(), Net::Port(Config::instance().getPort()));
    Resizer stats(addr,
                Config::instance().getSourceImageDir(),
                Config::instance().getImageCacheDir());

    stats.init(Config::instance().getThreads());
    stats.start();

    stats.shutdown();
    return EXIT_SUCCESS;
}


