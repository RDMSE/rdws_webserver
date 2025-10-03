#include <gtest/gtest.h>
#include "../common/test_helpers.h"

class UsersServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Path to the users service executable
        servicePath = ExecutableTestHelper::getServiceExecutablePath("users");
    }

    std::string servicePath;
};

// Basic test: check if the service responds
TEST_F(UsersServiceTest, ServiceResponds) {
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);
    
    EXPECT_FALSE(output.empty());
}

// Test: check if it returns valid JSON with expected structure
TEST_F(UsersServiceTest, ReturnsValidJsonStructure) {
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if it contains the expected keys
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "users"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "total"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "source"));
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "endpoint"));
}

// Test: check if it returns users
TEST_F(UsersServiceTest, ReturnsUsers) {
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if the total is greater than 0
    std::string total = ExecutableTestHelper::extractJsonValue(output, "total");
    EXPECT_FALSE(total.empty());
    EXPECT_GT(std::stoi(total), 0);
}

// Test: check if it returns correct service metadata
TEST_F(UsersServiceTest, ReturnsCorrectMetadata) {
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);
    
    std::string source = ExecutableTestHelper::extractJsonValue(output, "source");
    std::string endpoint = ExecutableTestHelper::extractJsonValue(output, "endpoint");
    
    EXPECT_EQ(source, "users_service C++ executable");
    EXPECT_EQ(endpoint, "/users");
}

// Test: check if it handles different parameters
TEST_F(UsersServiceTest, HandlesGetUsersById) {
    std::string command = servicePath + " \"GET\" \"/users/1\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Should respond with something (even if it's an error or not implemented)
    EXPECT_FALSE(output.empty());
}