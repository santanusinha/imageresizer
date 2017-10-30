#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

namespace imageresizer {

bool
exists(const std::string &file) {
    struct stat buffer;
    return (stat(file.c_str(), &buffer) == 0);
}

bool
isNewer(const std::string &lhs, const std::string &rhs) {
    struct stat lhsStat, rhsStat;
    if(0 != stat(lhs.c_str(), &lhsStat) || 0 != stat(rhs.c_str(), &rhsStat)) {
        return false;
    }
    //std::cout<<"L: "<<lhsStat.st_mtime<<" R: "<<rhsStat.st_mtime<<std::endl;
    return lhsStat.st_mtime > rhsStat.st_mtime; 
}

bool ensurePath(const std::string &path, bool create) {
    struct stat pathStat;
    if(0 != stat(path.c_str(), &pathStat)) {
        std::cerr<<"Could not get info for: "<<path<<std::endl;
        return false;
    }
    if(!S_ISDIR(pathStat.st_mode)) {
        std::cerr<<"Given path is not a directory: "<<path<<std::endl;
        return false;
    }
    if( 0 != access(path.c_str(), R_OK | X_OK)) {
        std::cerr<<"Path is not readable: "<<path<<std::endl;
        return false;
    }
    return true;
}

} //namespace imageresizer
