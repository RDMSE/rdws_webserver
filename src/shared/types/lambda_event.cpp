#include "lambda_event.h"
#include <algorithm>
#include <sstream>
#include <random>
#include <regex>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace rdws {
namespace types {

// Helper function to generate request ID
std::string generateRequestId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::string chars = "0123456789abcdef";
    std::string requestId;
    requestId.reserve(32);
    
    for (int i = 0; i < 32; ++i) {
        requestId += chars[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) {
            requestId += '-';
        }
    }
    
    return requestId;
}

// HttpRequestInfo implementation
HttpRequestInfo::HttpRequestInfo(const std::string& method, const std::string& path, const std::string& body)
    : method(method), path(path), resource(path), body(body) {
}

// RequestContext implementation
RequestContext::RequestContext() 
    : requestId(generateRequestId()),
      stage("prod"),
      protocol("HTTP/1.1"),
      sourceIp("127.0.0.1"),
      userAgent("rdws-microservice/1.0") {
    
    auto now = std::chrono::system_clock::now();
    requestTimeEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

// LambdaEvent implementation
LambdaEvent::LambdaEvent(const std::string& method, const std::string& path, const std::string& body)
    : httpRequest_(method, path, body) {
    
    requestContext_.httpMethod = method;
    requestContext_.resourcePath = path;
    
    // Parse query string from path if present
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        httpRequest_.path = path.substr(0, queryPos);
        httpRequest_.resource = httpRequest_.path;
        requestContext_.resourcePath = httpRequest_.path;
        parseQueryString(path.substr(queryPos + 1));
    }
}

LambdaEvent::LambdaEvent(const std::string& jsonString) {
    // Parse JSON string to reconstruct event
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());
    
    if (doc.HasParseError()) {
        throw std::runtime_error("Invalid JSON in LambdaEvent constructor");
    }
    
    // Extract HTTP method and path
    if (doc.HasMember("httpMethod") && doc["httpMethod"].IsString()) {
        httpRequest_.method = doc["httpMethod"].GetString();
    }
    
    if (doc.HasMember("path") && doc["path"].IsString()) {
        httpRequest_.path = doc["path"].GetString();
    }
    
    if (doc.HasMember("resource") && doc["resource"].IsString()) {
        httpRequest_.resource = doc["resource"].GetString();
    }
    
    // Extract body
    if (doc.HasMember("body") && doc["body"].IsString()) {
        httpRequest_.body = doc["body"].GetString();
    }
    
    // Extract headers
    if (doc.HasMember("headers") && doc["headers"].IsObject()) {
        for (auto& member : doc["headers"].GetObject()) {
            httpRequest_.headers[member.name.GetString()] = member.value.GetString();
        }
    }
    
    // Extract query parameters
    if (doc.HasMember("queryStringParameters") && doc["queryStringParameters"].IsObject()) {
        for (auto& member : doc["queryStringParameters"].GetObject()) {
            httpRequest_.queryStringParameters[member.name.GetString()] = member.value.GetString();
        }
    }
    
    // Extract path parameters
    if (doc.HasMember("pathParameters") && doc["pathParameters"].IsObject()) {
        for (auto& member : doc["pathParameters"].GetObject()) {
            httpRequest_.pathParameters[member.name.GetString()] = member.value.GetString();
        }
    }
    
    // Extract request context
    if (doc.HasMember("requestContext") && doc["requestContext"].IsObject()) {
        const auto& ctx = doc["requestContext"];
        
        if (ctx.HasMember("requestId") && ctx["requestId"].IsString()) {
            requestContext_.requestId = ctx["requestId"].GetString();
        }
        
        if (ctx.HasMember("stage") && ctx["stage"].IsString()) {
            requestContext_.stage = ctx["stage"].GetString();
        }
        
        if (ctx.HasMember("httpMethod") && ctx["httpMethod"].IsString()) {
            requestContext_.httpMethod = ctx["httpMethod"].GetString();
        }
        
        if (ctx.HasMember("resourcePath") && ctx["resourcePath"].IsString()) {
            requestContext_.resourcePath = ctx["resourcePath"].GetString();
        }
        
        if (ctx.HasMember("protocol") && ctx["protocol"].IsString()) {
            requestContext_.protocol = ctx["protocol"].GetString();
        }
        
        if (ctx.HasMember("sourceIp") && ctx["sourceIp"].IsString()) {
            requestContext_.sourceIp = ctx["sourceIp"].GetString();
        }
        
        if (ctx.HasMember("userAgent") && ctx["userAgent"].IsString()) {
            requestContext_.userAgent = ctx["userAgent"].GetString();
        }
        
        if (ctx.HasMember("requestTimeEpoch") && ctx["requestTimeEpoch"].IsInt64()) {
            requestContext_.requestTimeEpoch = ctx["requestTimeEpoch"].GetInt64();
        }
    }
    
    // Extract stage variables
    if (doc.HasMember("stageVariables") && doc["stageVariables"].IsObject()) {
        for (auto& member : doc["stageVariables"].GetObject()) {
            stageVariables_[member.name.GetString()] = member.value.GetString();
        }
    }
}

