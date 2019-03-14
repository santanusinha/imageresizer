#pragma once
#include <chrono>
#include <pistache/http.h>
#include <pistache/router.h>

namespace imageresizer {

class AccessLog {
public:
    AccessLog(const Pistache::Rest::Request& request, const Pistache::Http::ResponseWriter &response);
    ~AccessLog();
private:
    const Pistache::Rest::Request& request;
    const Pistache::Http::ResponseWriter &response;
    std::chrono::steady_clock::time_point startTime;
};

} //namespace imageresizer
