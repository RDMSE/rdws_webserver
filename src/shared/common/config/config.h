#pragma once

#include <map>
#include <optional>
#include <string>

namespace rdws {

class Config {
  private:
    std::map<std::string, std::string> settings;

  public:
    Config();

    // Get configuration value
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const;

    // Set configuration value
    void set(const std::string& key, const std::string& value);

    // Database configuration
    [[nodiscard]] std::string getDatabaseHost() const;
    [[nodiscard]] std::string getDatabasePort() const;
    [[nodiscard]] std::string getDatabaseName() const;
    [[nodiscard]] std::string getDatabaseUser() const;
    [[nodiscard]] std::string getDatabasePassword() const;
    [[nodiscard]] std::string getConnectionString() const;

    // Environment detection
    [[nodiscard]] std::string getEnvironment() const;
    [[nodiscard]] bool isDevelopment() const;
    [[nodiscard]] bool isProduction() const;

  private:
    void loadEnvironmentVariables();
    static void loadEnvFile(const std::string& filename);
    [[nodiscard]] static std::optional<std::string> getEnvVar(const std::string& name);
};

} // namespace rdws
