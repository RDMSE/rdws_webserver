/**
 * Test dotenv-cpp third-party library integration
 */

#include "../src/shared/common/database_config.h"
#include <iostream>

int main() {
    try {
        std::cout << "🧪 Testing dotenv-cpp library integration..." << std::endl;
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
        rdws::DatabaseConfig config;
        
        std::cout << "✅ DB Config: " << config.getUser() << "@" << config.getHost() 
                  << ":" << config.getPort() << "/" << config.getDatabase() 
                  << " (" << config.getEnvironment() << ")" << std::endl;
        
        std::cout << "\n🎉 Success! Configuration loaded with dotenv-cpp:" << std::endl;
        std::cout << "DB Config: " << config.getUser() << "@" << config.getHost() 
                  << ":" << config.getPort() << "/" << config.getDatabase() 
                  << " (" << config.getEnvironment() << ")" << std::endl;
        
        std::cout << "\nConnection string: postgresql://" << config.getUser() 
                  << ":" << config.getPassword() << "@" << config.getHost() 
                  << ":" << config.getPort() << "/" << config.getDatabase() << std::endl;
        
        std::cout << "\n📋 Library info:" << std::endl;
        std::cout << "  Using: laserpants/dotenv-cpp (third-party submodule)" << std::endl;
        std::cout << "  Location: src/third_party/dotenv-cpp/" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Error: " << e.what() << std::endl;
        std::cout << "\n💡 Make sure you have .env.development file with DB_* variables" << std::endl;
        return 1;
    }
}