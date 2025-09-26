#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "hello_server.h"
#include <pistache/http.h>
#include <pistache/router.h>

using namespace testing;

class HelloServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test server
        address = Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(0)); // Use port 0 for testing
        server = std::make_unique<HelloServer>(address);
    }

    void TearDown() override {
        server.reset();
    }

    Pistache::Address address;
    std::unique_ptr<HelloServer> server;
};

// Test server initialization
TEST_F(HelloServerTest, ServerInitialization) {
    ASSERT_NE(server, nullptr);
    
    // Test that init doesn't throw
    EXPECT_NO_THROW(server->init(1));
}

// Test server construction with different addresses
TEST(HelloServerConstructorTest, ValidAddress) {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
    EXPECT_NO_THROW(HelloServer server(addr));
}

// Test with IPv4 localhost
TEST(HelloServerConstructorTest, LocalhostAddress) {
    Pistache::Address addr(Pistache::Ipv4::loopback(), Pistache::Port(9090));
    EXPECT_NO_THROW(HelloServer server(addr));
}

// Mock class for testing HTTP responses (simplified approach)
class MockResponseWriter {
public:
    MOCK_METHOD(void, send, (Pistache::Http::Code code, const std::string& body), ());
};

// Since HelloServer uses Pistache's ResponseWriter which is not easily mockable,
// we'll test the response logic by creating a test that verifies the server
// can handle basic operations without network I/O

class HelloServerFunctionalTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a random high port for testing
        test_port = 19080; // High port less likely to conflict
        address = Pistache::Address(Pistache::Ipv4::loopback(), Pistache::Port(test_port));
        server = std::make_unique<HelloServer>(address);
    }

    void TearDown() override {
        server.reset();
    }

    uint16_t test_port;
    Pistache::Address address;
    std::unique_ptr<HelloServer> server;
};

TEST_F(HelloServerFunctionalTest, ServerCanInitialize) {
    EXPECT_NO_THROW(server->init(1));
}

// Test that we can create multiple instances
TEST(HelloServerMultipleInstancesTest, CanCreateMultipleInstances) {
    Pistache::Address addr1(Pistache::Ipv4::loopback(), Pistache::Port(19081));
    Pistache::Address addr2(Pistache::Ipv4::loopback(), Pistache::Port(19082));
    
    HelloServer server1(addr1);
    HelloServer server2(addr2);
    
    EXPECT_NO_THROW(server1.init(1));
    EXPECT_NO_THROW(server2.init(1));
}

// Integration-style test that verifies the server configuration
class HelloServerConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        address = Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(0));
        server = std::make_unique<HelloServer>(address);
    }

    Pistache::Address address;
    std::unique_ptr<HelloServer> server;
};

TEST_F(HelloServerConfigTest, InitializationWithDifferentThreadCounts) {
    // Test with 1 thread
    EXPECT_NO_THROW(server->init(1));
    
    // Create new server for different thread count
    HelloServer server2(address);
    EXPECT_NO_THROW(server2.init(4));
    
    // Test with many threads
    HelloServer server3(address);
    EXPECT_NO_THROW(server3.init(8));
}

// Test edge cases
TEST(HelloServerEdgeCasesTest, ZeroThreads) {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(0));
    HelloServer server(addr);
    
    // Pistache should handle 0 threads gracefully (likely converting to 1)
    EXPECT_NO_THROW(server.init(0));
}