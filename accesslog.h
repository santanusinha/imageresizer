#pragma once
#include <chrono>
#include <http.h>
#include <router.h>

namespace imageresizer {

class AccessLog {
public:
    AccessLog(const Net::Rest::Request& request, const Net::Http::ResponseWriter &response);
    ~AccessLog();
private:
    const Net::Rest::Request& request;
    const Net::Http::ResponseWriter &response;
    std::chrono::steady_clock::time_point startTime;
};

} //namespace imageresizer
