#pragma once

#include <string>
#include <rapidjson/document.h>

namespace rdws {
namespace types {

class Order {
public:
    int id;
    int userId;
    std::string product;
    double amount;
    std::string status;
    std::string createdAt;

    // Constructors
    Order();
    Order(int userId, const std::string& product, double amount, const std::string& status = "pending");
    Order(int id, int userId, const std::string& product, double amount, const std::string& status, const std::string& createdAt);

    // JSON serialization
    rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;
    void fromJson(const rapidjson::Value& json);

    // Utility methods
    bool isValid() const;
    std::string toString() const;

    // Operators
    bool operator==(const Order& other) const;
    bool operator!=(const Order& other) const;
};

} // namespace types
} // namespace rdws