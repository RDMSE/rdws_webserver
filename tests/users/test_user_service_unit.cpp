#include <gtest/gtest.h>
#include "../mocks/mock_database.h"
#include "../../src/services/users/user_service.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

class UserServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockDb = std::make_shared<rdws::testing::MockDatabase>();
        mockDb->reset(); // Ensure clean state
        userService = std::make_unique<rdws::users::UserService>(mockDb);
    }

    void TearDown() override {
        userService.reset();
        mockDb.reset();
    }

    std::shared_ptr<rdws::testing::MockDatabase> mockDb;
    std::unique_ptr<rdws::users::UserService> userService;

    // Helper to parse JSON and check structure
    bool isValidJson(const std::string& json) {
        rapidjson::Document doc;
        return !doc.Parse(json.c_str()).HasParseError();
    }

    // Helper to extract value from JSON response
    std::string extractJsonValue(const std::string& json, const std::string& key) {
        rapidjson::Document doc;
        if (doc.Parse(json.c_str()).HasParseError()) return "";

        if (doc.HasMember(key.c_str())) {
            const auto& value = doc[key.c_str()];
            if (value.IsString()) {
                return value.GetString();
            } else if (value.IsInt()) {
                return std::to_string(value.GetInt());
            } else if (value.IsBool()) {
                return value.GetBool() ? "true" : "false";
            }
        }
        return "";
    }

    // Helper to count users in JSON array
    int countUsersInJson(const std::string& json) {
        rapidjson::Document doc;
        if (doc.Parse(json.c_str()).HasParseError()) return 0;

        if (doc.HasMember("users") && doc["users"].IsArray()) {
            return doc["users"].Size();
        }
        return 0;
    }
};


// ========== BASIC FUNCTIONALITY TESTS ==========

TEST_F(UserServiceUnitTest, GetAllUsers_ReturnsValidJsonStructure) {
    std::string result = userService->getAllUsers();

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_FALSE(extractJsonValue(result, "timestamp").empty()) << "Should include timestamp";
    EXPECT_EQ(extractJsonValue(result, "source"), "microservice C++ with PostgreSQL");
}

TEST_F(UserServiceUnitTest, GetAllUsers_ReturnsCorrectUserCount) {
    std::string result = userService->getAllUsers();

    int userCount = countUsersInJson(result);
    std::string totalStr = extractJsonValue(result, "total");
    int total = totalStr.empty() ? 0 : std::stoi(totalStr);

    EXPECT_EQ(userCount, 3) << "Should return 3 mock users";
    EXPECT_EQ(total, 3) << "Total field should match actual count";
    EXPECT_EQ(userCount, total) << "Array count should match total field";
}

TEST_F(UserServiceUnitTest, GetAllUsers_ContainsMockUserData) {
    std::string result = userService->getAllUsers();

    EXPECT_TRUE(result.find("John Doe") != std::string::npos) << "Should contain John Doe";
    EXPECT_TRUE(result.find("Jane Smith") != std::string::npos) << "Should contain Jane Smith";
    EXPECT_TRUE(result.find("Bob Johnson") != std::string::npos) << "Should contain Bob Johnson";
    EXPECT_TRUE(result.find("john@example.com") != std::string::npos) << "Should contain John's email";
}

// ========== GET USER BY ID TESTS ==========

TEST_F(UserServiceUnitTest, GetUserById_ValidId_ReturnsUser) {
    std::string result = userService->getUserById(1);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("John Doe") != std::string::npos) << "Should return John Doe";
    EXPECT_TRUE(result.find("\"user\":") != std::string::npos) << "Should contain user object";
}

TEST_F(UserServiceUnitTest, GetUserById_ValidId_ReturnsCorrectUser) {
    std::string result = userService->getUserById(2);

    EXPECT_TRUE(result.find("Jane Smith") != std::string::npos) << "Should return Jane Smith";
    EXPECT_TRUE(result.find("jane@example.com") != std::string::npos) << "Should return Jane's email";
    EXPECT_TRUE(result.find("\"id\":2") != std::string::npos) << "Should return correct ID";
}

TEST_F(UserServiceUnitTest, GetUserById_NonExistentId_ReturnsError) {
    std::string result = userService->getUserById(999);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";
    EXPECT_TRUE(result.find("\"error\":") != std::string::npos) << "Should contain error field";
}

// ========== GET USERS COUNT TESTS ==========

