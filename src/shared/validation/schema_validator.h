#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <climits>
#include <cfloat>
#include <jsoncpp/json/json.h>

namespace rdws {
namespace validation {

enum class ValidationResult {
    VALID,
    INVALID_FORMAT,
    MISSING_REQUIRED_FIELD,
    INVALID_FIELD_TYPE,
    INVALID_FIELD_VALUE,
    FIELD_TOO_LONG,
    FIELD_TOO_SHORT,
    INVALID_EMAIL_FORMAT,
    INVALID_DATE_FORMAT,
    VALUE_OUT_OF_RANGE
};

struct ValidationError {
    std::string field;
    ValidationResult result;
    std::string message;
    
    ValidationError(const std::string& f, ValidationResult r, const std::string& m)
        : field(f), result(r), message(m) {}
};

// Schema definition structures
struct FieldSchema {
    std::string type;           // "string", "integer", "number", "boolean", "array", "object"
    std::string format;         // "email", "date", "datetime", etc.
    int minLength = -1;
    int maxLength = -1;
    int minimum = INT_MIN;
    int maximum = INT_MAX;
    std::vector<std::string> enumValues;
    std::string pattern;        // regex pattern
    
    FieldSchema() = default;
    FieldSchema(const std::string& t) : type(t) {}
};

struct Schema {
    std::string type = "object";
    std::map<std::string, FieldSchema> properties;
    std::set<std::string> required;
    
    Schema& addProperty(const std::string& name, const FieldSchema& field) {
        properties[name] = field;
        return *this;
    }
    
    Schema& addRequired(const std::string& name) {
        required.insert(name);
        return *this;
    }
    
    Schema& addRequired(const std::vector<std::string>& names) {
        for (const auto& name : names) {
            required.insert(name);
        }
        return *this;
    }
};

class SchemaValidator {
private:
    std::string schemaName;
    Schema schema;
    
    std::vector<ValidationError> validateField(
        const std::string& fieldName,
        const Json::Value& value,
        const FieldSchema& fieldSchema
    ) const;
    
    bool isValidEmail(const std::string& email) const;
    bool isValidDate(const std::string& date) const;
    bool matchesPattern(const std::string& value, const std::string& pattern) const;
    
public:
    SchemaValidator(const std::string& name, const Schema& s) 
        : schemaName(name), schema(s) {}
    
    std::vector<ValidationError> validate(const Json::Value& json) const;
    std::vector<ValidationError> validate(const std::string& jsonString) const;
    
    bool isValid(const Json::Value& json) const {
        return validate(json).empty();
    }
    
    bool isValid(const std::string& jsonString) const {
        return validate(jsonString).empty();
    }
    
    std::string getErrorsAsJson(const std::vector<ValidationError>& errors) const;
};

// Helper functions for creating field schemas
FieldSchema stringField(int minLen = -1, int maxLen = -1);
FieldSchema emailField();
FieldSchema dateField();
FieldSchema integerField(int min = INT_MIN, int max = INT_MAX);
FieldSchema numberField(double min = -DBL_MAX, double max = DBL_MAX);
FieldSchema booleanField();
FieldSchema enumField(const std::vector<std::string>& values);

// Predefined schemas for User operations
namespace UserSchemas {
    Schema createUserSchema();
    Schema updateUserSchema();
    Schema getUserQuerySchema();
}

} // namespace validation
} // namespace rdws