#pragma once

#include <json/json.h>
#include <memory>
#include <string>
#include <valijson/adapters/jsoncpp_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/jsoncpp_utils.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>
#include <vector>

namespace rdws {
namespace validation {

struct ValidationError {
    std::string field;
    std::string message;
    std::string context;

    ValidationError(const std::string& f, const std::string& m, const std::string& c = "")
        : field(f), message(m), context(c) {}
};

class SchemaValidator {
  private:
    std::string schemaName;
    std::unique_ptr<valijson::Schema> schema;
    std::unique_ptr<valijson::Validator> validator;

    // Private default constructor for factory methods
    SchemaValidator() = default;

    std::string getSchemaPath(const std::string& schemaFile) const;
    bool loadSchemaFromFile(const std::string& filePath);
    bool loadSchemaFromString(const std::string& schemaString);
    std::vector<ValidationError>
    convertValidationResults(const valijson::ValidationResults& results) const;

  public:
    // Constructor for file-based schemas (legacy)
    SchemaValidator(const std::string& name, const std::string& schemaFile);

    // Constructor for string-based schemas (recommended)
    static SchemaValidator fromString(const std::string& name, const std::string& schemaString);

    ~SchemaValidator() = default;

    // Move constructor and assignment
    SchemaValidator(SchemaValidator&& other) noexcept;
    SchemaValidator& operator=(SchemaValidator&& other) noexcept;

    // Disable copy
    SchemaValidator(const SchemaValidator&) = delete;
    SchemaValidator& operator=(const SchemaValidator&) = delete;

    std::vector<ValidationError> validate(const Json::Value& json) const;
    std::vector<ValidationError> validate(const std::string& jsonString) const;

    bool isValid(const Json::Value& json) const {
        return validate(json).empty();
    }

    bool isValid(const std::string& jsonString) const {
        return validate(jsonString).empty();
    }

    std::string getErrorsAsJson(const std::vector<ValidationError>& errors) const;

    const std::string& getName() const {
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
    SchemaManager(const std::string& path = "schemas");

    std::shared_ptr<valijson::Schema> getSchema(const std::string& schemaFile) const;
    void clearCache();
    bool reloadSchema(const std::string& schemaFile);
};

} // namespace validation
} // namespace rdws
