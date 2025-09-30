#include "hello_server.h"
#include <iostream>
#include <sstream>
#include <future>
#include <chrono>
#include <fstream>
#include <cstdlib>

HelloServer::HelloServer(Pistache::Address addr)
    : httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr))
{
    loadEnvironmentVariables();
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

    // Rota /hello agora faz proxy para função serverless
    Routes::Get(router, "/hello", Routes::bind(&HelloServer::proxyToServerlessFunction, this));
    
    // Rota / mantém comportamento original como fallback
    Routes::Get(router, "/", Routes::bind(&HelloServer::helloHandler, this));
}

void HelloServer::helloHandler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    response.send(Pistache::Http::Code::Ok, "Hello World from C++ REST Server!");
}

void HelloServer::proxyToServerlessFunction(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        std::cout << "Proxying /hello request to serverless function..." << std::endl;
        
        // Fazer requisição para função serverless
        std::string serverlessResponse = makeHttpRequest(serverlessFunctionUrl);
        
        if (!serverlessResponse.empty()) {
            // Função serverless respondeu - enviar resposta
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, serverlessResponse);
            std::cout << "Successfully proxied to serverless function" << std::endl;
        } else {
            // Fallback para resposta local se função serverless não responder
            std::cout << "Serverless function unavailable, using fallback" << std::endl;
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, 
                "{\"message\":\"Hello from C++ Server (serverless function unavailable)\",\"fallback\":true}");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error proxying to serverless function: " << e.what() << std::endl;
        
        // Fallback em caso de erro
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, 
            "{\"message\":\"Hello from C++ Server (proxy error)\",\"fallback\":true,\"error\":\"" + std::string(e.what()) + "\"}");
    }
}

std::string HelloServer::makeHttpRequest(const std::string& url) {
    try {
        // Usar Pistache HTTP Client para fazer requisição
        Pistache::Http::Client client;
        
        auto opts = Pistache::Http::Client::options()
            .threads(1)
            .maxConnectionsPerHost(8);
        client.init(opts);

        std::promise<std::string> responsePromise;
        auto responseFuture = responsePromise.get_future();

        auto response = client.get(url)
            .cookie(Pistache::Http::Cookie("lang", "en-US"))
            .send();

        response.then([&responsePromise](Pistache::Http::Response rsp) {
            std::cout << "Serverless function responded with status: " << rsp.code() << std::endl;
            if (rsp.code() == Pistache::Http::Code::Ok) {
                responsePromise.set_value(rsp.body());
            } else {
                responsePromise.set_value("");
            }
        }, [&responsePromise](std::exception_ptr exc) {
            try {
                std::rethrow_exception(exc);
            } catch (const std::exception& e) {
                std::cerr << "HTTP request failed: " << e.what() << std::endl;
            }
            responsePromise.set_value("");
        });

        // Aguardar resposta com timeout de 5 segundos
        auto status = responseFuture.wait_for(std::chrono::seconds(5));
        
        if (status == std::future_status::ready) {
            client.shutdown();
            return responseFuture.get();
        } else {
            std::cerr << "Serverless function request timeout" << std::endl;
            client.shutdown();
            return "";
        }
    } catch (const std::exception& e) {
        std::cerr << "HTTP client error: " << e.what() << std::endl;
        return "";
    }
}

void HelloServer::loadEnvironmentVariables() {
    // Tentar carregar arquivo .env primeiro
    std::ifstream envFile(".env");
    if (envFile.is_open()) {
        std::string line;
        while (std::getline(envFile, line)) {
            // Ignorar linhas vazias e comentários
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // Encontrar posição do =
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Remover espaços em branco
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // Remover aspas se existirem
                if (value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                
                // Definir variável de ambiente (apenas se não existir)
                if (std::getenv(key.c_str()) == nullptr) {
                    setenv(key.c_str(), value.c_str(), 0);
                }
            }
        }
        envFile.close();
        std::cout << "Loaded environment variables from .env file" << std::endl;
    }
    
    // Carregar URL da função serverless
    serverlessFunctionUrl = getEnvVar("SERVERLESS_FUNCTION_URL", "http://localhost:8082/");
    std::cout << "Serverless function URL: " << serverlessFunctionUrl << std::endl;
}

std::string HelloServer::getEnvVar(const std::string& key, const std::string& defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
}