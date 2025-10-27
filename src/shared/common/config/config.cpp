#include "config.h"
#include <sstream>
#include <iostream>
#include <fstream>

namespace rdws {

Config::Config() {
    loadEnvironmentVariables();
}

std::string Config::get(const std::string& key, const std::string& defaultValue) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return defaultValue;
}

void Config::set(const std::string& key, const std::string& value) {
    settings[key] = value;
}

std::string Config::getDatabaseHost() const {
    return get("DB_HOST", "fedora-server.local");
}

std::string Config::getDatabasePort() const {
    return get("DB_PORT", "5432");
}

std::string Config::getDatabaseName() const {
    std::string dbName = get("DB_NAME", "");
    if (dbName.empty()) {
        std::string env = getEnvironment();
        if (env == "production") {
            return "rdws_production";
        } else {
            return "rdws_development";
        }
    }
    return dbName;
}

std::string Config::getDatabaseUser() const {
    return get("DB_USER", "rdws_user");
}

std::string Config::getDatabasePassword() const {
    return get("DB_PASSWORD", "rdws_pass123");
}

std::string Config::getConnectionString() const {
    std::ostringstream oss;
    oss << "host=" << getDatabaseHost()
        << " port=" << getDatabasePort()
        << " dbname=" << getDatabaseName()
        << " user=" << getDatabaseUser()
        << " password=" << getDatabasePassword();
    return oss.str();
}

std::string Config::getEnvironment() const {
    return get("RDWS_ENVIRONMENT", "development");
}

bool Config::isDevelopment() const {
    return getEnvironment() == "development";
}

bool Config::isProduction() const {
    return getEnvironment() == "production";
}

void Config::loadEnvironmentVariables() {
    // Load from environment variables
    settings["RDWS_ENVIRONMENT"] = getEnvVar("RDWS_ENVIRONMENT", "test");
    settings["DB_PORT"] = getEnvVar("DB_PORT", "1234");
    settings["DB_HOST"] = getEnvVar("DB_HOST", "test-server");
    settings["DB_USER"] = getEnvVar("DB_USER", "db_user");
    settings["DB_PASSWORD"] = getEnvVar("DB_PASSWORD", "db_psswd");
    settings["DB_NAME"] = getEnvVar("DB_NAME", ""); // Will be set by getDatabaseName()
    
    // Try to load environment-specific .env file
    std::string env = getEnvironment();
    std::string envFile = ".env." + env;
    loadEnvFile(envFile);
    
    // Also try generic .env file
    loadEnvFile(".env");
}

std::string Config::getEnvVar(const std::string& name, const std::string& defaultValue) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
}

void Config::loadEnvFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return; // File doesn't exist, that's ok
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Find the = separator
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Remove quotes if present
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            // Only override if not already set by environment variables
            if (settings.find(key) == settings.end() || settings[key].empty()) {
                settings[key] = value;
            }
        }
    }
}

} // namespace rdws