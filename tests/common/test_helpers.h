#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <string>
#include <memory>
#include <cstdio>

class ExecutableTestHelper {
public:
    // Runs a command and captures its output
    static std::string executeCommand(const std::string& command);
    
    // Checks if a JSON contains a specific key
    static bool jsonContainsKey(const std::string& json, const std::string& key);

    // Extracts the value of a key from the JSON (simple implementation)
    static std::string extractJsonValue(const std::string& json, const std::string& key);

    // Builds the path to a microservice executable
    static std::string getServiceExecutablePath(const std::string& serviceName);
};

#endif // TEST_HELPERS_H