LambdaEvent::LambdaEvent(int argc, char* argv[]) {
    // Default values
    std::string method = "GET";
    std::string path = "/";
    std::string body = "";
    
    // Parse command line arguments
    if (argc > 1) {
        method = argv[1];
    }
    if (argc > 2) {
        path = argv[2];
    }
    if (argc > 3) {
        body = argv[3];
    }
    
    // Initialize
    httpRequest_ = HttpRequestInfo(method, path, body);
    requestContext_.httpMethod = method;
    requestContext_.resourcePath = path;
    
    // Parse query string from path if present
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        httpRequest_.path = path.substr(0, queryPos);
        httpRequest_.resource = httpRequest_.path;
        requestContext_.resourcePath = httpRequest_.path;
        parseQueryString(path.substr(queryPos + 1));
    }
}

LambdaEvent LambdaEvent::fromJson(const std::string& jsonString) {
    return LambdaEvent(jsonString);
}

std::string LambdaEvent::getHeader(const std::string& name) const {
    auto it = httpRequest_.headers.find(name);
    return (it != httpRequest_.headers.end()) ? it->second : "";
}

void LambdaEvent::setHeader(const std::string& name, const std::string& value) {
    httpRequest_.headers[name] = value;
}

std::string LambdaEvent::getQueryParameter(const std::string& name) const {
    auto it = httpRequest_.queryStringParameters.find(name);
    return (it != httpRequest_.queryStringParameters.end()) ? it->second : "";
}

void LambdaEvent::setQueryParameter(const std::string& name, const std::string& value) {
    httpRequest_.queryStringParameters[name] = value;
}

std::string LambdaEvent::getPathParameter(const std::string& name) const {
    auto it = httpRequest_.pathParameters.find(name);
    return (it != httpRequest_.pathParameters.end()) ? it->second : "";
}

void LambdaEvent::setPathParameter(const std::string& name, const std::string& value) {
    httpRequest_.pathParameters[name] = value;
}

std::string LambdaEvent::getStageVariable(const std::string& name) const {
    auto it = stageVariables_.find(name);
    return (it != stageVariables_.end()) ? it->second : "";
}

void LambdaEvent::setStageVariable(const std::string& name, const std::string& value) {
    stageVariables_[name] = value;
}

void LambdaEvent::setBody(const std::string& body) {
    httpRequest_.body = body;
    bodyParsed_ = false; // Reset JSON parsing
}

bool LambdaEvent::hasJsonBody() const {
    return !httpRequest_.body.empty() && 
           (httpRequest_.body[0] == '{' || httpRequest_.body[0] == '[');
}

const rapidjson::Document& LambdaEvent::getJsonBody() {
    if (!bodyParsed_ && hasJsonBody()) {
        jsonBody_.Parse(httpRequest_.body.c_str());
        bodyParsed_ = true;
    }
    return jsonBody_;
}

void LambdaEvent::extractPathParameters(const std::string& pattern) {
    std::regex paramRegex(R"(\{([^}]+)\})");
    std::string regexPattern = pattern;
    std::vector<std::string> paramNames;
    
    // Extract parameter names and build regex pattern
    std::sregex_iterator iter(pattern.begin(), pattern.end(), paramRegex);
    std::sregex_iterator end;
    
    size_t offset = 0;
    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        paramNames.push_back(match[1].str());
        
        size_t pos = match.position() - offset;
        regexPattern.replace(pos, match.length(), "([^/]+)");
        offset += match.length() - 7; // 7 is length of "([^/]+)"
    }
    
    // Match against actual path
    std::regex pathRegex(regexPattern);
    std::smatch pathMatch;
    
    if (std::regex_match(httpRequest_.path, pathMatch, pathRegex)) {
        for (size_t i = 0; i < paramNames.size() && i + 1 < pathMatch.size(); ++i) {
            httpRequest_.pathParameters[paramNames[i]] = pathMatch[i + 1].str();
        }
    }
}

