#pragma once

#include <string>
#include <ImageMagick-6/Magick++.h>

#include "config.h"

namespace imageresizer {

class DataManager {
public:
    DataManager();


    bool exists(const std::string &image, int32_t width, int32_t height) const;
    const std::string path(const std::string &image, int32_t width, int32_t height) const;

    const std::string
    get(const std::string &image, int32_t width, int32_t height, const Magick::Image &source) const;


private:
    bool exists(const std::string &image) const;

    void save(const std::string &image, int32_t width, int32_t height, const Magick::Image &source) const;


    int32_t _threadId;
};

} //namespace imageresizer
