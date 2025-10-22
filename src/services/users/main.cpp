#include "../../shared/common/database/postgresql_database.h"
#include "user_service.h"

#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

using namespace rdws::types;
using namespace rdws::database;
using namespace rdws::users;

int main(int argc, char* argv[]) {
    try {
        // Initialize database connection
        auto db = std::make_shared<rdws::database::PostgreSQLDatabase>();
        if (!db->isConnected()) {
            std::cerr << "{\"error\":\"Failed to connect to database\"}" << std::endl;
            return 1;
        }

        // Initialize user service
        rdws::users::UserService userService(db);

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
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}"
                              << std::endl;
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
                    std::cout << "{\"error\":\"No JSON data provided for user creation\"}"
                              << std::endl;
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
                        std::cout << "{\"error\":\"No JSON data provided for user update\"}"
                                  << std::endl;
                        return 1;
                    }

                    std::cout << userService.updateUser(userId, jsonData) << std::endl;
                    return 0;
                } catch (const std::exception& e) {
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}"
                              << std::endl;
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
                    std::cout << "{\"error\":\"Invalid user ID\",\"path\":\"" << path << "\"}"
                              << std::endl;
                    return 1;
                }
            }
        }

        // Method not supported
        std::cout << "{\"error\":\"Method not allowed\",\"method\":\"" << method << "\",\"path\":\""
                  << path << "\"}" << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "{\"error\":\"Service error: " << e.what() << "\"}" << std::endl;
        return 1;
    }
}
