// Example of how to define schemas using the new declarative API
// Similar to your TypeScript getDutyAssignmentsSchema

#include "../shared/validation/schema_validator.h"
using namespace rdws::validation;

// Example: Schema equivalent to your getDutyAssignmentsSchema
Schema getDutyAssignmentsSchema() {
    Schema schema;
    schema.addProperty("operator", stringField())
          .addProperty("crewCodes", stringField())
          .addProperty("startDate", dateField())    // automatically validates YYYY-MM-DD format
          .addProperty("endDate", dateField())
          .addProperty("showInactive", stringField())
          .addProperty("flightLegId", stringField());
    
    schema.addRequired("startDate");
    schema.addRequired("endDate");
    
    return schema;
}

// Example: User creation schema
Schema createUserSchema() {
    Schema schema;
    schema.addProperty("name", stringField(2, 100))           // min 2, max 100 chars
          .addProperty("email", emailField())                 // auto email validation
          .addProperty("age", integerField(18, 120));         // min 18, max 120
    
    schema.addRequired("name");
    schema.addRequired("email");
    
    return schema;
}

// Example: Product schema with enum
Schema createProductSchema() {
    Schema schema;
    schema.addProperty("name", stringField(1, 255))
          .addProperty("category", enumField({"electronics", "clothing", "books", "food"}))
          .addProperty("price", integerField(0, 999999))      // price in cents
          .addProperty("inStock", booleanField());
    
    schema.addRequired("name");
    schema.addRequired("category");
    schema.addRequired("price");
    
    return schema;
}

// Usage example:
void validateUserInput() {
    // Create schema
    auto schema = createUserSchema();
    SchemaValidator validator("create_user", schema);
    
    // Test JSON
    std::string jsonData = R"({
        "name": "John Doe",
        "email": "john@example.com",
        "age": 30
    })";
    
    // Validate
    auto errors = validator.validate(jsonData);
    if (errors.empty()) {
        std::cout << "✅ Validation passed!" << std::endl;
    } else {
        std::cout << "❌ Validation failed:" << std::endl;
        std::cout << validator.getErrorsAsJson(errors) << std::endl;
    }
}

// Test invalid data
void testInvalidData() {
    auto schema = getDutyAssignmentsSchema();
    SchemaValidator validator("duty_assignments", schema);
    
    // Missing required fields
    std::string invalidJson = R"({
        "operator": "TAM",
        "crewCodes": "A001,A002"
    })";
    
    auto errors = validator.validate(invalidJson);
    if (!errors.empty()) {
        std::cout << "Expected validation errors:" << std::endl;
        std::cout << validator.getErrorsAsJson(errors) << std::endl;
    }
}