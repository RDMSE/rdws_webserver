#include <gtest/gtest.h>
#include "hello_server.h"
#include <pistache/client.h>
#include <thread>
#include <chrono>
#include <curl/curl.h>

// Integration tests that test actual HTTP endpoints
class HelloServerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a free port for integration testing
        test_port = 19090;
        address = Pistache::Address(Pistache::Ipv4::loopback(), Pistache::Port(test_port));
        server = std::make_unique<HelloServer>(address);
        
        // Initialize and start server in a separate thread
        server->init(2);
        server_thread = std::thread([this]() {
            try {
                server->start();
            } catch (const std::exception& e) {
                // Server might be stopped, which is expected during teardown
            }
        });
        
        // Give the server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Initialize CURL
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void TearDown() override {
        // Cleanup
        curl_global_cleanup();
        
        // Note: In a real application, you'd want a proper shutdown mechanism
        // For testing, we'll let the thread terminate when the test ends
        if (server_thread.joinable()) {
            server_thread.detach(); // Let it run until process ends
        }
        server.reset();
    }

    // Helper function to make HTTP requests using CURL
    struct HTTPResponse {
        std::string body;
        long status_code;
    };

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HTTPResponse* response) {
        size_t total_size = size * nmemb;
        response->body.append((char*)contents, total_size);
        return total_size;
    }

    HTTPResponse makeRequest(const std::string& url) {
        CURL* curl;
        CURLcode res;
        HTTPResponse response;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5 second timeout
            
            res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            } else {
                response.status_code = -1; // Error indicator
            }
            
            curl_easy_cleanup(curl);
        }
        
        return response;
    }

    uint16_t test_port;
    Pistache::Address address;
    std::unique_ptr<HelloServer> server;
    std::thread server_thread;
};

// Test the root endpoint
TEST_F(HelloServerIntegrationTest, TestRootEndpoint) {
    std::string url = "http://127.0.0.1:" + std::to_string(test_port) + "/";
    
    // Wait a bit more for server to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    HTTPResponse response = makeRequest(url);
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.body, "Hello World from C++ REST Server!");
}

// Test the /hello endpoint
TEST_F(HelloServerIntegrationTest, TestHelloEndpoint) {
    std::string url = "http://127.0.0.1:" + std::to_string(test_port) + "/hello";
    
    // Wait a bit more for server to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    HTTPResponse response = makeRequest(url);
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.body, "Hello World from C++ REST Server!");
}

// Test non-existent endpoint (should return 404)
TEST_F(HelloServerIntegrationTest, TestNonExistentEndpoint) {
    std::string url = "http://127.0.0.1:" + std::to_string(test_port) + "/nonexistent";
    
    // Wait a bit more for server to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    HTTPResponse response = makeRequest(url);
    
    EXPECT_EQ(response.status_code, 404);
}

// Test multiple simultaneous requests
TEST_F(HelloServerIntegrationTest, TestConcurrentRequests) {
    std::string url = "http://127.0.0.1:" + std::to_string(test_port) + "/hello";
    
    // Wait for server to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::vector<std::thread> threads;
    std::vector<HTTPResponse> responses(5);
    
    // Make 5 concurrent requests
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([this, &url, &responses, i]() {
            responses[i] = makeRequest(url);
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all responses
    for (const auto& response : responses) {
        EXPECT_EQ(response.status_code, 200);
        EXPECT_EQ(response.body, "Hello World from C++ REST Server!");
    }
}