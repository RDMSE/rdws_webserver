#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <json/json.h>
#include <rapidjson/document.h>

namespace rdws {
namespace types {

struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string created_at;
    
    // Constructors
    User() = default;
    User(const std::string& n, const std::string& e) : name(n), email(e) {}
    User(int i, const std::string& n, const std::string& e, const std::string& c) 
        : id(i), name(n), email(e), created_at(c) {}
    
    // Convert to JSON string using jsoncpp (for backward compatibility)
    std::string toJsonString() const {
        Json::Value json;
        json["id"] = id;
        json["name"] = name;
        json["email"] = email;
        json["created_at"] = created_at;
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        return Json::writeString(builder, json);
    }
    
    // Convert to RapidJSON Value (for ResponseHelper)
    ::rapidjson::Value toJsonValue(::rapidjson::Document::AllocatorType& allocator) const {
        ::rapidjson::Value userObj(::rapidjson::kObjectType);
        
        userObj.AddMember("id", ::rapidjson::Value(id), allocator);
        userObj.AddMember("name", ::rapidjson::Value(name.c_str(), allocator), allocator);
        userObj.AddMember("email", ::rapidjson::Value(email.c_str(), allocator), allocator);
        userObj.AddMember("created_at", ::rapidjson::Value(created_at.c_str(), allocator), allocator);
        
        return userObj;
    }
};

} // namespace types
} // namespace rdws