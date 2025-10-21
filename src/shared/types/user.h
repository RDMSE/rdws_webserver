#pragma once

#include <string>
#include <jsoncpp/json/json.h>

namespace rdws {
namespace types {

struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string created_at;
    
    // Default constructor
    User() = default;
    
    // Constructor with parameters
    User(int id, const std::string& name, const std::string& email, const std::string& created_at = "")
        : id(id), name(name), email(email), created_at(created_at) {}
    
    // Constructor from name and email (for creation)
    User(const std::string& name, const std::string& email)
        : id(0), name(name), email(email) {}
    
    // JSON serialization
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["name"] = name;
        json["email"] = email;
        if (!created_at.empty()) {
            json["created_at"] = created_at;
        }
        return json;
    }
    
    // JSON serialization to string
    std::string toJsonString() const {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        return Json::writeString(builder, toJson());
    }
    
    // JSON deserialization
    static User fromJson(const Json::Value& json) {
        User user;
        if (json.isMember("id")) {
            user.id = json["id"].asInt();
        }
        if (json.isMember("name")) {
            user.name = json["name"].asString();
        }
        if (json.isMember("email")) {
            user.email = json["email"].asString();
        }
        if (json.isMember("created_at")) {
            user.created_at = json["created_at"].asString();
        }
        return user;
    }
    
    // Validation
    bool isValid() const {
        return !name.empty() && !email.empty() && email.find('@') != std::string::npos;
    }
    
    // Comparison operators
    bool operator==(const User& other) const {
        return id == other.id && name == other.name && email == other.email;
    }
    
    bool operator!=(const User& other) const {
        return !(*this == other);
    }
};

} // namespace types
} // namespace rdws