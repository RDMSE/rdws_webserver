#ifndef HELLO_SERVER_H
#define HELLO_SERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/client.h>
#include <string>

class HelloServer
{
public:
    explicit HelloServer(Pistache::Address addr);
    void init(size_t thr = 2);
    void start();

private:
    void setupRoutes();
    void helloHandler(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void usersHandler(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void proxyToServerlessFunction(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    std::string makeHttpRequest(const std::string &url);
    void loadEnvironmentVariables();
    std::string getEnvVar(const std::string &key, const std::string &defaultValue = "");

    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router router;
    std::string serverlessFunctionUrl;
};

#endif // HELLO_SERVER_H
