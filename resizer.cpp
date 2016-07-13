#include <iostream>
#include <sstream>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ImageMagick-6/Magick++.h>

#include "resizer.h"
#include "message.h"
#include "datamanager.h"


namespace {
static thread_local imageresizer::DataManager _dataManager;
bool exists(const std::string &image) {
    struct stat buffer;   
    return (stat(image.c_str(), &buffer) == 0);
}
}

namespace imageresizer {

Resizer::Resizer(
        Net::Address addr,
        const std::string &sourceImageDir,
        const std::string &imageCacheDir)
    : _httpEndpoint(std::make_shared<Net::Http::Endpoint>(addr)),
    _sourceImageDir(sourceImageDir),
    _cachedImageDir(imageCacheDir),
    _router() {
}

void
Resizer::init(size_t threads) {
    auto opts = Net::Http::Endpoint::options()
        .threads(threads)
        .flags(Net::Tcp::Options::InstallSignalHandler
                | Net::Tcp::Options::ReuseAddr
                | Net::Tcp::Options::NoDelay);
    _httpEndpoint->init(opts);
    setupRoutes();
}

void
Resizer::start() {
    _httpEndpoint->setHandler(_router.handler());
    _httpEndpoint->serve();
}

void
Resizer::shutdown() {
    _httpEndpoint->shutdown();
}

void
Resizer::setupRoutes() {
    using namespace Net::Rest;

    Routes::Get(_router, "/images/:width/:height/:imageFileName", Routes::bind(&Resizer::handleResizeRequest, this));
    std::cout<<"Routes registered"<<std::endl;

}

void
Resizer::handleResizeRequest(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {
    auto imageFileName = request.param(":imageFileName").as<std::string>();
    auto width = request.param(":width").as<int32_t>();
    auto height = request.param(":height").as<int32_t>();
    if(!regex_match(imageFileName, std::regex("^[a-zA-Z0-9_-]*.(png|jpg|gif)$"))) {
        response.send(Net::Http::Code::Bad_Request, Message("Invalid request"), MIME( Application, Json ));
        return;
    }
    if(_dataManager.exists(imageFileName, width, height)) {
        Net::Http::serveFile(response,
                        _dataManager.path(imageFileName, width, height).c_str());
        return;
    }
    std::ostringstream sout;
    sout<<_sourceImageDir<<"/"<<imageFileName;
    if(!exists(sout.str())) {
        response.send(Net::Http::Code::Not_Found, Message("Not Found"), MIME( Application, Json ));
        return;
    }
    Magick::Image master(sout.str());
    auto resizedImage = _dataManager.get(imageFileName, width, height, master); 
    Net::Http::serveFile(response, resizedImage.c_str());
}

} //namespace imageresizer
