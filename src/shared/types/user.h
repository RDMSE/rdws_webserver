#pragma once

#include <iomanip>
#include <json/json.h>
#include <rapidjson/document.h>
#include <string>
#include <utility>


namespace rdws::types {

struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string created_at;

    // Constructors
    User() = default;
    User(std::string  n, std::string  e) : name(std::move(n)), email(std::move(e)) {}
    User(const int i, std::string  n, std::string  e, std::string  c)
        : id(i), name(std::move(n)), email(std::move(e)), created_at(std::move(c)) {}

    // Convert to JSON string using jsoncpp (for backward compatibility)
    [[nodiscard]] std::string toJsonString() const {
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
        userObj.AddMember("created_at", ::rapidjson::Value(created_at.c_str(), allocator),
                          allocator);

        return userObj;
    }
};

} // namespace rdws::types

