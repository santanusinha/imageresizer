/* http_server.cc
   Mathieu Stefani, 07 février 2016
   
   Example of an http server
*/

#include "net.h"
#include "http.h"
#include "peer.h"
#include "http_headers.h"
#include "cookie.h"
#include "endpoint.h"

using namespace std;
using namespace Net;

struct PrintException {
    void operator()(std::exception_ptr exc) const {
        try {
            std::rethrow_exception(exc);
        } catch (const std::exception& e) {
            std::cerr << "An exception occured: " << e.what() << std::endl;
        }
    }
};

struct LoadMonitor {
    LoadMonitor(const std::shared_ptr<Net::Http::Endpoint>& endpoint)
        : endpoint_(endpoint)
        , interval(std::chrono::seconds(1))
    { }

    void setInterval(std::chrono::seconds secs) {
        interval = secs;
    }

    void start() {
        shutdown_ = false;
        thread.reset(new std::thread(std::bind(&LoadMonitor::run, this)));
    }

    void shutdown() {
        shutdown_ = true;
    }

    ~LoadMonitor() {
        shutdown_ = true;
        if (thread) thread->join();
    }

private:
    std::shared_ptr<Net::Http::Endpoint> endpoint_;
    std::unique_ptr<std::thread> thread;
    std::chrono::seconds interval;

    std::atomic<bool> shutdown_;

    void run() {
        Net::Tcp::Listener::Load old;
        while (!shutdown_) {
            if (!endpoint_->isBound()) continue;

            endpoint_->requestLoad(old).then([&](const Net::Tcp::Listener::Load& load) {
                old = load;

                double global = load.global;
                if (global > 100) global = 100;

                if (global > 1)
                    std::cout << "Global load is " << global << "%" << std::endl;
                else
                    std::cout << "Global load is 0%" << std::endl;
            },
            Async::NoExcept);

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
};


class MyHandler : public Net::Http::Handler {

    HTTP_PROTOTYPE(MyHandler)

    void onRequest(
            const Net::Http::Request& req,
            Net::Http::ResponseWriter response) {

        if (req.resource() == "/ping") {
            if (req.method() == Net::Http::Method::Get) {

                using namespace Net::Http;

                auto query = req.query();
                if (query.has("chunked")) {
                    std::cout << "Using chunked encoding" << std::endl;

                    response.headers()
                        .add<Header::Server>("pistache/0.1")
                        .add<Header::ContentType>(MIME(Text, Plain));

                    response.cookies()
                        .add(Cookie("lang", "en-US"));

                    auto stream = response.stream(Net::Http::Code::Ok);
                    stream << "PO";
                    stream << "NG";
                    stream << ends;
                }
                else {
                    response.send(Net::Http::Code::Ok, "PONG");
                }

            }
        }
        else if (req.resource() == "/echo") {
            if (req.method() == Net::Http::Method::Post) {
                response.send(Net::Http::Code::Ok, req.body(), MIME(Text, Plain));
            } else {
                response.send(Net::Http::Code::Method_Not_Allowed);
            }
        }
        else if (req.resource() == "/exception") {
            throw std::runtime_error("Exception thrown in the handler");
        }
        else if (req.resource() == "/timeout") {
            response.timeoutAfter(std::chrono::seconds(2));
        }
        else if (req.resource() == "/static") {
            if (req.method() == Net::Http::Method::Get) {
                Net::Http::serveFile(response, "README.md").then([](ssize_t bytes) {;
                    std::cout << "Sent " << bytes << " bytes" << std::endl;
                }, Async::NoExcept);
            }
        } else {
            response.send(Http::Code::Not_Found);
        }

    }

    void onTimeout(const Net::Http::Request& req, Net::Http::ResponseWriter response) {
        response
            .send(Net::Http::Code::Request_Timeout, "Timeout")
            .then([=](ssize_t) { }, PrintException());
    }

};

int main(int argc, char *argv[]) {
    Net::Port port(9080);

    int thr = 2;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Net::Address addr(Net::Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    auto server = std::make_shared<Net::Http::Endpoint>(addr);

    auto opts = Net::Http::Endpoint::options()
        .threads(thr)
        .flags(Net::Tcp::Options::InstallSignalHandler);
    server->init(opts);
    server->setHandler(Http::make_handler<MyHandler>());
    server->serve();

    std::cout << "Shutdowning server" << std::endl;
    server->shutdown();
}
