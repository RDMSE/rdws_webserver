#include "schema_validator.h"
#include <sstream>
#include <regex>
#include <climits>
#include <cfloat>

namespace rdws {
namespace validation {

// Helper functions for creating field schemas
FieldSchema stringField(int minLen, int maxLen) {
    FieldSchema field("string");
    field.minLength = minLen;
    field.maxLength = maxLen;
    return field;
}

FieldSchema emailField() {
    FieldSchema field("string");
    field.format = "email";
    field.minLength = 5;
    field.maxLength = 255;
    return field;
}

FieldSchema dateField() {
    FieldSchema field("string");
    field.format = "date";
    return field;
}

FieldSchema integerField(int min, int max) {
    FieldSchema field("integer");
    field.minimum = min;
    field.maximum = max;
    return field;
}

FieldSchema numberField(double min, double max) {
    FieldSchema field("number");
    // Note: using int fields for simplicity, could be extended for doubles
    return field;
}

FieldSchema booleanField() {
    return FieldSchema("boolean");
}

FieldSchema enumField(const std::vector<std::string>& values) {
    FieldSchema field("string");
    field.enumValues = values;
    return field;
}

// SchemaValidator implementation
std::vector<ValidationError> SchemaValidator::validate(const Json::Value& json) const {
    std::vector<ValidationError> errors;
    
    if (!json.isObject()) {
        errors.emplace_back("root", ValidationResult::INVALID_FORMAT, "Input must be a JSON object");
        return errors;
    }
    
    // Check required fields
    for (const auto& requiredField : schema.required) {
        if (!json.isMember(requiredField)) {
            errors.emplace_back(requiredField, ValidationResult::MISSING_REQUIRED_FIELD,
                              "Required field '" + requiredField + "' is missing");
        } else if (json[requiredField].isNull()) {
            errors.emplace_back(requiredField, ValidationResult::MISSING_REQUIRED_FIELD,
                              "Required field '" + requiredField + "' cannot be null");
        }
    }
    
    // Validate each property
    for (const auto& member : json.getMemberNames()) {
        auto propIt = schema.properties.find(member);
        if (propIt != schema.properties.end()) {
            auto fieldErrors = validateField(member, json[member], propIt->second);
            errors.insert(errors.end(), fieldErrors.begin(), fieldErrors.end());
        }
        // Note: We're not strict about unknown properties (could add option for this)
    }
    
    return errors;
}

std::vector<ValidationError> SchemaValidator::validate(const std::string& jsonString) const {
    std::vector<ValidationError> errors;
    
    Json::Value json;
    Json::Reader reader;
    
    if (!reader.parse(jsonString, json)) {
        errors.emplace_back("root", ValidationResult::INVALID_FORMAT,
                          "Invalid JSON format: " + reader.getFormattedErrorMessages());
        return errors;
    }
    
    return validate(json);
}

std::vector<ValidationError> SchemaValidator::validateField(
    const std::string& fieldName,
    const Json::Value& value,
    const FieldSchema& fieldSchema
) const {
    std::vector<ValidationError> errors;
    
    // Skip validation if value is null and field is not required
    if (value.isNull() && schema.required.find(fieldName) == schema.required.end()) {
        return errors;
    }
    
    // Type validation
    if (fieldSchema.type == "string" && !value.isString()) {
        errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_TYPE,
                          "Field '" + fieldName + "' must be a string");
        return errors; // Skip further validation
    } else if (fieldSchema.type == "integer" && !value.isInt()) {
        errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_TYPE,
                          "Field '" + fieldName + "' must be an integer");
        return errors;
    } else if (fieldSchema.type == "number" && !value.isNumeric()) {
        errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_TYPE,
                          "Field '" + fieldName + "' must be a number");
        return errors;
    } else if (fieldSchema.type == "boolean" && !value.isBool()) {
        errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_TYPE,
                          "Field '" + fieldName + "' must be a boolean");
        return errors;
    }
    
    // String-specific validations
    if (fieldSchema.type == "string" && value.isString()) {
        std::string strValue = value.asString();
        
        // Length validation
        if (fieldSchema.minLength >= 0 && static_cast<int>(strValue.length()) < fieldSchema.minLength) {
            errors.emplace_back(fieldName, ValidationResult::FIELD_TOO_SHORT,
                              "Field '" + fieldName + "' must be at least " + 
                              std::to_string(fieldSchema.minLength) + " characters");
        }
        if (fieldSchema.maxLength >= 0 && static_cast<int>(strValue.length()) > fieldSchema.maxLength) {
            errors.emplace_back(fieldName, ValidationResult::FIELD_TOO_LONG,
                              "Field '" + fieldName + "' must be at most " + 
                              std::to_string(fieldSchema.maxLength) + " characters");
        }
        
        // Format validation
        if (fieldSchema.format == "email" && !isValidEmail(strValue)) {
            errors.emplace_back(fieldName, ValidationResult::INVALID_EMAIL_FORMAT,
                              "Field '" + fieldName + "' must be a valid email address");
        } else if (fieldSchema.format == "date" && !isValidDate(strValue)) {
            errors.emplace_back(fieldName, ValidationResult::INVALID_DATE_FORMAT,
                              "Field '" + fieldName + "' must be a valid date (YYYY-MM-DD)");
        }
        
        // Enum validation
        if (!fieldSchema.enumValues.empty()) {
            bool found = false;
            for (const auto& enumValue : fieldSchema.enumValues) {
                if (strValue == enumValue) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_VALUE,
                                  "Field '" + fieldName + "' must be one of the allowed values");
            }
        }
        
        // Pattern validation
        if (!fieldSchema.pattern.empty() && !matchesPattern(strValue, fieldSchema.pattern)) {
            errors.emplace_back(fieldName, ValidationResult::INVALID_FIELD_VALUE,
                              "Field '" + fieldName + "' does not match required pattern");
        }
    }
    
    // Integer-specific validations
    if (fieldSchema.type == "integer" && value.isInt()) {
        int intValue = value.asInt();
        if (intValue < fieldSchema.minimum) {
            errors.emplace_back(fieldName, ValidationResult::VALUE_OUT_OF_RANGE,
                              "Field '" + fieldName + "' must be at least " + 
                              std::to_string(fieldSchema.minimum));
        }
        if (intValue > fieldSchema.maximum) {
            errors.emplace_back(fieldName, ValidationResult::VALUE_OUT_OF_RANGE,
                              "Field '" + fieldName + "' must be at most " + 
                              std::to_string(fieldSchema.maximum));
        }
    }
    
    return errors;
}

