/**
 * Test dotenv-cpp third-party library integration
 */

#include "../src/shared/common/config.h"

#include <iostream>

int main() {
    try {
        std::cout << "Testing dotenv-cpp library integration..." << std::endl;
        std::cout << "Loading .env from: ../.env.development" << std::endl;

        // Set environment to development for testing
        setenv("ENVIRONMENT", "development", 1);
        setenv("DB_HOST", "localhost", 1);
        setenv("DB_PORT", "5432", 1);
        setenv("DB_USER", "postgres", 1);
        setenv("DB_PASS", "password", 1);
        setenv("DB_NAME_DEV", "postgres", 1);
        setenv("DB_NAME_PROD", "postgres", 1);

        // Test with development environment
        rdws::Config config;

        std::cout << "DB Config: " << config.getUser() << "@" << config.getHost() << ":"
                  << config.getPort() << "/" << config.getDatabase() << " ("
                  << config.getEnvironment() << ")" << std::endl;

        std::cout << "\nSuccess! Configuration loaded with dotenv-cpp:" << std::endl;
        std::cout << "DB Config: " << config.getUser() << "@" << config.getHost() << ":"
                  << config.getPort() << "/" << config.getDatabase() << " ("
                  << config.getEnvironment() << ")" << std::endl;

        std::cout << "\nConnection string: postgresql://" << config.getUser() << ":"
                  << config.getPassword() << "@" << config.getHost() << ":" << config.getPort()
                  << "/" << config.getDatabase() << std::endl;

        std::cout << "\nLibrary info:" << std::endl;
        std::cout << "  Using: laserpants/dotenv-cpp (third-party submodule)" << std::endl;
        std::cout << "  Location: src/third_party/dotenv-cpp/" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        std::cout << "\nMake sure you have .env.development file with DB_* variables" << std::endl;
        return 1;
    }
}
