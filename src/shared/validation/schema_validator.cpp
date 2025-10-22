#include "schema_validator.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace rdws {
namespace validation {

// SchemaValidator Implementation
SchemaValidator::SchemaValidator(const std::string& name, const std::string& schemaFile)
    : schemaName(name),
      schema(std::make_unique<valijson::Schema>()),
      validator(std::make_unique<valijson::Validator>()) {
    
    if (!loadSchemaFromFile(getSchemaPath(schemaFile))) {
        throw std::runtime_error("Failed to load schema: " + schemaFile);
    }
}

SchemaValidator::SchemaValidator(SchemaValidator&& other) noexcept
    : schemaName(std::move(other.schemaName)),
      schema(std::move(other.schema)),
      validator(std::move(other.validator)) {
}

SchemaValidator& SchemaValidator::operator=(SchemaValidator&& other) noexcept {
    if (this != &other) {
        schemaName = std::move(other.schemaName);
        schema = std::move(other.schema);
        validator = std::move(other.validator);
    }
    return *this;
}

std::string SchemaValidator::getSchemaPath(const std::string& schemaFile) const {
    // Try relative to current directory first (new location in src)
    std::string relativePath = "src/schemas/" + schemaFile;
    if (std::filesystem::exists(relativePath)) {
        return relativePath;
    }
    
    // Try old schemas path for backward compatibility
    std::string oldPath = "schemas/" + schemaFile;
    if (std::filesystem::exists(oldPath)) {
        return oldPath;
    }
    
    // Try absolute path
    if (std::filesystem::exists(schemaFile)) {
        return schemaFile;
    }
    
    // Default to new schemas directory
    return relativePath;
}

bool SchemaValidator::loadSchemaFromFile(const std::string& filePath) {
    try {
        Json::Value schemaDoc;
        if (!valijson::utils::loadDocument(filePath, schemaDoc)) {
            std::cerr << "Failed to load schema document: " << filePath << std::endl;
            return false;
        }
        
        valijson::SchemaParser parser;
        valijson::adapters::JsonCppAdapter schemaAdapter(schemaDoc);
        parser.populateSchema(schemaAdapter, *schema);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading schema " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

std::vector<ValidationError> SchemaValidator::validate(const Json::Value& json) const {
    valijson::ValidationResults results;
    valijson::adapters::JsonCppAdapter targetAdapter(json);
    
    if (validator->validate(*schema, targetAdapter, &results)) {
        return {}; // No errors
    }
    
    return convertValidationResults(results);
}

std::vector<ValidationError> SchemaValidator::validate(const std::string& jsonString) const {
    Json::Value json;
    Json::Reader reader;
    
    if (!reader.parse(jsonString, json)) {
        return {ValidationError("root", "Invalid JSON format: " + reader.getFormattedErrorMessages())};
    }
    
    return validate(json);
}

std::vector<ValidationError> SchemaValidator::convertValidationResults(
    const valijson::ValidationResults& results
) const {
    std::vector<ValidationError> errors;
    
    // Note: results.popError modifies the object, so we need non-const
    valijson::ValidationResults& mutableResults = const_cast<valijson::ValidationResults&>(results);
    
    valijson::ValidationResults::Error error;
    while (mutableResults.popError(error)) {
        std::string fieldPath;
        for (const auto& token : error.context) {
            if (!fieldPath.empty()) {
                fieldPath += ".";
            }
            fieldPath += token;
        }
        
        if (fieldPath.empty()) {
            fieldPath = "root";
        }
        
        errors.emplace_back(fieldPath, error.description, "");
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
        errorObj["message"] = error.message;
        if (!error.context.empty()) {
            errorObj["context"] = error.context;
        }
        result["errors"].append(errorObj);
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, result);
}

// Factory functions
namespace UserValidators {
    SchemaValidator createUserValidator() {
        return SchemaValidator("create_user", "user/create.json");
    }
    
    SchemaValidator updateUserValidator() {
        return SchemaValidator("update_user", "user/update.json");
    }
    
    SchemaValidator queryUserValidator() {
        return SchemaValidator("query_user", "user/query.json");
    }
}

// SchemaManager Implementation
SchemaManager::SchemaManager(const std::string& path) : schemasPath(path) {
}

std::shared_ptr<valijson::Schema> SchemaManager::getSchema(const std::string& schemaFile) const {
    auto it = schemaCache.find(schemaFile);
    if (it != schemaCache.end()) {
        return std::shared_ptr<valijson::Schema>(it->second.get(), [](valijson::Schema*){});
    }
    
    // Load schema
    auto schema = std::make_unique<valijson::Schema>();
    std::string filePath = schemasPath + "/" + schemaFile;
    
    Json::Value schemaDoc;
    if (!valijson::utils::loadDocument(filePath, schemaDoc)) {
        return nullptr;
    }
    
    valijson::SchemaParser parser;
    valijson::adapters::JsonCppAdapter schemaAdapter(schemaDoc);
    parser.populateSchema(schemaAdapter, *schema);
    
    auto rawPtr = schema.get();
    schemaCache[schemaFile] = std::move(schema);
    
    return std::shared_ptr<valijson::Schema>(rawPtr, [](valijson::Schema*){});
}

void SchemaManager::clearCache() {
    schemaCache.clear();
}

bool SchemaManager::reloadSchema(const std::string& schemaFile) {
    schemaCache.erase(schemaFile);
    return getSchema(schemaFile) != nullptr;
}

} // namespace validation
} // namespace rdws