#include <gtest/gtest.h>
#include "../common/test_helpers.h"
#include <regex>
#include <sstream>
#include <chrono>
#include <algorithm>

class UsersServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Path to the users service executable
        servicePath = ExecutableTestHelper::getServiceExecutablePath("users");

        // Verify the executable exists
        ASSERT_FALSE(servicePath.empty()) << "Users service executable not found";
    }

    std::string servicePath;

    // Helper function to count users in JSON response
    int countUsersInResponse(const std::string &json)
    {
        std::regex pattern(R"("id":\s*(\d+))");
        std::sregex_iterator iter(json.begin(), json.end(), pattern);
        std::sregex_iterator end;
        return std::distance(iter, end);
    }

    // Helper function to check if user exists in response
    bool userExistsInResponse(const std::string &json, int userId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(userId);
        return json.find(searchPattern) != std::string::npos;
    }

    // Helper function to extract user name by ID
    std::string extractUserName(const std::string &json, int userId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(userId);
        size_t pos = json.find(searchPattern);
        if (pos != std::string::npos)
        {
            // Find the name field after the id
            size_t namePos = json.find("\"name\":\"", pos);
            if (namePos != std::string::npos)
            {
                namePos += 8; // Skip '"name":"'
                size_t endPos = json.find("\"", namePos);
                if (endPos != std::string::npos)
                {
                    return json.substr(namePos, endPos - namePos);
                }
            }
        }
        return "";
    }
};

// ========== BASIC FUNCTIONALITY TESTS ==========

// Test: Service responds and exits successfully
TEST_F(UsersServiceTest, ServiceRespondsSuccessfully)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Service should return output";
    EXPECT_TRUE(output.find("{") != std::string::npos) << "Output should contain JSON";
    EXPECT_TRUE(output.find("}") != std::string::npos) << "Output should be valid JSON";
}

// Test: Returns valid JSON with complete expected structure
TEST_F(UsersServiceTest, ReturnsValidJsonStructure)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if it contains all required keys
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "users")) << "Should contain 'users' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "total")) << "Should contain 'total' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "source")) << "Should contain 'source' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "success")) << "Should contain 'success' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "timestamp")) << "Should contain 'timestamp' key";

    // Verify JSON is properly formatted
    EXPECT_TRUE(output.find("[") != std::string::npos) << "Should contain array of users";
    EXPECT_TRUE(output.find("]") != std::string::npos) << "Should close array of users";
}

// ========== DATA VALIDATION TESTS ==========

// Test: Returns users with correct count
TEST_F(UsersServiceTest, ReturnsCorrectUserCount)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if the total is greater than 0
    std::string total = ExecutableTestHelper::extractJsonValue(output, "total");
    EXPECT_FALSE(total.empty()) << "Total should not be empty";

    int totalCount = std::stoi(total);
    EXPECT_GT(totalCount, 0) << "Should have at least one user";
    EXPECT_GE(totalCount, 5) << "Should have at least 5 users as per database data";

    // Verify actual count matches reported total
    int actualCount = countUsersInResponse(output);
    EXPECT_EQ(actualCount, totalCount) << "Actual user count should match total field";
}

// Test: Users have required fields
TEST_F(UsersServiceTest, UsersHaveRequiredFields)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if users have all required fields
    EXPECT_TRUE(output.find("\"id\":") != std::string::npos) << "Users should have id field";
    EXPECT_TRUE(output.find("\"name\":") != std::string::npos) << "Users should have name field";
    EXPECT_TRUE(output.find("\"email\":") != std::string::npos) << "Users should have email field";

    // Check for specific users from mock data
    EXPECT_TRUE(userExistsInResponse(output, 1)) << "Should contain user with ID 1";
    EXPECT_TRUE(userExistsInResponse(output, 2)) << "Should contain user with ID 2";
    EXPECT_TRUE(userExistsInResponse(output, 5)) << "Should contain user with ID 5";
}

// Test: Validates user data integrity
TEST_F(UsersServiceTest, ValidatesUserDataIntegrity)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check specific user data
    std::string user1Name = extractUserName(output, 1);
    EXPECT_EQ(user1Name, "John Doe") << "User 1 should be John Doe";

    // Verify email format is present
    EXPECT_TRUE(output.find("@example.com") != std::string::npos) << "Should contain proper email format";

    // Verify no empty or null values in critical fields
    EXPECT_FALSE(output.find("\"name\":\"\"") != std::string::npos) << "Should not have empty names";
    EXPECT_FALSE(output.find("\"email\":\"\"") != std::string::npos) << "Should not have empty emails";
}

