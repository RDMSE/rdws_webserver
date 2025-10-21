/**
 * Simple UserRepository test without heavy dependencies
 */

#include "../common/config.h"
#include <iostream>
#include <vector>
#include <string>

// Simple User struct for testing
struct SimpleUser {
    int id;
    std::string name;
    std::string email;
    
    SimpleUser(int id, const std::string& name, const std::string& email)
        : id(id), name(name), email(email) {}
};

// Mock UserRepository for testing
class MockUserRepository {
private:
    std::vector<SimpleUser> users;
    
public:
    MockUserRepository() {
        // Add some mock data
        users.emplace_back(1, "John Doe", "john@example.com");
        users.emplace_back(2, "Jane Smith", "jane@example.com");
        users.emplace_back(3, "Bob Wilson", "bob@example.com");
    }
    
    std::vector<SimpleUser> findAll() {
        return users;
    }
    
    SimpleUser* findById(int id) {
        for (auto& user : users) {
            if (user.id == id) {
                return &user;
            }
        }
        return nullptr;
    }
    
    size_t count() {
        return users.size();
    }
};

class SimpleUserService {
private:
    rdws::Config config;
    MockUserRepository repository;
    
public:
    SimpleUserService() : config() {
        std::cout << "Simple UserService initialized with: " << config.getDebugInfo() << std::endl;
    }
    
    std::string getAllUsers() {
        auto users = repository.findAll();
        
        std::string result = "{\n  \"users\": [\n";
        
        for (size_t i = 0; i < users.size(); ++i) {
            result += "    {\n";
            result += "      \"id\": " + std::to_string(users[i].id) + ",\n";
            result += "      \"name\": \"" + users[i].name + "\",\n";
            result += "      \"email\": \"" + users[i].email + "\"\n";
            result += "    }";
            if (i < users.size() - 1) {
                result += ",";
            }
            result += "\n";
        }
        
        result += "  ],\n";
        result += "  \"total\": " + std::to_string(users.size()) + ",\n";
        result += "  \"source\": \"simple_users_service C++ executable\",\n";
        result += "  \"environment\": \"" + config.getEnvironment() + "\"\n";
        result += "}";
        
        return result;
    }
    
    std::string getUserById(int id) {
        auto user = repository.findById(id);
        
        std::string result = "{\n";
        
        if (user) {
            result += "  \"user\": {\n";
            result += "    \"id\": " + std::to_string(user->id) + ",\n";
            result += "    \"name\": \"" + user->name + "\",\n";
            result += "    \"email\": \"" + user->email + "\"\n";
            result += "  },\n";
            result += "  \"found\": true\n";
        } else {
            result += "  \"found\": false,\n";
            result += "  \"message\": \"User not found\"\n";
        }
        
        result += "}";
        return result;
    }
    
    std::string getUsersCount() {
        size_t count = repository.count();
        
        std::string result = "{\n";
        result += "  \"count\": " + std::to_string(count) + ",\n";
        result += "  \"source\": \"simple_users_service C++ executable\",\n";
        result += "  \"environment\": \"" + config.getEnvironment() + "\"\n";
        result += "}";
        
        return result;
    }
};

int main(int argc, char* argv[]) {
    try {
        SimpleUserService service;

        if (argc >= 3) {
            std::string method = argv[1];
            std::string path = argv[2];

            if (method == "GET" && path == "/users") {
                std::cout << service.getAllUsers() << std::endl;
                return 0;
            } else if (method == "GET" && path.substr(0, 7) == "/users/" && argc >= 4) {
                // GET /users/{id}
                int userId = std::stoi(argv[3]);
                std::cout << service.getUserById(userId) << std::endl;
                return 0;
            } else if (method == "GET" && path == "/users/count") {
                std::cout << service.getUsersCount() << std::endl;
                return 0;
            }
        }

        // Default response
        std::cout << "{\n";
        std::cout << "  \"error\": \"Usage: simple_users_service <method> <path> [id]\",\n";
        std::cout << "  \"examples\": [\n";
        std::cout << "    \"simple_users_service GET /users\",\n";
        std::cout << "    \"simple_users_service GET /users/ 1\",\n";
        std::cout << "    \"simple_users_service GET /users/count\"\n";
        std::cout << "  ]\n";
        std::cout << "}" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "{\n";
        std::cout << "  \"error\": \"Service initialization failed: " << e.what() << "\"\n";
        std::cout << "}" << std::endl;
        return 1;
    }
}