#include <string>
#include <unordered_map>
#include "accesslog.h"
#include "logger.h"

namespace {

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

const std::unordered_map<Pistache::Http::Method, std::string, EnumClassHash> REQ_METHODS = {
      { Pistache::Http::Method::Options, "OPTIONS"}
    , { Pistache::Http::Method::Get, "GET"}
    , { Pistache::Http::Method::Post, "POST"}
    , { Pistache::Http::Method::Head, "HEAD"}
    , { Pistache::Http::Method::Put, "PUT"}
    , { Pistache::Http::Method::Delete, "DELETE"}
    , { Pistache::Http::Method::Trace, "TRACE"}
    , { Pistache::Http::Method::Connect, "CONNECT"}
};

const std::unordered_map<Pistache::Http::Version, std::string, EnumClassHash> REQ_VERSIONS = {
      { Pistache::Http::Version::Http10, "HTTP/1.0"}
    , { Pistache::Http::Version::Http11, "HTTP/1.1"}
};

const std::string &
resolve(const Pistache::Http::Method &method) {
    return (*REQ_METHODS.find(method)).second;
}

const std::string &
resolve(const Pistache::Http::Version &version) {
    return (*REQ_VERSIONS.find(version)).second;
}

}

namespace imageresizer {
AccessLog::AccessLog(const Pistache::Rest::Request& request, const Pistache::Http::ResponseWriter &response)
    :request(request),
    response(response),
    startTime(std::chrono::steady_clock::now()) {
}

AccessLog::~AccessLog() {
    INFO("{} {} {} {} {} {}",
            resolve(request.method()),
            request.resource(), 
            resolve(request.version()),
            static_cast<int>(response.code()),
            response.body().size(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count());
}
} //namespace imageresizer

