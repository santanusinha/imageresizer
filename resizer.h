#pragma once

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

namespace imageresizer {

class Resizer {
public:

    Resizer(
        Net::Address addr,
        const std::string &sourceImageDir,
        const std::string &imageCacheDir);

    void
    init(size_t threads);

    void
    start();

    void
    shutdown();

    void
    setupRoutes();

    void
    handleResizeRequest(
                const Net::Rest::Request& request,
                Net::Http::ResponseWriter response);

    void
    handleHealthcheckRequest(
                const Net::Rest::Request& request,
                Net::Http::ResponseWriter response);
private:
    std::shared_ptr<Net::Http::Endpoint> _httpEndpoint;
    const std::string _sourceImageDir;
    const std::string _cachedImageDir;
    Net::Rest::Router _router;
    
};

} //namespace imageresizer
