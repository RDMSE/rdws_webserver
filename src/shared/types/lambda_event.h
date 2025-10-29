#pragma once

#include <string>
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>


namespace rdws::types {

/**
 * HTTP Request Information similar to AWS API Gateway Event
 */
struct HttpRequestInfo {
    std::string method;
    std::string path;
    std::string resource;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryStringParameters;
    std::map<std::string, std::string> pathParameters;
    std::string body;
    bool isBase64Encoded = false;
    
    HttpRequestInfo() = default;
    HttpRequestInfo(std::string  method, const std::string& path, std::string  body = "");
};

/**
 * Request Context Information
 */
struct RequestContext {
    std::string requestId;
    std::string stage;
    std::string httpMethod;
    std::string resourcePath;
    std::string protocol;
    std::string sourceIp;
    std::string userAgent;
    int64_t requestTimeEpoch{};
    
    RequestContext();
};

/**
 * Event object similar to AWS Lambda Event for API Gateway
 * Contains all request information and metadata
 */
class LambdaEvent {
private:
    HttpRequestInfo httpRequest_;
    RequestContext requestContext_;
    std::map<std::string, std::string> stageVariables_;
    rapidjson::Document jsonBody_;
    bool bodyParsed_ = false;

public:
    /**
     * Constructor for LambdaEvent
     * @param method HTTP method (GET, POST, PUT, DELETE, etc.)
     * @param path Request path
     * @param body Request body (optional)
     */
    LambdaEvent(const std::string& method, const std::string& path, const std::string& body = "");

    /**
     * Constructor from JSON string (for API Gateway integration)
     * @param jsonString JSON representation of the event
     */
    explicit LambdaEvent(const std::string& jsonString);

    /**
     * Constructor from command line arguments (backward compatibility)
     * @param argc Argument count
     * @param argv Argument values
     */
    LambdaEvent(int argc, char* argv[]);

    // HTTP Request getters
    [[nodiscard]] const std::string& getHttpMethod() const { return httpRequest_.method; }
    [[nodiscard]] const std::string& getPath() const { return httpRequest_.path; }
    [[nodiscard]] const std::string& getResource() const { return httpRequest_.resource; }
    [[nodiscard]] const std::string& getBody() const { return httpRequest_.body; }
    [[nodiscard]] bool isBase64Encoded() const { return httpRequest_.isBase64Encoded; }

    // Headers
    [[nodiscard]] const std::map<std::string, std::string>& getHeaders() const { return httpRequest_.headers; }
    [[nodiscard]] std::string getHeader(const std::string& name) const;
    void setHeader(const std::string& name, const std::string& value);

    // Query parameters
    [[nodiscard]] const std::map<std::string, std::string>& getQueryStringParameters() const { return httpRequest_.queryStringParameters; }
    [[nodiscard]] std::string getQueryParameter(const std::string& name) const;
    void setQueryParameter(const std::string& name, const std::string& value);

    // Path parameters
    [[nodiscard]] const std::map<std::string, std::string>& getPathParameters() const { return httpRequest_.pathParameters; }
    [[nodiscard]] std::string getPathParameter(const std::string& name) const;
    void setPathParameter(const std::string& name, const std::string& value);

    // Request context
    [[nodiscard]] const RequestContext& getRequestContext() const { return requestContext_; }
    RequestContext& getRequestContext() { return requestContext_; }

    // Stage variables
    [[nodiscard]] const std::map<std::string, std::string>& getStageVariables() const { return stageVariables_; }
    [[nodiscard]] std::string getStageVariable(const std::string& name) const;
    void setStageVariable(const std::string& name, const std::string& value);

    // Body handling
    void setBody(const std::string& body);
    [[nodiscard]] bool hasJsonBody() const;
    const rapidjson::Document& getJsonBody();
    
    /**
     * Extract path parameter from URL pattern
     * Example: /users/{id} with path /users/123 returns {id: "123"}
     * @param pattern URL pattern with {param} placeholders
     */
    void extractPathParameters(const std::string& pattern);

    /**
     * Parse query string from URL
     * @param queryString Query string part of URL (after ?)
     */
    void parseQueryString(const std::string& queryString);

    /**
     * Create LambdaEvent from JSON string
     * @param jsonString JSON representation of event
     * @return LambdaEvent instance
     */
    static LambdaEvent fromJson(const std::string& jsonString);
    
    /**
     * Convert to JSON string
     * @return JSON representation of this event
     */
    [[nodiscard]] std::string toJson() const;

    /**
     * Check if this is a specific HTTP method
     */
    [[nodiscard]] bool isGet() const { return httpRequest_.method == "GET"; }
    [[nodiscard]] bool isPost() const { return httpRequest_.method == "POST"; }
    [[nodiscard]] bool isPut() const { return httpRequest_.method == "PUT"; }
    [[nodiscard]] bool isDelete() const { return httpRequest_.method == "DELETE"; }
    [[nodiscard]] bool isPatch() const { return httpRequest_.method == "PATCH"; }

    /**
     * Check if path matches a pattern
     * @param pattern Pattern to match (supports wildcards and parameters)
     * @return True if path matches pattern
     */
    [[nodiscard]] bool pathMatches(const std::string& pattern) const;
};

} // namespace rdws::types
