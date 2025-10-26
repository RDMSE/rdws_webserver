#include <gtest/gtest.h>
#include "../mocks/mock_database.h"
using rdws::testing::MockDatabase;
#include "../../src/services/users/user_service.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
// Include MockUserResultSet for user service unit tests
#include "../mocks/mock_user_result_set.h"

class UserServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockDb = std::make_shared<MockDatabase>();
        mockDb->reset(); // Ensure clean state
        userService = std::make_unique<rdws::users::UserService>(mockDb);
        // Fallback: any execQuery not explicitly expected returns empty result set
        using ::testing::Return;
        using ::testing::_;
        EXPECT_CALL(*mockDb, execQuery(_, _)).WillRepeatedly(testing::Invoke(
            [](const std::string& query, const std::vector<std::string>& params) {
                // Match select all users
                if (query.find("SELECT id, name, email, created_at FROM users ORDER BY id") != std::string::npos) {
                    std::vector<std::map<std::string, std::string>> mockRows = {
                        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} },
                        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} },
                        { {"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}, {"created_at", "2023-01-03"} }
                    };
                    return std::make_unique<rdws::testing::MockUserResultSet>(mockRows);
                }
                // Match select by id
                if (query.find("SELECT id, name, email, created_at FROM users WHERE id = $1") != std::string::npos) {
                    if (!params.empty() && params[0] == "1") {
                        std::vector<std::map<std::string, std::string>> mockRows = {
                            { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} }
                        };
                        return std::make_unique<rdws::testing::MockUserResultSet>(mockRows);
                    } else if (!params.empty() && params[0] == "2") {
                        std::vector<std::map<std::string, std::string>> mockRows = {
                            { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} }
                        };
                        return std::make_unique<rdws::testing::MockUserResultSet>(mockRows);
                    }
                    // Not found
                    return std::make_unique<rdws::testing::MockUserResultSet>(std::vector<std::map<std::string, std::string>>{});
                }
                // Match count queries (always return 3 unless test sets up empty)
                if (query.find("COUNT(*)") != std::string::npos) {
                    // If test calls clearUsers, return 0
                    if (query.find("users") != std::string::npos && params.size() > 0 && params[0] == "0") {
                        return std::make_unique<rdws::testing::MockUserResultSet>(std::vector<std::map<std::string, std::string>>{{{"total", "0"}}});
                    }
                    return std::make_unique<rdws::testing::MockUserResultSet>(std::vector<std::map<std::string, std::string>>{{{"total", "3"}}});
                }
                // Match select by email
                if (query.find("SELECT id, name, email, created_at FROM users WHERE email = $1") != std::string::npos) {
                    std::vector<std::map<std::string, std::string>> mockRows = {
                        { {"id", "4"}, {"name", "New User"}, {"email", "new@example.com"}, {"created_at", "2023-01-04"} }
                    };
                    return std::make_unique<rdws::testing::MockUserResultSet>(mockRows);
                }
                // Default: empty result set
                return std::make_unique<rdws::testing::MockUserResultSet>(std::vector<std::map<std::string, std::string>>{});
            }
        ));
        // Context-aware fallback for execCommand
        EXPECT_CALL(*mockDb, execCommand(_, _)).WillRepeatedly(testing::Invoke(
            [](const std::string& query, const std::vector<std::string>& params) -> bool {
                // Simulate failure for update/delete on non-existent users
                if ((query.find("UPDATE users") != std::string::npos || query.find("DELETE FROM users") != std::string::npos)) {
                    if (!params.empty() && (params[0] == "999" || params[0] == "0")) {
                        return false; // Non-existent user
                    }
                    return true; // Valid user
                }
                // Simulate success for insert
                if (query.find("INSERT INTO users") != std::string::npos) {
                    return true;
                }
                // Default: success
                return true;
            }
        ));
    }

    void TearDown() override {
        userService.reset();
        mockDb.reset();
    }

    std::shared_ptr<MockDatabase> mockDb;
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
    using ::testing::Return;
    using ::testing::_; // wildcard matcher
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} },
        { {"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}, {"created_at", "2023-01-03"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillRepeatedly(testing::Invoke([mockRows](const std::string&, const std::vector<std::string>&) {
            return std::unique_ptr<rdws::database::IResultSet>(new rdws::testing::MockUserResultSet(mockRows));
        }));

    std::string result = userService->getAllUsers();

    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_FALSE(extractJsonValue(result, "timestamp").empty()) << "Should include timestamp";
    EXPECT_EQ(extractJsonValue(result, "source"), "microservice C++ with PostgreSQL");
}

