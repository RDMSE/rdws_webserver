#include "user_service.h"
#include "../../shared/validation/schema_validator.h"
#include <rapidjson/document.h>
#include <jsoncpp/json/json.h>

namespace rdws {
namespace users {

UserService::UserService(std::shared_ptr<rdws::database::IDatabase> db) 
    : userRepository(db) {}

std::string UserService::getAllUsers() {
    try {
        auto users = userRepository.findAll();
        return rdws::utils::ResponseHelper::returnEntities(users, "users");
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

std::string UserService::getUserById(int id) {
    try {
        auto user = userRepository.findById(id);
        
        if (user.has_value()) {
            return rdws::utils::ResponseHelper::returnEntity(user.value(), "user");
        } else {
            return rdws::utils::ResponseHelper::returnError("User not found", 404);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

std::string UserService::getUsersCount() {
    try {
        auto count = userRepository.count();
        
        ::rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
        
        ::rapidjson::Value countData(::rapidjson::kObjectType);
        countData.AddMember("count", ::rapidjson::Value(static_cast<int>(count)), allocator);
        
        return rdws::utils::ResponseHelper::returnData(countData);
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

std::string UserService::createUser(const std::string& jsonData) {
    try {
        auto validator = rdws::validation::UserValidators::createUserValidator();
        auto errors = validator.validate(jsonData);

        if (!errors.empty()) {
            // Convert validation errors to our standard format
            ::rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();
            
            ::rapidjson::Value errorsArray(::rapidjson::kArrayType);
            for (const auto& error : errors) {
                ::rapidjson::Value errorObj(::rapidjson::kObjectType);
                errorObj.AddMember("field", ::rapidjson::Value(error.field.c_str(), allocator), allocator);
                errorObj.AddMember("message", ::rapidjson::Value(error.message.c_str(), allocator), allocator);
                if (!error.context.empty()) {
                    errorObj.AddMember("context", ::rapidjson::Value(error.context.c_str(), allocator), allocator);
                }
                errorsArray.PushBack(errorObj, allocator);
            }
            
            return rdws::utils::ResponseHelper::returnError("Validation failed", 400, &errorsArray);
        }

        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(jsonData, json)) {
            return rdws::utils::ResponseHelper::returnError("Invalid JSON format", 400);
        }

        rdws::types::User newUser(json["name"].asString(), json["email"].asString());
        bool success = userRepository.create(newUser);

        if (success) {
            // Get the created user (assuming it's the last one with the same email)
            auto users = userRepository.findAll();
            rdws::types::User* createdUser = nullptr;
            for (auto& user : users) {
                if (user.email == newUser.email && user.name == newUser.name) {
                    createdUser = &user;
                    break;
                }
            }

            if (createdUser) {
                return rdws::utils::ResponseHelper::returnEntity(*createdUser, "user", "User created successfully", 201);
            } else {
                return rdws::utils::ResponseHelper::returnError("User created but could not retrieve details", 500);
            }
        } else {
            return rdws::utils::ResponseHelper::returnError("Failed to create user", 500);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

std::string UserService::updateUser(int id, const std::string& jsonData) {
    try {
        // Add id to JSON for validation
        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(jsonData, json)) {
            return rdws::utils::ResponseHelper::returnError("Invalid JSON format", 400);
        }
        json["id"] = id;

        auto validator = rdws::validation::UserValidators::updateUserValidator();
        auto errors = validator.validate(json);

        if (!errors.empty()) {
            // Convert validation errors to our standard format
            ::rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();
            
            ::rapidjson::Value errorsArray(::rapidjson::kArrayType);
            for (const auto& error : errors) {
                ::rapidjson::Value errorObj(::rapidjson::kObjectType);
                errorObj.AddMember("field", ::rapidjson::Value(error.field.c_str(), allocator), allocator);
                errorObj.AddMember("message", ::rapidjson::Value(error.message.c_str(), allocator), allocator);
                if (!error.context.empty()) {
                    errorObj.AddMember("context", ::rapidjson::Value(error.context.c_str(), allocator), allocator);
                }
                errorsArray.PushBack(errorObj, allocator);
            }
            
            return rdws::utils::ResponseHelper::returnError("Validation failed", 400, &errorsArray);
        }

        // Get existing user
        auto existingUser = userRepository.findById(id);
        
        if (!existingUser.has_value()) {
            return rdws::utils::ResponseHelper::returnError("User not found", 404);
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
            return rdws::utils::ResponseHelper::returnEntity(updatedUser, "user", "User updated successfully");
        } else {
            return rdws::utils::ResponseHelper::returnError("Failed to update user", 500);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

std::string UserService::deleteUser(int id) {
    try {
        auto existingUser = userRepository.findById(id);
        
        if (!existingUser.has_value()) {
            return rdws::utils::ResponseHelper::returnError("User not found", 404);
        }

        bool success = userRepository.deleteById(id);
        if (success) {
            ::rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();
            
            ::rapidjson::Value deleteData(::rapidjson::kObjectType);
            deleteData.AddMember("userId", ::rapidjson::Value(id), allocator);
            
            return rdws::utils::ResponseHelper::returnData(deleteData, "User deleted successfully");
        } else {
            return rdws::utils::ResponseHelper::returnError("Failed to delete user", 500);
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Database error: " + std::string(e.what());
        return rdws::utils::ResponseHelper::returnError(errorMsg, 500);
    }
}

} // namespace users
} // namespace rdws