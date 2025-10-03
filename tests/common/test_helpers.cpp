#include "test_helpers.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

std::string ExecutableTestHelper::executeCommand(const std::string& command) {
    char buffer[128];
    std::string result = "";
    
    // Runs a command and captures its output
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

bool ExecutableTestHelper::jsonContainsKey(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    return json.find(searchKey) != std::string::npos;
}

std::string ExecutableTestHelper::extractJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = json.find(searchKey);
    
    if (keyPos == std::string::npos) {
        return "";
    }
    
    size_t valueStart = keyPos + searchKey.length();
    
    // Skip whitespace
    while (valueStart < json.length() && json[valueStart] == ' ') {
        valueStart++;
    }
    
    if (valueStart >= json.length()) {
        return "";
    }
    
    // If it's a string (starts with ")
    if (json[valueStart] == '"') {
        valueStart++; // Skip the first "
        size_t valueEnd = json.find('"', valueStart);
        if (valueEnd != std::string::npos) {
            return json.substr(valueStart, valueEnd - valueStart);
        }
    } else {
        // If it's a number or another value
        size_t valueEnd = json.find_first_of(",}", valueStart);
        if (valueEnd != std::string::npos) {
            return json.substr(valueStart, valueEnd - valueStart);
        }
    }
    
    return "";
}

std::string ExecutableTestHelper::getServiceExecutablePath(const std::string& serviceName) {
    // Assuming we are running from the build/tests directory
    // and need to go up to build/ and then into services/
    return "../services/" + serviceName + "/" + serviceName + "_service";
}