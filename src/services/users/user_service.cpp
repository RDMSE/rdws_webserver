#include "user_service.h"
#include "validation/schema_validator.h"
#include <json/json.h>
#include <rapidjson/document.h>

namespace rdws {
namespace users {

UserService::UserService(std::shared_ptr<rdws::database::IDatabase> db) : userRepository(db) {}

rdws::types::UsersResult UserService::getAllUsers() {
    try {
        auto users = userRepository.findAll();
        return rdws::types::UsersResult::success(std::move(users));
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UsersResult::error(errorMsg, 500);
    }
}

rdws::types::UserResult UserService::getUserById(int id) {
    try {
        auto user = userRepository.findById(id);

        if (user.has_value()) {
            return rdws::types::UserResult::success(user.value());
        } else {
            return rdws::types::UserResult::error("User not found", 404);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UserResult::error(errorMsg, 500);
    }
}

rdws::types::CountResult UserService::getUsersCount() {
    try {
        auto count = userRepository.count();
        return rdws::types::CountResult::success(count);
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::CountResult::error(errorMsg, 500);
    }
}

rdws::types::UserResult UserService::createUser(const std::string& jsonData) {
    try {
        auto validator = rdws::validation::UserValidators::createUserValidator();
        auto errors = validator.validate(jsonData);

        if (!errors.empty()) {
            // Return validation error with first error message
            std::string errorMsg = "Validation failed: " + errors[0].message;
            return rdws::types::UserResult::error(errorMsg, 400);
        }

        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(jsonData, json)) {
            return rdws::types::UserResult::error("Invalid JSON format", 400);
        }

        rdws::types::User newUser(json["name"].asString(), json["email"].asString());
        bool success = userRepository.create(newUser);

        if (success) {
            // Get the created user (assuming it's the last one with the same email)
            auto users = userRepository.findAll();
            for (auto& user : users) {
                if (user.email == newUser.email && user.name == newUser.name) {
                    return rdws::types::UserResult::success(user);
                }
            }
            return rdws::types::UserResult::error("User created but could not retrieve details", 500);
        } else {
            return rdws::types::UserResult::error("Failed to create user", 500);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UserResult::error(errorMsg, 500);
    }
}

rdws::types::UserResult UserService::updateUser(int id, const std::string& jsonData) {
    try {
        // Add id to JSON for validation
        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(jsonData, json)) {
            return rdws::types::UserResult::error("Invalid JSON format", 400);
        }
        json["id"] = id;

        auto validator = rdws::validation::UserValidators::updateUserValidator();
        auto errors = validator.validate(json);

        if (!errors.empty()) {
            std::string errorMsg = "Validation failed: " + errors[0].message;
            return rdws::types::UserResult::error(errorMsg, 400);
        }

        // Get existing user
        auto existingUser = userRepository.findById(id);

        if (!existingUser.has_value()) {
            return rdws::types::UserResult::error("User not found", 404);
        }

        // Update fields if provided
        rdws::types::User updatedUser = existingUser.value();
        if (json.isMember("name")) {
            updatedUser.name = json["name"].asString();
        }
        if (json.isMember("email")) {
            updatedUser.email = json["email"].asString();
        }

        bool success = userRepository.update(updatedUser);
        if (success) {
            return rdws::types::UserResult::success(updatedUser);
        } else {
            return rdws::types::UserResult::error("User not found or update failed", 404);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UserResult::error(errorMsg, 500);
    }
}

rdws::types::OperationResult UserService::deleteUser(int id) {
    try {
        auto existingUser = userRepository.findById(id);

        if (!existingUser.has_value()) {
            auto status = rdws::types::OperationStatus::createError("User not found", 404);
            return rdws::types::OperationResult::success(status);
        }

        bool success = userRepository.deleteById(id);
        if (success) {
            auto status = rdws::types::OperationStatus::createSuccess("User deleted successfully");
            return rdws::types::OperationResult::success(status);
        } else {
            auto status = rdws::types::OperationStatus::createError("Failed to delete user", 500);
            return rdws::types::OperationResult::success(status);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::OperationResult::error(errorMsg, 500);
    }
}

} // namespace users
} // namespace rdws
