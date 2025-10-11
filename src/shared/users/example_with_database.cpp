/**
 * Example: Using DatabaseConfig in a C++ Microservice
 *
 * This shows how to use the database configuration in your microservices
 */

#include "common/config.h"

#include <iostream>
#include <json/json.h>
#include <pqxx/pqxx>

class UserService {
  private:
    rdws::DatabaseConfig db_config;

  public:
    UserService() : db_config() {
        std::cout << "Initialized with: " << db_config.getDebugInfo() << std::endl;
    }

    std::string getAllUsers() {
        try {
            // Use libpqxx with our configuration
            pqxx::connection conn(db_config.getConnectionString());
            pqxx::work txn{conn};

            auto result = txn.exec("SELECT id, name, email FROM users ORDER BY id");

            Json::Value users(Json::arrayValue);
            for (auto row : result) {
                Json::Value user;
                user["id"] = row["id"].as<int>();
                user["name"] = row["name"].as<std::string>();
                user["email"] = row["email"].as<std::string>();
                users.append(user);
            }

            Json::Value response;
            response["users"] = users;
            response["total"] = static_cast<int>(result.size());
            response["source"] = "users_service C++ executable";
            response["environment"] = db_config.getEnvironment();

            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, response);
        } catch (const std::exception& e) {
            Json::Value error;
            error["error"] = "Database connection failed: " + std::string(e.what());
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
            }
        }

        // Default response
        Json::Value response;
        response["error"] = "Usage: users_service GET /users";

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
