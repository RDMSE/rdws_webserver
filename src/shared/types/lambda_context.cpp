#include "lambda_context.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <utility>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>


namespace rdws::types {

LambdaContext::LambdaContext(std::string  requestId,
                             std::string  functionName,
                             std::string  functionVersion,
                             const std::chrono::milliseconds timeoutMs,
                             const int memoryLimitMB)
    : requestId_(std::move(requestId)),
      functionName_(std::move(functionName)),
      functionVersion_(std::move(functionVersion)),
      timeoutMs_(timeoutMs),
      startTime_(std::chrono::steady_clock::now()),
      memoryLimitMB_(memoryLimitMB) {
}

LambdaContext::LambdaContext(const std::string& jsonString) 
    : startTime_(std::chrono::steady_clock::now()) {
    
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());
    
    if (doc.HasParseError()) {
        throw std::runtime_error("Invalid JSON in LambdaContext constructor");
    }
    
    // Set defaults
    requestId_ = "unknown";
    functionName_ = "unknown";
    functionVersion_ = "1.0";
    timeoutMs_ = std::chrono::milliseconds(30000);
    memoryLimitMB_ = 128;
    
    // Extract values from JSON
    if (doc.HasMember("requestId") && doc["requestId"].IsString()) {
        requestId_ = doc["requestId"].GetString();
    }
    
    if (doc.HasMember("functionName") && doc["functionName"].IsString()) {
        functionName_ = doc["functionName"].GetString();
    }
    
    if (doc.HasMember("functionVersion") && doc["functionVersion"].IsString()) {
        functionVersion_ = doc["functionVersion"].GetString();
    }
    
    if (doc.HasMember("timeoutMs") && doc["timeoutMs"].IsInt()) {
        timeoutMs_ = std::chrono::milliseconds(doc["timeoutMs"].GetInt());
    }
    
    if (doc.HasMember("memoryLimitMB") && doc["memoryLimitMB"].IsInt()) {
        memoryLimitMB_ = doc["memoryLimitMB"].GetInt();
    }
}

std::chrono::milliseconds LambdaContext::getRemainingTimeMs() const {
    const auto elapsed = getElapsedTimeMs();
    return timeoutMs_ - elapsed;
}

bool LambdaContext::isTimeoutImminent(const std::chrono::milliseconds bufferMs) const {
    return getRemainingTimeMs() <= bufferMs;
}

std::chrono::milliseconds LambdaContext::getElapsedTimeMs() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
}

void LambdaContext::log(const std::string& message, const std::string& level) const {
    const auto now = std::chrono::system_clock::now();
    const auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << "[" << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ") << "] "
        << "[" << level << "] "
        << "[" << requestId_ << "] "
        << "[" << functionName_ << "] "
        << message;
    
    std::cerr << oss.str() << std::endl;
}

LambdaContext LambdaContext::fromJson(const std::string& jsonString) {
    return LambdaContext(jsonString);
}

std::string LambdaContext::toJson() const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    
    // Add all context properties
    doc.AddMember("requestId", rapidjson::Value(requestId_.c_str(), allocator), allocator);
    doc.AddMember("functionName", rapidjson::Value(functionName_.c_str(), allocator), allocator);
    doc.AddMember("functionVersion", rapidjson::Value(functionVersion_.c_str(), allocator), allocator);
    doc.AddMember("timeoutMs", rapidjson::Value(static_cast<int>(timeoutMs_.count())), allocator);
    doc.AddMember("memoryLimitMB", rapidjson::Value(memoryLimitMB_), allocator);
    
    // Convert to string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    
    return buffer.GetString();
}

} // namespace rdws::types