void LambdaEvent::parseQueryString(const std::string& queryString) {
    std::istringstream qs(queryString);
    std::string pair;
    
    while (std::getline(qs, pair, '&')) {
        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string key = pair.substr(0, eqPos);
            std::string value = pair.substr(eqPos + 1);
            httpRequest_.queryStringParameters[key] = value;
        } else {
            httpRequest_.queryStringParameters[pair] = "";
        }
    }
}

bool LambdaEvent::pathMatches(const std::string& pattern) const {
    if (pattern == httpRequest_.path) {
        return true;
    }
    
    // Simple wildcard matching
    if (pattern.find("*") != std::string::npos) {
        std::string regexPattern = pattern;
        std::replace(regexPattern.begin(), regexPattern.end(), '*', '.');
        regexPattern = "^" + regexPattern + "$";
        std::regex regex(regexPattern);
        return std::regex_match(httpRequest_.path, regex);
    }
    
    // Parameter matching
    if (pattern.find("{") != std::string::npos) {
        std::regex paramRegex(R"(\{[^}]+\})");
        std::string regexPattern = std::regex_replace(pattern, paramRegex, "[^/]+");
        regexPattern = "^" + regexPattern + "$";
        std::regex regex(regexPattern);
        return std::regex_match(httpRequest_.path, regex);
    }
    
    return false;
}

std::string LambdaEvent::toJson() const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    
    // Add HTTP request information
    doc.AddMember("httpMethod", rapidjson::Value(httpRequest_.method.c_str(), allocator), allocator);
    doc.AddMember("path", rapidjson::Value(httpRequest_.path.c_str(), allocator), allocator);
    doc.AddMember("resource", rapidjson::Value(httpRequest_.resource.c_str(), allocator), allocator);
    doc.AddMember("body", rapidjson::Value(httpRequest_.body.c_str(), allocator), allocator);
    
    // Add headers
    rapidjson::Value headers(rapidjson::kObjectType);
    for (const auto& pair : httpRequest_.headers) {
        headers.AddMember(
            rapidjson::Value(pair.first.c_str(), allocator),
            rapidjson::Value(pair.second.c_str(), allocator),
            allocator
        );
    }
    doc.AddMember("headers", headers, allocator);
    
    // Add query string parameters
    rapidjson::Value queryParams(rapidjson::kObjectType);
    for (const auto& pair : httpRequest_.queryStringParameters) {
        queryParams.AddMember(
            rapidjson::Value(pair.first.c_str(), allocator),
            rapidjson::Value(pair.second.c_str(), allocator),
            allocator
        );
    }
    doc.AddMember("queryStringParameters", queryParams, allocator);
    
    // Add path parameters
    rapidjson::Value pathParams(rapidjson::kObjectType);
    for (const auto& pair : httpRequest_.pathParameters) {
        pathParams.AddMember(
            rapidjson::Value(pair.first.c_str(), allocator),
            rapidjson::Value(pair.second.c_str(), allocator),
            allocator
        );
    }
    doc.AddMember("pathParameters", pathParams, allocator);
    
    // Add request context
    rapidjson::Value requestContext(rapidjson::kObjectType);
    requestContext.AddMember("requestId", rapidjson::Value(requestContext_.requestId.c_str(), allocator), allocator);
    requestContext.AddMember("stage", rapidjson::Value(requestContext_.stage.c_str(), allocator), allocator);
    requestContext.AddMember("httpMethod", rapidjson::Value(requestContext_.httpMethod.c_str(), allocator), allocator);
    requestContext.AddMember("resourcePath", rapidjson::Value(requestContext_.resourcePath.c_str(), allocator), allocator);
    requestContext.AddMember("protocol", rapidjson::Value(requestContext_.protocol.c_str(), allocator), allocator);
    requestContext.AddMember("sourceIp", rapidjson::Value(requestContext_.sourceIp.c_str(), allocator), allocator);
    requestContext.AddMember("userAgent", rapidjson::Value(requestContext_.userAgent.c_str(), allocator), allocator);
    requestContext.AddMember("requestTimeEpoch", rapidjson::Value(requestContext_.requestTimeEpoch), allocator);
    doc.AddMember("requestContext", requestContext, allocator);
    
    // Add stage variables
    rapidjson::Value stageVars(rapidjson::kObjectType);
    for (const auto& pair : stageVariables_) {
        stageVars.AddMember(
            rapidjson::Value(pair.first.c_str(), allocator),
            rapidjson::Value(pair.second.c_str(), allocator),
            allocator
        );
    }
    doc.AddMember("stageVariables", stageVars, allocator);
    
    // Convert to string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    
    return buffer.GetString();
}

} // namespace types
} // namespace rdws