// ========== METADATA AND ENDPOINT TESTS ==========

// Test: Returns correct service metadata
TEST_F(UsersServiceTest, ReturnsCorrectMetadata)
{
    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    std::string source = ExecutableTestHelper::extractJsonValue(output, "source");
    std::string timestamp = ExecutableTestHelper::extractJsonValue(output, "timestamp");
    std::string success = ExecutableTestHelper::extractJsonValue(output, "success");

    EXPECT_EQ(source, "microservice C++ with PostgreSQL") << "Source should identify the service";
    EXPECT_EQ(success, "true") << "Should return success true";
    EXPECT_FALSE(timestamp.empty()) << "Should include timestamp";
    EXPECT_GT(std::stoll(timestamp), 0) << "Timestamp should be valid";
}

// ========== INDIVIDUAL USER TESTS ==========

// Test: Handles get user by valid ID
TEST_F(UsersServiceTest, HandlesGetUserByValidId)
{
    std::string command = servicePath + " \"GET\" \"/users/1\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for valid user ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "user")) << "Should contain 'user' key for single user";
    EXPECT_TRUE(output.find("\"id\":1") != std::string::npos) << "Should return user with ID 1";
    EXPECT_TRUE(output.find("John Doe") != std::string::npos) << "Should return correct user name";
}

// Test: Handles get user by another valid ID
TEST_F(UsersServiceTest, HandlesGetUserByValidIdTwo)
{
    std::string command = servicePath + " \"GET\" \"/users/2\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for valid user ID 2";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "user")) << "Should contain 'user' key";
    EXPECT_TRUE(output.find("\"id\":2") != std::string::npos) << "Should return user with ID 2";
    EXPECT_TRUE(output.find("Jane Smith") != std::string::npos) << "Should return correct user name";
}

// Test: Handles non-existent user ID
TEST_F(UsersServiceTest, HandlesNonExistentUserId)
{
    std::string command = servicePath + " \"GET\" \"/users/999\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error response for non-existent user";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("User not found") != std::string::npos) << "Should indicate user not found";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "success")) << "Should contain success field";
    std::string success = ExecutableTestHelper::extractJsonValue(output, "success");
    EXPECT_EQ(success, "false") << "Should return success false for error";
}

// Test: Handles invalid user ID format
TEST_F(UsersServiceTest, HandlesInvalidUserIdFormat)
{
    std::string command = servicePath + " \"GET\" \"/users/invalid\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error response for invalid ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("Invalid user ID") != std::string::npos) << "Should indicate invalid ID format";
}

// ========== METHOD AND PATH TESTS ==========

// Test: Handles POST method (user creation)
TEST_F(UsersServiceTest, HandlesPostMethod)
{
    std::string command = servicePath + " \"POST\" \"/users\" '{\"name\":\"Test User\",\"email\":\"test@example.com\"}'";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should handle POST requests";
    // Should either succeed with user creation or fail with validation error
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "success") ||
                ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should return valid response";
}

// Test: Handles unsupported HTTP methods
TEST_F(UsersServiceTest, HandlesUnsupportedMethods)
{
    std::string command = servicePath + " \"PATCH\" \"/users/1\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error for unsupported method";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error") ||
                ExecutableTestHelper::jsonContainsKey(output, "success")) << "Should contain response";
}

// Test: Handles root path
TEST_F(UsersServiceTest, HandlesRootPath)
{
    std::string command = servicePath + " \"GET\" \"/\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should handle root path";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "users")) << "Should return users list for root path";
}

// ========== EDGE CASES AND ERROR HANDLING ==========

// Test: Handles no arguments
TEST_F(UsersServiceTest, HandlesNoArguments)
{
    std::string command = servicePath;
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should handle default arguments";
    // Should default to GET /users
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "users")) << "Should default to users list";
}

// Test: Performance - service responds quickly
TEST_F(UsersServiceTest, ServicePerformance)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::string command = servicePath + " \"GET\" \"/users\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_FALSE(output.empty()) << "Should return response";
    EXPECT_LT(duration.count(), 1000) << "Should respond within 1 second";
}
