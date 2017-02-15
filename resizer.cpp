#include <iostream>
#include <sstream>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ImageMagick-6/Magick++.h>

#include "resizer.h"
#include "logger.h"
#include "accesslog.h"
#include "message.h"
#include "datamanager.h"
#include "util.h"


namespace {
static thread_local imageresizer::DataManager _dataManager;

std::string
path(const std::string &dir, const std::string &file) {
    std::ostringstream sout;
    sout<<dir<<"/"<<file;
    return sout.str();
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

    Routes::Get(_router, "/images/:dir/:width/:height/:imageFileName", Routes::bind(&Resizer::handleResizeRequest, this));
    Routes::Get(_router, "/healthcheck", Routes::bind(&Resizer::handleHealthcheckRequest, this));
//    Routes::Post(_router, "/images/:imageFileName", Routes::bind(&Resizer::handleImageSave, this));
    std::cout<<"Routes registered"<<std::endl;

}

void
Resizer::handleResizeRequest(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {
    AccessLog l(request, response);
    auto imageFileDir = request.param(":dir").as<std::string>();
    auto imageFileNamePart = request.param(":imageFileName").as<std::string>();
    auto width = request.param(":width").as<int32_t>();
    auto height = request.param(":height").as<int32_t>();
    if(!regex_match(imageFileNamePart, std::regex("^[a-zA-Z0-9_-]*.(png|jpg|gif)$"))) {
        response.send(Net::Http::Code::Bad_Request, Message("Invalid request"), MIME( Application, Json ));
        return;
    }
    auto imageFileName = path(imageFileDir, imageFileNamePart);

    std::ostringstream sout;
    sout<<_sourceImageDir<<"/"<<imageFileName;

    auto imageFilePath = sout.str();
    if(!exists(imageFilePath)) {
        response.send(Net::Http::Code::Not_Found,
                        Message("Not Found"), MIME( Application, Json ));
        return;
    }
    if(_dataManager.exists(imageFileName, width, height)) {
        const std::string &existingPath = _dataManager.path(imageFileName, width, height);
        if(!isNewer(imageFilePath, existingPath)) {
            //std::cout<<"Sending cached image"<<std::endl;
            Net::Http::serveFile(response, existingPath.c_str());
            return;
        }
        std::cout<<"Source file "<<imageFilePath<<" is newer than cache"<<std::endl;
    }
    Magick::Image master(sout.str());
    auto resizedImage = _dataManager.get(imageFileName, width, height, master);
    response.headers().add(std::make_shared<Net::Http::Header::CacheControl>(Net::Http::CacheDirective(Net::Http::CacheDirective::Directive::MaxAge, std::chrono::seconds(Config::instance().getCacheTimeSeconds()))));
    Net::Http::serveFile(response, resizedImage.c_str());
}

void
Resizer::handleHealthcheckRequest(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {
    AccessLog l(request, response);
    response.send(Net::Http::Code::Ok, Message("Service is healthy"), MIME( Application, Json ));
}

} //namespace imageresizer
