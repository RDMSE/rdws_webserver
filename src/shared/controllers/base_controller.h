#pragma once

#include <ctime>
#include <string>

namespace rdws::controllers {

class BaseController {
  public:
    virtual ~BaseController() = default;

    static std::string formatMethodNotAllowedError(const std::string& method,
                                                   const std::string& path) {

        rapidjson::Document doc;
        doc.SetObject();

        auto& allocator = doc.GetAllocator();
        doc.AddMember("method", rapidjson::Value(method.c_str(), allocator), allocator);
        doc.AddMember("path", rapidjson::Value(path.c_str(), allocator), allocator);

        return formatErrorResponse("Method not allowed", doc, 405);
    }

    static std::string formatNoDataProvidedError(const std::string& operation) {
        return formatErrorResponse("No JSON data provided for " + operation, 400);
    }

    static std::string formatDatabaseError() {
        return formatErrorResponse("Failed to connect to database", 500);
    }

    static std::string formatUsageError() {
        return formatErrorResponse("Usage error: should be <service> <json lambda event> <json lambda context>", 500);
    }

    static std::string formatServiceError(const std::string& message) {
        return formatErrorResponse("Service error: " + message, 500);
    }

    static std::string formatError(const std::string& message, const int statusCode) {
        return formatErrorResponse(message, statusCode);
    }

  protected:
    // Helper method to format error responses
    static std::string formatErrorResponse(const std::string& errorMessage, int statusCode) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Build error response object
        doc.AddMember("success", false, allocator);
        doc.AddMember("error", rapidjson::Value(errorMessage.c_str(), allocator), allocator);
        doc.AddMember("statusCode", statusCode, allocator);
        doc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        doc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    // Helper method to format error responses with custom JSON data
    static std::string formatErrorResponse(const std::string& errorMessage,
                                           const rapidjson::Value& jsonData, int statusCode) {
        rapidjson::Document errorDoc;
        errorDoc.SetObject();
        auto& allocator = errorDoc.GetAllocator();

        // Build error response object
        errorDoc.AddMember("success", false, allocator);
        errorDoc.AddMember("error", rapidjson::Value(errorMessage.c_str(), allocator), allocator);
        errorDoc.AddMember("statusCode", statusCode, allocator);
        errorDoc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        errorDoc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        // Append all members from jsonData to errorDoc
        if (jsonData.IsObject()) {
            for (const auto& [name, value] : jsonData.GetObject()) {
                rapidjson::Value keyMember(name, allocator);
                rapidjson::Value valueMember(value, allocator);
                errorDoc.AddMember(keyMember, valueMember, allocator);
            }
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        errorDoc.Accept(writer);

        return buffer.GetString();
    }
};

} // namespace rdws::controllers
