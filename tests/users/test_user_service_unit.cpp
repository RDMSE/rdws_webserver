#include "../mocks/mock_database.h"

#include <gtest/gtest.h>
using rdws::testing::MockDatabase;
#include "../../src/services/users/user_service.h"
#include "controllers/user_controller.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
// Include MockUserResultSet for user service unit tests
#include "../mocks/mock_user_result_set.h"

class UserServiceUnitTest : public ::testing::Test {
  protected:
    void SetUp() override {
        mockDb = std::make_shared<MockDatabase>();
        mockDb->reset(); // Ensure clean state
        userService = std::make_unique<rdws::users::UserService>(mockDb);
        userController = std::make_unique<rdws::controllers::UserController>();

        // Setup default expectations
        setupDefaultExpectations();
    }

    void setupDefaultExpectations() {
        using ::testing::_;
        using ::testing::Return;

        // Default fallback: return empty result set for any unmatched query
        EXPECT_CALL(*mockDb, execQuery(_, _))
            .WillRepeatedly(testing::Invoke(
                [](const std::string& query, const std::vector<std::string>& params) {
                    (void)query;
                    (void)params;
                    return std::make_unique<rdws::testing::MockUserResultSet>(
                        std::vector<std::map<std::string, std::string>>{});
                }));
    }

    std::shared_ptr<MockDatabase> mockDb;
    std::unique_ptr<rdws::users::UserService> userService;
    std::unique_ptr<rdws::controllers::UserController> userController;

    // Helper to parse JSON and check structure
    bool isValidJson(const std::string& json) {
        rapidjson::Document doc;
        return !doc.Parse(json.c_str()).HasParseError();
    }

    // Helper to extract value from JSON response
    std::string extractJsonValue(const std::string& json, const std::string& key) {
        rapidjson::Document doc;
        if (doc.Parse(json.c_str()).HasParseError())
            return "";

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
};

// Test UserService returns correct ServiceResult for getAllUsers
TEST_F(UserServiceUnitTest, GetAllUsers_ServiceReturnsSuccessResult) {
    using ::testing::_;
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {{{"id", "1"},
                                                                 {"name", "John Doe"},
                                                                 {"email", "john@example.com"},
                                                                 {"created_at", "2023-01-01"}},
                                                                {{"id", "2"},
                                                                 {"name", "Jane Smith"},
                                                                 {"email", "jane@example.com"},
                                                                 {"created_at", "2023-01-02"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    auto result = userService->getAllUsers();

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().size(), 2) << "Should return 2 users";
    EXPECT_EQ(result.getData()[0].id, 1) << "First user should have ID 1";
    EXPECT_EQ(result.getData()[0].name, "John Doe") << "First user should be John Doe";
}

// Test UserController formats ServiceResult correctly
TEST_F(UserServiceUnitTest, GetAllUsers_ControllerFormatsJsonCorrectly) {
    using ::testing::_;
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {{{"id", "1"},
                                                                 {"name", "John Doe"},
                                                                 {"email", "john@example.com"},
                                                                 {"created_at", "2023-01-01"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    auto serviceResult = userService->getAllUsers();
    std::string json = userController->formatUsersResponse(serviceResult);

    EXPECT_TRUE(isValidJson(json)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(json, "success"), "true") << "Should indicate success";
    EXPECT_FALSE(extractJsonValue(json, "timestamp").empty()) << "Should include timestamp";
}

// Test getUserById with valid ID
TEST_F(UserServiceUnitTest, GetUserById_ValidId_ReturnsUser) {
    using ::testing::_;
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {{{"id", "1"},
                                                                 {"name", "John Doe"},
                                                                 {"email", "john@example.com"},
                                                                 {"created_at", "2023-01-01"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1",
                                   std::vector<std::string>{"1"}))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    auto result = userService->getUserById(1);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().id, 1) << "Should return user with ID 1";
    EXPECT_EQ(result.getData().name, "John Doe") << "Should return correct user name";
}

// Test getUserById with non-existent ID
TEST_F(UserServiceUnitTest, GetUserById_NonExistentId_ReturnsError) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1",
                                   std::vector<std::string>{"999"}))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(
            std::vector<std::map<std::string, std::string>>{})));

    auto result = userService->getUserById(999);

    EXPECT_FALSE(result.isSuccess()) << "Service should return error";
    EXPECT_EQ(result.getErrorMessage(), "User not found")
        << "Should return appropriate error message";
}

// Test getUsersCount
TEST_F(UserServiceUnitTest, GetUsersCount_ReturnsCorrectCount) {
    using ::testing::_;
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {{{"total", "3"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT COUNT(*) as total FROM users", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    auto result = userService->getUsersCount();

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData(), 3) << "Should return count of 3";
}

// Test createUser with valid JSON
TEST_F(UserServiceUnitTest, CreateUser_ValidData_CreatesUser) {
    using ::testing::_;
    using ::testing::Return;

    std::string jsonData = R"({"name": "New User", "email": "new@example.com"})";

    // Mock successful insert command
    EXPECT_CALL(*mockDb, execCommand(testing::HasSubstr("INSERT INTO users"), _))
        .WillOnce(Return(true));

    // Mock the subsequent findAll query to return the created user
    std::vector<std::map<std::string, std::string>> mockRows = {{{"id", "4"},
                                                                 {"name", "New User"},
                                                                 {"email", "new@example.com"},
                                                                 {"created_at", "2023-01-04"}}};

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("SELECT"), testing::IsEmpty()))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    auto result = userService->createUser(jsonData);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().name, "New User") << "Should return created user";
}

