/**
 * Example: Using DatabaseConfig with .env files in a C++ Microservice
 * 
 * This shows how to use .env files directly in C++ microservices
 * Priority: .env files → JSON config → environment variables
 */

#include "common/database_config.h"
#include <pqxx/pqxx>
#include <iostream>
#include <json/json.h>

class UserService {
private:
    rdws::DatabaseConfig db_config;
    
public:
    UserService(const std::string& environment = "development") 
        : db_config(environment) {
        std::cout << "UserService initialized!" << std::endl;
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
            response["source"] = "users_service C++ executable with .env support";
            response["environment"] = db_config.getEnvironment();
            response["config_source"] = "dotenv-cpp";
            
            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, response);
            
        } catch (const std::exception& e) {
            Json::Value error;
            error["error"] = "Database connection failed: " + std::string(e.what());
            error["environment"] = db_config.getEnvironment();
            error["debug_info"] = db_config.getDebugInfo();
            
            Json::StreamWriterBuilder builder;
            return Json::writeString(builder, error);
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        // Determine environment from args or environment variable
        std::string environment = "development";
        
        // Check command line arguments
        for (int i = 0; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--env" && i + 1 < argc) {
                environment = argv[i + 1];
                break;
            }
        }
        
        // Override with environment variable if set
        if (std::getenv("ENVIRONMENT")) {
            environment = std::getenv("ENVIRONMENT");
        }
        
        std::cout << "Starting with environment: " << environment << std::endl;
        
        UserService service(environment);
        
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
        response["error"] = "Usage: users_service GET /users [--env development|production]";
        response["example"] = "users_service GET /users --env development";
        
        Json::StreamWriterBuilder builder;
        std::cout << Json::writeString(builder, response) << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = "Service initialization failed: " + std::string(e.what());
        error["hint"] = "Check if .env.development or .env.production exists, or if DB_* environment variables are set";
        
        Json::StreamWriterBuilder builder;
        std::cout << Json::writeString(builder, error) << std::endl;
        return 1;
    }
}