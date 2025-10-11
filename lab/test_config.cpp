#include <iostream>
#include <cstdlib>
#include "../src/shared/common/config.h"

int main()
{
    try
    {
        // Test development environment
        setenv("ENVIRONMENT", "development", 1);
        setenv("DB_HOST", "localhost", 1);
        setenv("DB_PORT", "5432", 1);
        setenv("DB_USER", "postgres", 1);
        setenv("DB_PASS", "password", 1);
        setenv("DB_NAME_DEV", "rdws_dev", 1);
        setenv("DB_NAME_PROD", "rdws_prod", 1);

        rdws::Config config_dev;
        std::cout << "=== Development Config ===" << std::endl;
        std::cout << "Host: " << config_dev.getHost() << std::endl;
        std::cout << "Port: " << config_dev.getPort() << std::endl;
        std::cout << "User: " << config_dev.getUser() << std::endl;
        std::cout << "Database: " << config_dev.getDatabase() << std::endl;
        std::cout << "Environment: " << config_dev.getEnvironment() << std::endl;

        // Test production environment
        setenv("ENVIRONMENT", "production", 1);

        rdws::Config config_prod;
        std::cout << "\n=== Production Config ===" << std::endl;
        std::cout << "Host: " << config_prod.getHost() << std::endl;
        std::cout << "Port: " << config_prod.getPort() << std::endl;
        std::cout << "User: " << config_prod.getUser() << std::endl;
        std::cout << "Database: " << config_prod.getDatabase() << std::endl;
        std::cout << "Environment: " << config_prod.getEnvironment() << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
