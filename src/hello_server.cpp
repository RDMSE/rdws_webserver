#include "hello_server.h"
#include <iostream>

HelloServer::HelloServer(Pistache::Address addr)
    : httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr))
{
}

void HelloServer::init(size_t thr) {
    auto opts = Pistache::Http::Endpoint::options()
        .threads(static_cast<int>(thr));
    httpEndpoint->init(opts);
    setupRoutes();
}

void HelloServer::start() {
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void HelloServer::setupRoutes() {
    using namespace Pistache::Rest;

    Routes::Get(router, "/hello", Routes::bind(&HelloServer::helloHandler, this));
    Routes::Get(router, "/", Routes::bind(&HelloServer::helloHandler, this));
}

void HelloServer::helloHandler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    response.send(Pistache::Http::Code::Ok, "Hello World from C++ REST Server!");
}