// Test createUser with invalid JSON
TEST_F(UserServiceUnitTest, CreateUser_InvalidJson_ReturnsError) {
    std::string invalidJson = "{invalid json}";

    auto result = userService->createUser(invalidJson);

    EXPECT_FALSE(result.isSuccess()) << "Service should return error";
    EXPECT_TRUE(result.getErrorMessage().find("Invalid JSON") != std::string::npos)
        << "Should return JSON error message";
}

// Test updateUser
TEST_F(UserServiceUnitTest, UpdateUser_ValidData_UpdatesUser) {
    using ::testing::_;
    using ::testing::Return;

    std::string jsonData = R"({"name": "Updated User"})";

    // Mock finding existing user
    std::vector<std::map<std::string, std::string>> existingUser = {{{"id", "1"},
                                                                     {"name", "John Doe"},
                                                                     {"email", "john@example.com"},
                                                                     {"created_at", "2023-01-01"}}};

    std::vector<std::map<std::string, std::string>> updatedUser = {{{"id", "1"},
                                                                    {"name", "Updated User"},
                                                                    {"email", "john@example.com"},
                                                                    {"created_at", "2023-01-01"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1",
                                   std::vector<std::string>{"1"}))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(existingUser)));

    EXPECT_CALL(*mockDb, execCommand(testing::HasSubstr("UPDATE users"), _)).WillOnce(Return(true));

    auto result = userService->updateUser(1, jsonData);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().name, "Updated User") << "Should return updated user";
}

// Test deleteUser
TEST_F(UserServiceUnitTest, DeleteUser_ValidId_DeletesUser) {
    using ::testing::_;
    using ::testing::Return;

    // Mock finding existing user first
    std::vector<std::map<std::string, std::string>> existingUser = {{{"id", "1"},
                                                                     {"name", "John Doe"},
                                                                     {"email", "john@example.com"},
                                                                     {"created_at", "2023-01-01"}}};

    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1",
                                   std::vector<std::string>{"1"}))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(existingUser)));

    EXPECT_CALL(*mockDb,
                execCommand("DELETE FROM users WHERE id = $1", std::vector<std::string>{"1"}))
        .WillOnce(Return(true));

    auto result = userService->deleteUser(1);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().message, "User deleted successfully")
        << "Should return success message";
}

// Test that controllers format error responses correctly
TEST_F(UserServiceUnitTest, Controller_FormatsErrorResponseCorrectly) {
    auto errorResult = rdws::types::ServiceResult<rdws::types::User>::error("Test error message");
    std::string json = userController->formatUserResponse(errorResult);

    EXPECT_TRUE(isValidJson(json)) << "Should return valid JSON even for errors";
    EXPECT_EQ(extractJsonValue(json, "success"), "false") << "Should indicate failure";
    EXPECT_EQ(extractJsonValue(json, "error"), "Test error message")
        << "Should include error message";
}