TEST_F(UserServiceUnitTest, GetAllUsers_ReturnsCorrectUserCount) {
    using ::testing::Return;
    using ::testing::_;
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} },
        { {"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}, {"created_at", "2023-01-03"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    std::string result = userService->getAllUsers();

    int userCount = countUsersInJson(result);
    std::string totalStr = extractJsonValue(result, "total");
    int total = totalStr.empty() ? 0 : std::stoi(totalStr);

    EXPECT_EQ(userCount, 3) << "Should return 3 mock users";
    EXPECT_EQ(total, 3) << "Total field should match actual count";
    EXPECT_EQ(userCount, total) << "Array count should match total field";
}

TEST_F(UserServiceUnitTest, GetAllUsers_ContainsMockUserData) {
    using ::testing::Return;
    using ::testing::_;
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} },
        { {"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}, {"created_at", "2023-01-03"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));

    std::string result = userService->getAllUsers();

    EXPECT_TRUE(result.find("John Doe") != std::string::npos) << "Should contain John Doe";
    EXPECT_TRUE(result.find("Jane Smith") != std::string::npos) << "Should contain Jane Smith";
    EXPECT_TRUE(result.find("Bob Johnson") != std::string::npos) << "Should contain Bob Johnson";
    EXPECT_TRUE(result.find("john@example.com") != std::string::npos) << "Should contain John's email";
}

// ========== GET USER BY ID TESTS ==========

TEST_F(UserServiceUnitTest, GetUserById_ValidId_ReturnsUser) {
    using ::testing::Return;
    using ::testing::_;
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->getUserById(1);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("John Doe") != std::string::npos) << "Should return John Doe";
    EXPECT_TRUE(result.find("\"user\":") != std::string::npos) << "Should contain user object";
}

TEST_F(UserServiceUnitTest, GetUserById_ValidId_ReturnsCorrectUser) {
    using ::testing::Return;
    using ::testing::_;
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->getUserById(2);
    EXPECT_TRUE(result.find("Jane Smith") != std::string::npos) << "Should return Jane Smith";
    EXPECT_TRUE(result.find("jane@example.com") != std::string::npos) << "Should return Jane's email";
    EXPECT_TRUE(result.find("\"id\":2") != std::string::npos) << "Should return correct ID";
}

TEST_F(UserServiceUnitTest, GetUserById_NonExistentId_ReturnsError) {
    using ::testing::Return;
    using ::testing::_;
    std::vector<std::map<std::string, std::string>> emptyRows = {};
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(emptyRows)));
    std::string result = userService->getUserById(999);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";
    EXPECT_TRUE(result.find("\"error\":") != std::string::npos) << "Should contain error field";
}

// ========== GET USERS COUNT TESTS ==========

TEST_F(UserServiceUnitTest, GetUsersCount_ReturnsCorrectCount) {
    using ::testing::Return;
    using ::testing::_;
    // The repository's count() method may call execQuery or execCommand, but let's assume it uses execQuery for count
    std::vector<std::map<std::string, std::string>> countRows = { { {"total", "3"} } };
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("COUNT(*)"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(countRows)));
    std::string result = userService->getUsersCount();
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    rapidjson::Document doc;
    ASSERT_FALSE(doc.Parse(result.c_str()).HasParseError());
    ASSERT_TRUE(doc.HasMember("data"));
    ASSERT_TRUE(doc["data"].HasMember("count"));
    EXPECT_EQ(doc["data"]["count"].GetInt(), 3) << "Should return count of 3";
}

// ========== CREATE USER TESTS ==========

TEST_F(UserServiceUnitTest, CreateUser_ValidData_CreatesUser) {
    using ::testing::Return;
    using ::testing::_;
    std::string jsonData = R"({"name":"New User","email":"new@example.com"})";
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("INSERT INTO users"), _))
        .WillOnce(Return(true));
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "4"}, {"name", "New User"}, {"email", "new@example.com"}, {"created_at", "2023-01-04"} }
    };
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("email = $1"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->createUser(jsonData);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("New User") != std::string::npos) << "Should return created user";
    EXPECT_TRUE(result.find("new@example.com") != std::string::npos) << "Should return user email";
}

TEST_F(UserServiceUnitTest, CreateUser_InvalidJson_ReturnsError) {
    std::string invalidJson = R"({"name":"New User","email":})"; // Invalid JSON
    // No DB call expected for invalid JSON
    std::string result = userService->createUser(invalidJson);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON error response";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("Validation failed") != std::string::npos) << "Should contain validation error message";
}

TEST_F(UserServiceUnitTest, CreateUser_MissingFields_ReturnsValidationError) {
    std::string incompleteJson = R"({"name":"New User"})"; // Missing email
    // No DB call expected for validation error
    std::string result = userService->createUser(incompleteJson);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON error response";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("Validation failed") != std::string::npos) << "Should contain validation error";
}

// ========== UPDATE USER TESTS ==========

