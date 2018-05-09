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

//std::string
//path(const std::string &dir, const std::string &file) {
//    std::ostringstream sout;
//    sout<<dir<<"/"<<file;
//    return sout.str();
//}

std::vector<std::string>
splitUrl(const std::string &url) {
    std::vector<std::string> fragments;

    std::istringstream iss(url);
    std::string p;

    while (std::getline(iss, p, '/')) {
        if (p.empty()) continue;

        fragments.push_back(std::move(p));
    }

    return fragments;
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

    Routes::Get(_router, "/.*", Routes::bind(&Resizer::handle, this));
    //Routes::Get(_router, "/images/*/:width/:height/:imageFileName", Routes::bind(&Resizer::handleResizeRequest, this));
    //Routes::Get(_router, "/healthcheck", Routes::bind(&Resizer::handleHealthcheckRequest, this));
//    Routes::Post(_router, "/images/:imageFileName", Routes::bind(&Resizer::handleImageSave, this));
    std::cout<<"Routes registered"<<std::endl;

}

void
Resizer::handle(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {
    AccessLog l(request, response);
#ifdef DEBUG
    std::cout<<"Resource: "<<request.resource()<<std::endl;
#endif
    auto requestPath = request.resource();
    if( requestPath == "/healthcheck" ) {
        response.send(Net::Http::Code::Ok, Message("Service is healthy"), MIME( Application, Json ));
        return;
    }
    auto urlParts = splitUrl(requestPath);
    auto urlPartsSize = urlParts.size();
    if(urlParts.empty() || urlParts.size() < 5 || urlParts[0] != "images") {
        response.send(Net::Http::Code::Bad_Request, Message("Route missing"), MIME( Application, Json ));
        return;
    }
    auto imageFileNamePart = urlParts[urlPartsSize - 1];
    int32_t width = 0;
    int32_t height = 0;
    try {
        height = std::stoi(urlParts[urlPartsSize - 2]);
        width = std::stoi(urlParts[urlPartsSize - 3]);
    } catch(std::exception &e) {
        response.send(Net::Http::Code::Bad_Request, Message("Invalid request"), MIME( Application, Json ));
        return;
    }
    if(!regex_match(imageFileNamePart, std::regex("^[a-zA-Z0-9_-]*.(png|jpg|gif)$"))) {
        response.send(Net::Http::Code::Bad_Request, Message("Invalid request"), MIME( Application, Json ));
        return;
    }
    if(width <= 0 || height <= 0) {
        response.send(Net::Http::Code::Bad_Request,
                        Message("Invalid image dimensions specified"), MIME( Application, Json ));
        return;
    }
    std::string imageFilePath = _sourceImageDir + "/";
    for(size_t i = 1; i < urlPartsSize - 3; i++ ) {
        imageFilePath += urlParts[i] + "/";
    }
    auto imageFileName = imageFilePath + imageFileNamePart;
#ifdef DEBUG
    std::cout<< "Image: " << imageFileNamePart << std::endl
            << " Path: " << imageFilePath << std::endl
            << " File: " << imageFileName << std::endl
            << " Width: " << width << std::endl
            << " Height: " << height << std::endl
            << std::endl;
#endif
    if(!exists(imageFileName)) {
        response.send(Net::Http::Code::Not_Found,
                        Message("Not Found"), MIME( Application, Json ));
        return;
    }
#ifdef DEBUG
    std::cout<<"File Exists .. checking cache: "<<imageFileName<<std::endl;
#endif
    response.headers().add(
                        std::make_shared<Net::Http::Header::CacheControl>(
                            Net::Http::CacheDirective(
                                Net::Http::CacheDirective::Directive::MaxAge,
                                std::chrono::seconds(Config::instance().getCacheTimeSeconds()))));
    if(_dataManager.exists(imageFileName, width, height)) {
#ifdef DEBUG
        std::cout<<"Exists in cache"<<std::endl;
#endif
        const std::string &existingPath = _dataManager.path(imageFileName, width, height);
        if(!isNewer(imageFilePath, existingPath)) {
            std::cout<<"Sending cached image "<< imageFileNamePart << std::endl;
            Net::Http::serveFile(response, existingPath.c_str());
            return;
        }
#ifdef DEBUG
        std::cout<<"Source file "<<imageFilePath<<" is newer than cache"<<std::endl;
#endif
    }
    std::cout<<"Sending generated image "<< imageFileNamePart << std::endl;
    Magick::Image master(imageFileName);
    try {
        auto resizedImage = _dataManager.get(imageFileName, width, height, master);
        Net::Http::serveFile(response, resizedImage.c_str());
    } catch(...) {
        response.send(Net::Http::Code::Internal_Server_Error,
                        Message("Could not process the image"), MIME( Application, Json ));
        return;
    }
}

} //namespace imageresizer