TEST_F(UserServiceUnitTest, GetUsersCount_ReturnsCorrectCount) {
    std::string result = userService->getUsersCount();

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";

    // Parse the count from the nested data structure
    rapidjson::Document doc;
    ASSERT_FALSE(doc.Parse(result.c_str()).HasParseError());
    ASSERT_TRUE(doc.HasMember("data"));
    ASSERT_TRUE(doc["data"].HasMember("count"));
    EXPECT_EQ(doc["data"]["count"].GetInt(), 3) << "Should return count of 3";
}

// ========== CREATE USER TESTS ==========

TEST_F(UserServiceUnitTest, CreateUser_ValidData_CreatesUser) {
    std::string jsonData = R"({"name":"New User","email":"new@example.com"})";

    std::string result = userService->createUser(jsonData);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("New User") != std::string::npos) << "Should return created user";
    EXPECT_TRUE(result.find("new@example.com") != std::string::npos) << "Should return user email";
}

TEST_F(UserServiceUnitTest, CreateUser_InvalidJson_ReturnsError) {
    std::string invalidJson = R"({"name":"New User","email":})"; // Invalid JSON
    std::string result = userService->createUser(invalidJson);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON error response";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("Validation failed") != std::string::npos) << "Should contain validation error message";
}

TEST_F(UserServiceUnitTest, CreateUser_MissingFields_ReturnsValidationError) {
    std::string incompleteJson = R"({"name":"New User"})"; // Missing email
    std::string result = userService->createUser(incompleteJson);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON error response";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("Validation failed") != std::string::npos) << "Should contain validation error";
}

// ========== UPDATE USER TESTS ==========

TEST_F(UserServiceUnitTest, UpdateUser_ValidData_UpdatesUser) {
    std::string jsonData = R"({"name":"Updated John","email":"updated.john@example.com"})";

    std::string result = userService->updateUser(1, jsonData);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("Updated John") != std::string::npos) << "Should return updated name";
    EXPECT_TRUE(result.find("updated.john@example.com") != std::string::npos) << "Should return updated email";
}

TEST_F(UserServiceUnitTest, UpdateUser_NonExistentId_ReturnsError) {
    std::string jsonData = R"({"name":"Updated User","email":"updated@example.com"})";

    std::string result = userService->updateUser(999, jsonData);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";
}

TEST_F(UserServiceUnitTest, UpdateUser_PartialUpdate_UpdatesOnlyProvidedFields) {
    std::string jsonData = R"({"name":"Only Name Updated"})"; // Only update name

    std::string result = userService->updateUser(2, jsonData);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("Only Name Updated") != std::string::npos) << "Should return updated name";
    EXPECT_TRUE(result.find("jane@example.com") != std::string::npos) << "Should keep original email";
}

// ========== DELETE USER TESTS ==========

TEST_F(UserServiceUnitTest, DeleteUser_ValidId_DeletesUser) {
    std::string result = userService->deleteUser(1);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("User deleted successfully") != std::string::npos) << "Should contain success message";

    // Verify user was actually removed from mock database
    EXPECT_EQ(mockDb->getUserCount(), 2) << "Should have 2 users after deletion";

    // Verify the deleted user cannot be retrieved
    std::string getResult = userService->getUserById(1);
    EXPECT_EQ(extractJsonValue(getResult, "success"), "false") << "Deleted user should not be found";
}

TEST_F(UserServiceUnitTest, DeleteUser_NonExistentId_ReturnsError) {
    std::string result = userService->deleteUser(999);

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";

    // Verify no users were deleted
    EXPECT_EQ(mockDb->getUserCount(), 3) << "Should still have 3 users";
}

// ========== DATABASE ERROR TESTS ==========

TEST_F(UserServiceUnitTest, GetAllUsers_DatabaseConnection_ChecksConnection) {
    // Disconnect the mock database
    mockDb->setConnectionStatus(false);

    std::string result = userService->getAllUsers();

    // Should still work as the database mock always returns data
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
}

// ========== EDGE CASES ==========

TEST_F(UserServiceUnitTest, GetAllUsers_EmptyDatabase_ReturnsEmptyArray) {
    mockDb->clearUsers();

    std::string result = userService->getAllUsers();

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_EQ(countUsersInJson(result), 0) << "Should return empty array";
}

TEST_F(UserServiceUnitTest, GetUsersCount_ReturnsZero) {
    mockDb->clearUsers();

    std::string result = userService->getUsersCount();

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";

    rapidjson::Document doc;
    ASSERT_FALSE(doc.Parse(result.c_str()).HasParseError());
    ASSERT_TRUE(doc.HasMember("data"));
    ASSERT_TRUE(doc["data"].HasMember("count"));
    EXPECT_EQ(doc["data"]["count"].GetInt(), 0) << "Should return count of 0";
}
