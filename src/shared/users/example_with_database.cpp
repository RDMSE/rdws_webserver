/**
 * Example: Using UserRepository in a C++ Microservice
 *
 * This shows how to use the UserRepository with database abstraction in your microservices
 */

#include "../common/config.h"
#include "../common/database/postgresql_database.h"
#include "../repository/user_repository.h"

#include <iostream>
#include <json/json.h>
#include <memory>

class UserService {
  private:
    rdws::Config db_config;
    std::shared_ptr<rdws::database::IDatabase> database;
    std::unique_ptr<rdws::repository::UserRepository> userRepository;

  public:
    UserService() : db_config() {
        std::cout << "Initialized with: " << db_config.getDebugInfo() << std::endl;

        // Initialize database connection
        database = std::make_shared<rdws::database::PostgreSQLDatabase>(db_config);

        // Initialize user repository
        userRepository = std::make_unique<rdws::repository::UserRepository>(database);
    }

    std::string getAllUsers() {
        try {
            // Use UserRepository with database abstraction
            auto users = userRepository->findAll();

            Json::Value usersJson(Json::arrayValue);
            for (const auto& user : users) {
                Json::Value userJson;
                userJson["id"] = user.id;
                userJson["name"] = user.name;
                userJson["email"] = user.email;
                userJson["created_at"] = user.created_at;
                usersJson.append(userJson);
            }

            Json::Value response;
            response["users"] = usersJson;
            response["total"] = static_cast<int>(users.size());
            response["source"] = "users_service C++ executable (with UserRepository)";
            response["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, response);
        } catch (const std::exception& e) {
            Json::Value error;
            error["error"] = "UserRepository operation failed: " + std::string(e.what());
            error["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, error);
        }
    }

    std::string getUserById(int id) {
        try {
            auto user = userRepository->findById(id);

            Json::Value response;
            if (user) {
                Json::Value userJson;
                userJson["id"] = user->id;
                userJson["name"] = user->name;
                userJson["email"] = user->email;
                userJson["created_at"] = user->created_at;

                response["user"] = userJson;
                response["found"] = true;
            } else {
                response["found"] = false;
                response["message"] = "User not found";
            }

            response["source"] = "users_service C++ executable (findById)";
            response["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, response);
        } catch (const std::exception& e) {
            Json::Value error;
            error["error"] = "UserRepository findById failed: " + std::string(e.what());
            error["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, error);
        }
    }

    std::string getUsersCount() {
        try {
            size_t count = userRepository->count();

            Json::Value response;
            response["count"] = static_cast<int>(count);
            response["source"] = "users_service C++ executable (count)";
            response["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, response);
        } catch (const std::exception& e) {
            Json::Value error;
            error["error"] = "UserRepository count failed: " + std::string(e.what());
            error["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, error);
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        UserService service;

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
        Json::Value response;
        response["error"] = "Usage: users_service <method> <path> [id]";
        response["examples"] = Json::Value(Json::arrayValue);
        response["examples"].append("users_service GET /users");
        response["examples"].append("users_service GET /users/ 1");
        response["examples"].append("users_service GET /users/count");

        Json::StreamWriterBuilder builder;
        std::cout << Json::writeString(builder, response) << std::endl;
        return 1;
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = "Service initialization failed: " + std::string(e.what());

        Json::StreamWriterBuilder builder;
        std::cout << Json::writeString(builder, error) << std::endl;
        return 1;
    }
}
