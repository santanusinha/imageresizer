#pragma once

#include <string>

namespace imageresizer {

bool exists(const std::string &file);
bool isNewer(const std::string &lhs, const std::string &rhs);
bool ensurePath(const std::string &path, bool create);

} //namespace imageresizer
