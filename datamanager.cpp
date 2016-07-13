#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "datamanager.h"
#include "threadcounter.h"

namespace imageresizer {

DataManager::DataManager()
    :_threadId(ThreadManager::threadId()) {
    std::cout<<"Started data manager for thread: "<<_threadId<<std::endl;
    Magick::InitializeMagick(NULL);
}

bool
DataManager::exists(const std::string &image, int32_t width, int32_t height) const {
    const std::string &imagePath = path(image, width, height);
    return exists(imagePath);
}


const std::string
DataManager::get(const std::string &image, int32_t width, int32_t height, const Magick::Image &source) const{
    const std::string &imagePath = path(image, width, height);
    std::cout<<"Path: "<<imagePath<<std::endl;
    if(!exists(imagePath)) {
        std::cout<<"Path doesn not exist: "<<imagePath<<std::endl;
        save(imagePath, width, height, source);
        std::cout<<"Image created: "<<imagePath<<std::endl;
    }
    return imagePath;
}

bool
DataManager::exists(const std::string &image) const{
    struct stat buffer;   
    return (stat(image.c_str(), &buffer) == 0);
}

void
DataManager::save(
                const std::string &image,
                int32_t width,
                int32_t height,
                const Magick::Image &source) const{
    Magick::Image dest(source);
    std::ostringstream dimensions;
    dimensions<<width<<"x"<<height;
    dest.resize(dimensions.str());
    dest.write(image);
}

const std::string
DataManager::path(const std::string &image, int32_t width, int32_t height) const {
    std::ostringstream sout;
    sout<<Config::instance().getImageCacheDir()<<"/"<<_threadId<<"-"<<width<<"x"<<height<<"-"<<image;
    return sout.str();
}

} //namespace imageresizer
