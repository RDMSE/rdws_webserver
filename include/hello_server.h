#ifndef HELLO_SERVER_H
#define HELLO_SERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

class HelloServer {
public:
    explicit HelloServer(Pistache::Address addr);
    void init(size_t thr = 2);
    void start();

private:
    void setupRoutes();
    void helloHandler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router router;
};

#endif // HELLO_SERVER_H