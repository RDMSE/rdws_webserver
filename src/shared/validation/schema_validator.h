#pragma once

#include <json/json.h>
#include <memory>
#include <string>
#include <utility>
#include <valijson/schema.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>
#include <vector>


namespace rdws::validation {

struct ValidationError {
    std::string field;
    std::string message;
    std::string context;

    ValidationError(std::string  f, std::string  m, std::string  c = "")
        : field(std::move(f)), message(std::move(m)), context(std::move(c)) {}
};

class SchemaValidator {
  private:
    std::string schemaName;
    std::unique_ptr<valijson::Schema> schema;
    std::unique_ptr<valijson::Validator> validator;

    // Private default constructor for factory methods
    SchemaValidator() = default;

    [[nodiscard]] static std::string getSchemaPath(const std::string& schemaFile) ;
    [[nodiscard]] bool loadSchemaFromString(const std::string& schemaString) const;
    [[nodiscard]] static std::vector<ValidationError>
        convertValidationResults(const valijson::ValidationResults& results) ;

  public:
    // Constructor for string-based schemas (recommended)
    static SchemaValidator fromString(const std::string& name, const std::string& schemaString);

    ~SchemaValidator() = default;

    // Move constructor and assignment
    SchemaValidator(SchemaValidator&& other) noexcept;
    SchemaValidator& operator=(SchemaValidator&& other) noexcept;

    // Disable copy
    SchemaValidator(const SchemaValidator&) = delete;
    SchemaValidator& operator=(const SchemaValidator&) = delete;

    [[nodiscard]] std::vector<ValidationError> validate(const Json::Value& json) const;
    [[nodiscard]] std::vector<ValidationError> validate(const std::string& jsonString) const;

    [[nodiscard]] bool isValid(const Json::Value& json) const {
        return validate(json).empty();
    }

    [[nodiscard]] bool isValid(const std::string& jsonString) const {
        return validate(jsonString).empty();
    }

    [[nodiscard]] std::string getErrorsAsJson(const std::vector<ValidationError>& errors) const;

    [[nodiscard]] const std::string& getName() const {
        return schemaName;
    }
};

// Factory functions for common validators
namespace UserValidators {
    SchemaValidator createUserValidator();
    SchemaValidator updateUserValidator();
    SchemaValidator queryUserValidator();
} // namespace UserValidators

// Factory functions for order validators
namespace OrderValidators {
    SchemaValidator createOrderValidator();
    SchemaValidator updateOrderValidator();
} // namespace OrderValidators

// Schema manager for loading and caching schemas
class SchemaManager {
  private:
    std::string schemasPath;
    mutable std::map<std::string, std::unique_ptr<valijson::Schema>> schemaCache;

  public:
    explicit SchemaManager(std::string  path = "schemas");

    std::shared_ptr<valijson::Schema> getSchema(const std::string& schemaFile) const;
    void clearCache() const;
    bool reloadSchema(const std::string& schemaFile) const;
};

} // namespace rdws::validation

