/**
 * Example: Using new string-based schemas
 *
 * This demonstrates how to use the improved schema validation
 * with embedded schemas instead of external files.
 */

#include "../src/shared/validation/schema_validator.h"

#include <iostream>
#include <json/json.h>

void demonstrateSchemaUsage() {
    std::cout << "=== Schema Validation Examples ===" << std::endl;

    // Example 1: Valid user creation
    {
        std::cout << "\n1. Testing valid user creation:" << std::endl;
        auto validator = rdws::validation::UserValidators::createUserValidator();

        std::string validUser = R"({
            "name": "John Doe",
            "email": "john.doe@example.com"
        })";

        auto errors = validator.validate(validUser);
        if (errors.empty()) {
            std::cout << "Valid user data!" << std::endl;
        } else {
            std::cout << "Validation errors:" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  - " << error.field << ": " << error.message << std::endl;
            }
        }
    }

    // Example 2: Invalid user creation (missing email)
    {
        std::cout << "\n2. Testing invalid user creation (missing email):" << std::endl;
        auto validator = rdws::validation::UserValidators::createUserValidator();

        std::string invalidUser = R"({
            "name": "Jane Doe"
        })";

        auto errors = validator.validate(invalidUser);
        if (errors.empty()) {
            std::cout << "Valid user data!" << std::endl;
        } else {
            std::cout << "Validation errors (expected):" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  - " << error.field << ": " << error.message << std::endl;
            }
        }
    }

    // Example 3: User update (partial)
    {
        std::cout << "\n3. Testing user update (partial data):" << std::endl;
        auto validator = rdws::validation::UserValidators::updateUserValidator();

        std::string updateUser = R"({
            "name": "John Smith"
        })";

        auto errors = validator.validate(updateUser);
        if (errors.empty()) {
            std::cout << "Valid update data!" << std::endl;
        } else {
            std::cout << "Validation errors:" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  - " << error.field << ": " << error.message << std::endl;
            }
        }
    }

    // Example 4: Order creation
    {
        std::cout << "\n4. Testing order creation:" << std::endl;
        auto validator = rdws::validation::OrderValidators::createOrderValidator();

        std::string validOrder = R"({
            "user_id": 123,
            "product_name": "Laptop Dell XPS 13",
            "quantity": 2,
            "price": 999.99
        })";

        auto errors = validator.validate(validOrder);
        if (errors.empty()) {
            std::cout << "Valid order data!" << std::endl;
        } else {
            std::cout << "Validation errors:" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  - " << error.field << ": " << error.message << std::endl;
            }
        }
    }

    std::cout << "\n=== Schema validation completed ===" << std::endl;
}

int main() {
    try {
        demonstrateSchemaUsage();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
