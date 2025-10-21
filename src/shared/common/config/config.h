#pragma once

#include <string>
#include <map>
#include <cstdlib>

namespace rdws {

class Config {
private:
    std::map<std::string, std::string> settings;
    
public:
    Config();
    
    // Get configuration value
    std::string get(const std::string& key, const std::string& defaultValue = "") const;
    
    // Set configuration value
    void set(const std::string& key, const std::string& value);
    
    // Database configuration
    std::string getDatabaseHost() const;
    std::string getDatabasePort() const;
    std::string getDatabaseName() const;
    std::string getDatabaseUser() const;
    std::string getDatabasePassword() const;
    std::string getConnectionString() const;
    
    // Environment detection
    std::string getEnvironment() const;
    bool isDevelopment() const;
    bool isProduction() const;
    
private:
    void loadEnvironmentVariables();
    void loadEnvFile(const std::string& filename);
    std::string getEnvVar(const std::string& name, const std::string& defaultValue = "") const;
};

} // namespace rdws