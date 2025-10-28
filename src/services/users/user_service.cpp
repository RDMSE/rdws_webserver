#include "user_service.h"
#include "validation/schema_validator.h"
#include <json/json.h>


namespace rdws::users {

UserService::UserService(std::shared_ptr<rdws::database::IDatabase> db) : userRepository(db) {}

rdws::types::UsersResult UserService::getAllUsers() const {
    try {
        auto users = userRepository.findAll();
        return rdws::types::UsersResult::success(std::move(users));
    } catch (const std::exception& e) {
        const std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UsersResult::error(errorMsg, 500);
    }
}

rdws::types::UserResult UserService::getUserById(const int id) const {
    try {
        if (auto user = userRepository.findById(id); user.has_value()) {
            return rdws::types::UserResult::success(user.value());
        } else {
            return rdws::types::UserResult::error("User not found", 404);
        }
    } catch (const std::exception& e) {
        const std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UserResult::error(errorMsg, 500);
    }
}

rdws::types::CountResult UserService::getUsersCount() const {
    try {
        const auto count = userRepository.count();
        return rdws::types::CountResult::success(count);
    } catch (const std::exception& e) {
        const std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::CountResult::error(errorMsg, 500);
    }
}

rdws::types::UserResult UserService::createUser(const std::string& jsonData) const {
    try {
        auto validator = rdws::validation::UserValidators::createUserValidator();

        if (auto errors = validator.validate(jsonData); !errors.empty()) {
            // Return validation error with first error message
            std::string errorMsg = "Validation failed: " + errors[0].message;
            return rdws::types::UserResult::error(errorMsg, 400);
        }

        Json::Value json;
        if (Json::Reader reader; !reader.parse(jsonData, json)) {
            return rdws::types::UserResult::error("Invalid JSON format", 400);
        }

        if (rdws::types::User newUser(json["name"].asString(), json["email"].asString()); userRepository.create(newUser)) {
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

rdws::types::UserResult UserService::updateUser(int id, const std::string& jsonData) const {
    try {
        // Add id to JSON for validation
        Json::Value json;
        if (Json::Reader reader; !reader.parse(jsonData, json)) {
            return rdws::types::UserResult::error("Invalid JSON format", 400);
        }
        json["id"] = id;

        auto validator = rdws::validation::UserValidators::updateUserValidator();

        if (auto errors = validator.validate(json); !errors.empty()) {
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

        if (userRepository.update(updatedUser)) {
            return rdws::types::UserResult::success(updatedUser);
        } else {
            return rdws::types::UserResult::error("User not found or update failed", 404);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::UserResult::error(errorMsg, 500);
    }
}

rdws::types::OperationResult UserService::deleteUser(const int id) const {
    try {
        if (auto existingUser = userRepository.findById(id); !existingUser.has_value()) {
            const auto status = rdws::types::OperationStatus::createError("User not found", 404);
            return rdws::types::OperationResult::success(status);
        }

        const auto status = userRepository.deleteById(id) ?
            rdws::types::OperationStatus::createSuccess("User deleted successfully") :
            rdws::types::OperationStatus::createError("Failed to delete user", 500);

        return rdws::types::OperationResult::success(status);
    } catch (const std::exception& e) {
        const std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::types::OperationResult::error(errorMsg, 500);
    }
}

} // namespace rdws::users

