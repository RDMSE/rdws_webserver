#include "common/database/postgresql_database.h"
#include "types/lambda_event.h"
#include "types/lambda_context.h"
#include "controllers/user_controller.h"
#include "user_service.h"

#include <iostream>
#include <memory>
#include <string>

using namespace rdws::types;
using namespace rdws::database;
using namespace rdws::users;
using namespace rdws::controllers;

int main(int argc, char* argv[]) {
    try {
        LambdaEvent event("GET", "/", ""); // Initialize with defaults
        LambdaContext context("unknown", "users-service"); // Initialize with defaults

        // Check if we have JSON parameters (new API Gateway approach)
        if (argc >= 3) {
            // New approach: API Gateway passes JSON strings
            std::string eventJson = argv[1];
            std::string contextJson = argv[2];

            try {
                event = LambdaEvent::fromJson(eventJson);
                context = LambdaContext::fromJson(contextJson);
            } catch (...) {
                // Fallback to old approach if JSON parsing fails
                event = LambdaEvent(argc, argv);
                context = LambdaContext(event.getRequestContext().requestId, "users-service");
            }
        } else {
            // Fallback to old approach for backwards compatibility
            event = LambdaEvent(argc, argv);
            context = LambdaContext(event.getRequestContext().requestId, "users-service");
        }

        context.log("Function started", "INFO");

        // Initialize database connection
        auto db = std::make_shared<rdws::database::PostgreSQLDatabase>();
        if (!db->isConnected()) {
            context.log("Failed to connect to database", "ERROR");
            std::cerr << UserController::formatDatabaseError() << std::endl;
            return 1;
        }

        // Initialize user service
        rdws::users::UserService userService(db);

        // Extract path parameters for routes like /users/{id}
        if (event.pathMatches("/users/{id}") || event.pathMatches("/users/{action}")) {
            event.extractPathParameters("/users/{id}");
        }

        context.log("Processing " + event.getHttpMethod() + " request to " + event.getPath(), "INFO");

        // Process request based on method and path
        if (event.isGet()) {
            if (event.pathMatches("/users") || event.pathMatches("/")) {
                // List all users
                context.log("Fetching all users", "INFO");
                auto result = userService.getAllUsers();
                std::cout << UserController::formatUsersResponse(result) << std::endl;
                return 0;
            } else if (event.pathMatches("/users/{id}")) {
                // Fetch specific user or handle special actions
                std::string idParam = event.getPathParameter("id");

                if (idParam == "count") {
                    context.log("Getting user count", "INFO");
                    auto result = userService.getUsersCount();
                    std::cout << UserController::formatCountResponse(result) << std::endl;
                    return 0;
                }

                try {
                    int userId = std::stoi(idParam);
                    context.log("Fetching user with ID: " + std::to_string(userId), "INFO");
                    auto result = userService.getUserById(userId);
                    std::cout << UserController::formatUserResponse(result) << std::endl;
                    return 0;
                } catch (...) {
                    context.log("Invalid user ID: " + idParam, "ERROR");
                    std::cout << R"({"error":"Invalid user ID","path":")" << event.getPath() << "\"}"
                              << std::endl;
                    return 1;
                }
            }
        } else if (event.isPost()) {
            if (event.pathMatches("/users") || event.pathMatches("/")) {
                // Create user
                const std::string& jsonData = event.getBody();

                if (jsonData.empty()) {
                    context.log("No JSON data provided for user creation", "ERROR");
                    std::cout << UserController::formatNoDataProvidedError("user creation") << std::endl;
                    return 1;
                }

                context.log("Creating new user", "INFO");
                auto result = userService.createUser(jsonData);
                std::cout << UserController::formatUserResponse(result) << std::endl;
                return 0;
            }
        } else if (event.isPut()) {
            if (event.pathMatches("/users/{id}")) {
                std::string idParam = event.getPathParameter("id");

                try {
                    int userId = std::stoi(idParam);
                    const std::string& jsonData = event.getBody();

                    if (jsonData.empty()) {
                        context.log("No JSON data provided for user update", "ERROR");
                        std::cout << UserController::formatNoDataProvidedError("user update") << std::endl;
                        return 1;
                    }

                    context.log("Updating user with ID: " + std::to_string(userId), "INFO");
                    auto result = userService.updateUser(userId, jsonData);
                    std::cout << UserController::formatUserResponse(result) << std::endl;
                    return 0;
                } catch (...) {
                    context.log("Invalid user ID: " + idParam, "ERROR");
                    std::cout << UserController::formatError("Invalid user ID", 400) << std::endl;
                    return 1;
                }
            }
        } else if (event.isDelete()) {
            if (event.pathMatches("/users/{id}")) {
                std::string idParam = event.getPathParameter("id");

                try {
                    int userId = std::stoi(idParam);
                    context.log("Deleting user with ID: " + std::to_string(userId), "INFO");
                    auto result = userService.deleteUser(userId);
                    std::cout << UserController::formatOperationResponse(result) << std::endl;
                    return 0;
                } catch (...) {
                    context.log("Invalid user ID: " + idParam, "ERROR");
                    std::cout << UserController::formatError("Invalid user ID", 400) << std::endl;
                    return 1;
                }
            }
        }

        // Method not supported
        context.log("Method not allowed: " + event.getHttpMethod() + " " + event.getPath(), "WARN");
        std::cout << UserController::formatMethodNotAllowedError(event.getHttpMethod(), event.getPath()) << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << UserController::formatServiceError(e.what()) << std::endl;
        return 1;
    }
}
