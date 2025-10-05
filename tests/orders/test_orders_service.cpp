#include <gtest/gtest.h>
#include "../common/test_helpers.h"

class OrdersServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Path to the orders service executable
        servicePath = ExecutableTestHelper::getServiceExecutablePath("orders");
    }

    std::string servicePath;
};

// Basic test: check if the service responds
TEST_F(OrdersServiceTest, ServiceResponds) {
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);
    
    EXPECT_FALSE(output.empty());
}

// Test: check if it returns valid JSON with expected structure
TEST_F(OrdersServiceTest, ReturnsValidJsonStructure) {
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if it contains the expected keys
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "orders"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "total"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "source"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "endpoint"));
}

// Test: check if it returns orders
TEST_F(OrdersServiceTest, ReturnsOrders) {
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if the total is greater than 0
    std::string total = ExecutableTestHelper::extractJsonValue(output, "total");
    EXPECT_FALSE(total.empty());
    EXPECT_GT(std::stoi(total), 0);
}

// Test: check if it returns correct service metadata
TEST_F(OrdersServiceTest, ReturnsCorrectMetadata) {
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);
    
    std::string source = ExecutableTestHelper::extractJsonValue(output, "source");
    std::string endpoint = ExecutableTestHelper::extractJsonValue(output, "endpoint");
    
    EXPECT_EQ(source, "orders_service C++ executable");
    EXPECT_EQ(endpoint, "/orders");
}

// Test: check if it verifies the structure of orders
TEST_F(OrdersServiceTest, OrdersHaveCorrectFields) {
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if the first order has the expected fields
    EXPECT_TRUE(output.find("\"id\":") != std::string::npos);
    EXPECT_TRUE(output.find("\"userId\":") != std::string::npos);
    EXPECT_TRUE(output.find("\"product\":") != std::string::npos);
    EXPECT_TRUE(output.find("\"amount\":") != std::string::npos);
    EXPECT_TRUE(output.find("\"status\":") != std::string::npos);
}