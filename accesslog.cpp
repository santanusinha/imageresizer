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

const std::unordered_map<Net::Http::Method, std::string, EnumClassHash> REQ_METHODS = {
      { Net::Http::Method::Options, "OPTIONS"}
    , { Net::Http::Method::Get, "GET"}
    , { Net::Http::Method::Post, "POST"}
    , { Net::Http::Method::Head, "HEAD"}
    , { Net::Http::Method::Put, "PUT"}
    , { Net::Http::Method::Delete, "DELETE"}
    , { Net::Http::Method::Trace, "TRACE"}
    , { Net::Http::Method::Connect, "CONNECT"}
};

const std::unordered_map<Net::Http::Version, std::string, EnumClassHash> REQ_VERSIONS = {
      { Net::Http::Version::Http10, "HTTP/1.0"}
    , { Net::Http::Version::Http11, "HTTP/1.1"}
};

const std::string &
resolve(const Net::Http::Method &method) {
    return (*REQ_METHODS.find(method)).second;
}

const std::string &
resolve(const Net::Http::Version &version) {
    return (*REQ_VERSIONS.find(version)).second;
}

}

namespace imageresizer {
AccessLog::AccessLog(const Net::Rest::Request& request, const Net::Http::ResponseWriter &response)
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

