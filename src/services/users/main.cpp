#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <sstream>
#include "../../shared/common/database/postgresql_database.h"
#include "../../shared/repository/user_repository.h"
#include "../../shared/types/user.h"
#include "../../shared/validation/schema_validator.h"

using namespace rdws::types;
using namespace rdws::database;
using namespace rdws::repository;
using namespace rdws::validation;

class UserService {
private:
    UserRepository userRepository;

public:
    UserService(std::shared_ptr<IDatabase> db) : userRepository(db) {}
    
    std::string getAllUsers() {
        try {
            auto users = userRepository.findAll();
            std::ostringstream oss;
            oss << "{\"users\":[";
            
            for (size_t i = 0; i < users.size(); ++i) {
                oss << users[i].toJsonString();
                if (i < users.size() - 1) {
                    oss << ",";
                }
            }
            
            oss << "],\"total\":" << users.size() << ","
                << "\"source\":\"users_service C++ with PostgreSQL\","
                << "\"endpoint\":\"/users\","
                << "\"timestamp\":" << std::time(nullptr) << "}";
            
            return oss.str();
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
    
    std::string getUserById(int id) {
        try {
            auto user = userRepository.findById(id);
            if (user.has_value()) {
                std::ostringstream oss;
                oss << "{\"user\":" << user->toJsonString() << ","
                    << "\"source\":\"users_service C++ with PostgreSQL\","
                    << "\"endpoint\":\"/users/" << id << "\","
                    << "\"timestamp\":" << std::time(nullptr) << "}";
                return oss.str();
            } else {
                return "{\"error\":\"User not found\",\"userId\":" + std::to_string(id) + "}";
            }
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
    
    std::string getUsersCount() {
        try {
            auto count = userRepository.count();
            std::ostringstream oss;
            oss << "{\"count\":" << count << ","
                << "\"source\":\"users_service C++ with PostgreSQL\","
                << "\"endpoint\":\"/users/count\","
                << "\"timestamp\":" << std::time(nullptr) << "}";
            return oss.str();
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
    
    std::string createUser(const std::string& jsonData) {
        try {
            auto schema = UserSchemas::createUserSchema();
            SchemaValidator validator("create_user", schema);
            auto errors = validator.validate(jsonData);
            
            if (!errors.empty()) {
                return validator.getErrorsAsJson(errors);
            }
            
            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(jsonData, json)) {
                return "{\"error\":\"Invalid JSON format\"}";
            }
            
            User newUser(json["name"].asString(), json["email"].asString());
            bool success = userRepository.create(newUser);
            
            if (success) {
                // Get the created user (assuming it's the last one with the same email)
                auto users = userRepository.findAll();
                User* createdUser = nullptr;
                for (auto& user : users) {
                    if (user.email == newUser.email && user.name == newUser.name) {
                        createdUser = &user;
                        break;
                    }
                }
                
                if (createdUser) {
                    std::ostringstream oss;
                    oss << "{\"message\":\"User created successfully\","
                        << "\"user\":" << createdUser->toJsonString() << ","
                        << "\"source\":\"users_service C++ with PostgreSQL\","
                        << "\"timestamp\":" << std::time(nullptr) << "}";
                    return oss.str();
                } else {
                    return "{\"error\":\"User created but could not retrieve details\"}";
                }
            } else {
                return "{\"error\":\"Failed to create user\"}";
            }
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
    
    std::string updateUser(int id, const std::string& jsonData) {
        try {
            // Add id to JSON for validation
            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(jsonData, json)) {
                return "{\"error\":\"Invalid JSON format\"}";
            }
            json["id"] = id;
            
            auto schema = UserSchemas::updateUserSchema();
            SchemaValidator validator("update_user", schema);
            auto errors = validator.validate(json);
            
            if (!errors.empty()) {
                return validator.getErrorsAsJson(errors);
            }
            
            // Get existing user
            auto existingUser = userRepository.findById(id);
            if (!existingUser.has_value()) {
                return "{\"error\":\"User not found\",\"userId\":" + std::to_string(id) + "}";
            }
            
            // Update fields if provided
            User updatedUser = existingUser.value();
            if (json.isMember("name")) {
                updatedUser.name = json["name"].asString();
            }
            if (json.isMember("email")) {
                updatedUser.email = json["email"].asString();
            }
            
            bool success = userRepository.update(updatedUser);
            if (success) {
                std::ostringstream oss;
                oss << "{\"message\":\"User updated successfully\","
                    << "\"user\":" << updatedUser.toJsonString() << ","
                    << "\"source\":\"users_service C++ with PostgreSQL\","
                    << "\"timestamp\":" << std::time(nullptr) << "}";
                return oss.str();
            } else {
                return "{\"error\":\"Failed to update user\"}";
            }
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
    
    std::string deleteUser(int id) {
        try {
            auto existingUser = userRepository.findById(id);
            if (!existingUser.has_value()) {
                return "{\"error\":\"User not found\",\"userId\":" + std::to_string(id) + "}";
            }
            
            bool success = userRepository.deleteById(id);
            if (success) {
                std::ostringstream oss;
                oss << "{\"message\":\"User deleted successfully\","
                    << "\"userId\":" << id << ","
                    << "\"source\":\"users_service C++ with PostgreSQL\","
                    << "\"timestamp\":" << std::time(nullptr) << "}";
                return oss.str();
            } else {
                return "{\"error\":\"Failed to delete user\"}";
            }
        } catch (const std::exception& e) {
            return "{\"error\":\"Database error: " + std::string(e.what()) + "\"}";
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        // Initialize database connection
        auto db = std::make_shared<PostgreSQLDatabase>();
        if (!db->isConnected()) {
            std::cerr << "{\"error\":\"Failed to connect to database\"}" << std::endl;
            return 1;
        }
        
        // Initialize user service
        UserService userService(db);
        
        // Process command line arguments
        std::string method = "GET";
        std::string path = "/users";
        
        if (argc > 1) {
            method = argv[1];
        }
        if (argc > 2) {
            path = argv[2];
        }
        
        // Process request
        if (method == "GET") {
            if (path == "/users" || path == "/") {
                // List all users
                std::cout << userService.getAllUsers() << std::endl;
                return 0;
            } else if (path.find("/users/") == 0) {
                // Fetch specific user
                try {
                    std::string idStr = path.substr(7); // Remove "/users/"
                    if (idStr == "count") {
                        std::cout << userService.getUsersCount() << std::endl;
                        return 0;
                    }
                    
                    int userId = std::stoi(idStr);
                    std::cout << userService.getUserById(userId) << std::endl;
                    return 0;
                } catch (const std::exception& e) {
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}" << std::endl;
                    return 1;
                }
            }
        } else if (method == "POST") {
            if (path == "/users" || path == "/") {
                // Create user - expect JSON data from stdin or args
                std::string jsonData;
                if (argc > 3) {
                    jsonData = argv[3];
                } else {
                    // Read from stdin
                    std::string line;
                    while (std::getline(std::cin, line)) {
                        jsonData += line;
                    }
                }
                
                if (jsonData.empty()) {
                    std::cout << "{\"error\":\"No JSON data provided for user creation\"}" << std::endl;
                    return 1;
                }
                
                std::cout << userService.createUser(jsonData) << std::endl;
                return 0;
            }
        } else if (method == "PUT") {
            if (path.find("/users/") == 0) {
                try {
                    std::string idStr = path.substr(7); // Remove "/users/"
                    int userId = std::stoi(idStr);
                    
                    std::string jsonData;
                    if (argc > 3) {
                        jsonData = argv[3];
                    } else {
                        // Read from stdin
                        std::string line;
                        while (std::getline(std::cin, line)) {
                            jsonData += line;
                        }
                    }
                    
                    if (jsonData.empty()) {
                        std::cout << "{\"error\":\"No JSON data provided for user update\"}" << std::endl;
                        return 1;
                    }
                    
                    std::cout << userService.updateUser(userId, jsonData) << std::endl;
                    return 0;
                } catch (const std::exception& e) {
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}" << std::endl;
                    return 1;
                }
            }
        } else if (method == "DELETE") {
            if (path.find("/users/") == 0) {
                try {
                    std::string idStr = path.substr(7); // Remove "/users/"
                    int userId = std::stoi(idStr);
                    
                    std::cout << userService.deleteUser(userId) << std::endl;
                    return 0;
                } catch (const std::exception& e) {
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}" << std::endl;
                    return 1;
                }
            }
        }
        
        // Method not supported
        std::cout << "{\"error\":\"Method not allowed\",\"method\":\"" << method << "\",\"path\":\"" << path << "\"}" << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        std::cerr << "{\"error\":\"Service error: " << e.what() << "\"}" << std::endl;
        return 1;
    }
}
