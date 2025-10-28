#pragma once

#include <string>
#include <chrono>


namespace rdws::types {

/**
 * Context object similar to AWS Lambda Context
 * Contains runtime information and utilities for the request
 */
class LambdaContext {
private:
    std::string requestId_;
    std::string functionName_;
    std::string functionVersion_;
    std::chrono::milliseconds timeoutMs_{};
    std::chrono::steady_clock::time_point startTime_;
    int memoryLimitMB_;

public:
    /**
     * Constructor for LambdaContext
     * @param requestId Unique identifier for this request
     * @param functionName Name of the function being executed
     * @param functionVersion Version of the function
     * @param timeoutMs Timeout in milliseconds for the function
     * @param memoryLimitMB Memory limit in MB
     */
    LambdaContext(std::string  requestId,
                  std::string  functionName,
                  std::string  functionVersion = "1.0",
                  std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(30000),
                  int memoryLimitMB = 128);

    /**
     * Constructor from JSON string
     * @param jsonString JSON representation of context
     */
    explicit LambdaContext(const std::string& jsonString);

    /**
     * Create LambdaContext from JSON string
     * @param jsonString JSON representation of context
     * @return LambdaContext instance
     */
    static LambdaContext fromJson(const std::string& jsonString);

    /**
     * Convert to JSON string
     * @return JSON representation of this context
     */
    [[nodiscard]] std::string toJson() const;

    // Getters
    [[nodiscard]] const std::string& getRequestId() const { return requestId_; }
    [[nodiscard]] const std::string& getFunctionName() const { return functionName_; }
    [[nodiscard]] const std::string& getFunctionVersion() const { return functionVersion_; }
    [[nodiscard]] std::chrono::milliseconds getTimeout() const { return timeoutMs_; }
    [[nodiscard]] int getMemoryLimitMB() const { return memoryLimitMB_; }

    /**
     * Get remaining time in milliseconds before timeout
     * @return Remaining time in milliseconds
     */
    [[nodiscard]] std::chrono::milliseconds getRemainingTimeMs() const;

    /**
     * Check if the function is about to timeout
     * @param bufferMs Buffer time in milliseconds before actual timeout
     * @return True if timeout is imminent
     */
    [[nodiscard]] bool isTimeoutImminent(std::chrono::milliseconds bufferMs = std::chrono::milliseconds(1000)) const;

    /**
     * Get elapsed time since function start
     * @return Elapsed time in milliseconds
     */
    [[nodiscard]] std::chrono::milliseconds getElapsedTimeMs() const;

    /**
     * Log a message with context information
     * @param message Message to log
     * @param level Log level (INFO, WARN, ERROR)
     */
    void log(const std::string& message, const std::string& level = "INFO") const;
};

} // namespace rdws::types
