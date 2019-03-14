#pragma once

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

namespace imageresizer {

class Resizer {
public:

    Resizer(
        Pistache::Address addr,
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
    handle(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

private:
    std::shared_ptr<Pistache::Http::Endpoint> _httpEndpoint;
    const std::string _sourceImageDir;
    const std::string _cachedImageDir;
    Pistache::Rest::Router _router;

};

} //namespace imageresizer