std::string SchemaValidator::getErrorsAsJson(const std::vector<ValidationError>& errors) const {
    Json::Value result;
    result["valid"] = false;
    result["schema"] = schemaName;
    result["errors"] = Json::Value(Json::arrayValue);
    
    for (const auto& error : errors) {
        Json::Value errorObj;
        errorObj["field"] = error.field;
        errorObj["code"] = static_cast<int>(error.result);
        errorObj["message"] = error.message;
        result["errors"].append(errorObj);
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, result);
}

// Validation helper methods
bool SchemaValidator::isValidEmail(const std::string& email) const {
    const std::regex emailRegex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );
    return std::regex_match(email, emailRegex);
}

bool SchemaValidator::isValidDate(const std::string& date) const {
    const std::regex dateRegex(R"(^\d{4}-\d{2}-\d{2}$)");
    return std::regex_match(date, dateRegex);
}

bool SchemaValidator::matchesPattern(const std::string& value, const std::string& pattern) const {
    try {
        const std::regex regex(pattern);
        return std::regex_match(value, regex);
    } catch (const std::exception&) {
        return false; // Invalid regex pattern
    }
}

// Predefined schemas
namespace UserSchemas {
    Schema createUserSchema() {
        Schema schema;
        schema.addProperty("name", stringField(2, 100))
              .addProperty("email", emailField());
        schema.addRequired("name");
        schema.addRequired("email");
        return schema;
    }
    
    Schema updateUserSchema() {
        Schema schema;
        schema.addProperty("id", integerField(1, INT_MAX))
              .addProperty("name", stringField(2, 100))
              .addProperty("email", emailField());
        schema.addRequired("id");
        return schema;
    }
    
    Schema getUserQuerySchema() {
        Schema schema;
        schema.addProperty("id", integerField(1, INT_MAX))
              .addProperty("limit", integerField(1, 1000))
              .addProperty("offset", integerField(0, INT_MAX));
        return schema;
    }
}

} // namespace validation
} // namespace rdws