TEST_F(UserServiceUnitTest, UpdateUser_ValidData_UpdatesUser) {
    using ::testing::Return;
    using ::testing::_;
    std::string jsonData = R"({"name":"Updated John","email":"updated.john@example.com"})";
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("UPDATE users"), _))
        .WillOnce(Return(true));
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "Updated John"}, {"email", "updated.john@example.com"}, {"created_at", "2023-01-01"} }
    };
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("id = $1"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->updateUser(1, jsonData);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("Updated John") != std::string::npos) << "Should return updated name";
    EXPECT_TRUE(result.find("updated.john@example.com") != std::string::npos) << "Should return updated email";
}

TEST_F(UserServiceUnitTest, UpdateUser_NonExistentId_ReturnsError) {
    using ::testing::Return;
    using ::testing::_;
    std::string jsonData = R"({"name":"Updated User","email":"updated@example.com"})";
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("UPDATE users"), _))
        .WillOnce(Return(false));
    std::vector<std::map<std::string, std::string>> emptyRows = {};
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("id = $1"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(emptyRows)));
    std::string result = userService->updateUser(999, jsonData);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";
}

TEST_F(UserServiceUnitTest, UpdateUser_PartialUpdate_UpdatesOnlyProvidedFields) {
    using ::testing::Return;
    using ::testing::_;
    std::string jsonData = R"({"name":"Only Name Updated"})"; // Only update name
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("UPDATE users"), _))
        .WillOnce(Return(true));
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "2"}, {"name", "Only Name Updated"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} }
    };
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("id = $1"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->updateUser(2, jsonData);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("Only Name Updated") != std::string::npos) << "Should return updated name";
    EXPECT_TRUE(result.find("jane@example.com") != std::string::npos) << "Should keep original email";
}

// ========== DELETE USER TESTS ==========

TEST_F(UserServiceUnitTest, DeleteUser_ValidId_DeletesUser) {
    using ::testing::Return;
    using ::testing::_;
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("DELETE FROM users"), _))
        .WillOnce(Return(true));
    std::string result = userService->deleteUser(1);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_TRUE(result.find("User deleted successfully") != std::string::npos) << "Should contain success message";
}

TEST_F(UserServiceUnitTest, DeleteUser_NonExistentId_ReturnsError) {
    using ::testing::Return;
    using ::testing::_;
    EXPECT_CALL(*mockDb, execCommand(::testing::HasSubstr("DELETE FROM users"), _))
        .WillOnce(Return(false));
    std::string result = userService->deleteUser(999);
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "false") << "Should indicate failure";
    EXPECT_TRUE(result.find("User not found") != std::string::npos) << "Should contain error message";
}

// ========== DATABASE ERROR TESTS ==========

TEST_F(UserServiceUnitTest, GetAllUsers_DatabaseConnection_ChecksConnection) {
    using ::testing::Return;
    using ::testing::_;
    // Disconnect the mock database
    mockDb->setConnectionStatus(false);
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}, {"created_at", "2023-01-02"} },
        { {"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}, {"created_at", "2023-01-03"} }
    };
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(mockRows)));
    std::string result = userService->getAllUsers();
    // Should still work as the database mock always returns data
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
}

// ========== EDGE CASES ==========

TEST_F(UserServiceUnitTest, GetAllUsers_EmptyDatabase_ReturnsEmptyArray) {
    using ::testing::Return;
    using ::testing::_;
    mockDb->clearUsers();
    std::vector<std::map<std::string, std::string>> emptyRows = {};
    EXPECT_CALL(*mockDb, execQuery("SELECT id, name, email, created_at FROM users ORDER BY id", _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(emptyRows)));
    std::string result = userService->getAllUsers();
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    EXPECT_EQ(countUsersInJson(result), 0) << "Should return empty array";
}

TEST_F(UserServiceUnitTest, GetUsersCount_ReturnsZero) {
    using ::testing::Return;
    using ::testing::_;
    mockDb->clearUsers();
    std::vector<std::map<std::string, std::string>> countRows = { { {"total", "0"} } };
    EXPECT_CALL(*mockDb, execQuery(::testing::HasSubstr("COUNT(*)"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockUserResultSet>(countRows)));
    std::string result = userService->getUsersCount();
    EXPECT_TRUE(isValidJson(result)) << "Should return valid JSON";
    EXPECT_EQ(extractJsonValue(result, "success"), "true") << "Should indicate success";
    rapidjson::Document doc;
    ASSERT_FALSE(doc.Parse(result.c_str()).HasParseError());
    ASSERT_TRUE(doc.HasMember("data"));
    ASSERT_TRUE(doc["data"].HasMember("count"));
    EXPECT_EQ(doc["data"]["count"].GetInt(), 0) << "Should return count of 0";
}
