#include "order.h"
#include <rapidjson/writer.h>
#include <sstream>
#include <iomanip>
#include <utility>


namespace rdws::types {

// Default constructor
Order::Order() : id(0), userId(0), product(""), amount(0.0), status("pending"), createdAt("") {}

// Constructor for new orders (without ID and timestamp)
Order::Order(int userId, std::string  product, double amount, std::string  status)
    : id(0), userId(userId), product(std::move(product)), amount(amount), status(std::move(status)){}

// Full constructor
Order::Order(const int id, const int userId, std::string  product, const double amount, std::string  status, std::string  createdAt)
    : id(id), userId(userId), product(std::move(product)), amount(amount), status(std::move(status)), createdAt(std::move(createdAt)) {}

// JSON serialization
rapidjson::Value Order::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value orderObj(rapidjson::kObjectType);
    
    orderObj.AddMember("id", rapidjson::Value(id), allocator);
    orderObj.AddMember("userId", rapidjson::Value(userId), allocator);
    orderObj.AddMember("product", rapidjson::Value(product.c_str(), allocator), allocator);
    orderObj.AddMember("amount", rapidjson::Value(amount), allocator);
    orderObj.AddMember("status", rapidjson::Value(status.c_str(), allocator), allocator);
    
    if (!createdAt.empty()) {
        orderObj.AddMember("createdAt", rapidjson::Value(createdAt.c_str(), allocator), allocator);
    }
    
    return orderObj;
}

void Order::fromJson(const rapidjson::Value& json) {
    if (json.HasMember("id") && json["id"].IsInt()) {
        id = json["id"].GetInt();
    }
    
    if (json.HasMember("userId") && json["userId"].IsInt()) {
        userId = json["userId"].GetInt();
    }
    
    if (json.HasMember("product") && json["product"].IsString()) {
        product = json["product"].GetString();
    }
    
    if (json.HasMember("amount") && json["amount"].IsNumber()) {
        amount = json["amount"].GetDouble();
    }
    
    if (json.HasMember("status") && json["status"].IsString()) {
        status = json["status"].GetString();
    }
    
    if (json.HasMember("createdAt") && json["createdAt"].IsString()) {
        createdAt = json["createdAt"].GetString();
    }
}

// Validation
bool Order::isValid() const {
    return userId > 0 && 
           !product.empty() && 
           amount >= 0.0 && 
           !status.empty() &&
           (status == "pending" || status == "confirmed" || status == "shipped" || status == "delivered" || status == "cancelled");
}

// String representation
std::string Order::toString() const {
    std::ostringstream oss;
    oss << "Order{id=" << id 
        << ", userId=" << userId 
        << ", product='" << product << "'"
        << ", amount=" << std::fixed << std::setprecision(2) << amount
        << ", status='" << status << "'"
        << ", createdAt='" << createdAt << "'}";
    return oss.str();
}

// Operators
bool Order::operator==(const Order& other) const {
    return id == other.id && 
           userId == other.userId && 
           product == other.product && 
           amount == other.amount && 
           status == other.status;
}

bool Order::operator!=(const Order& other) const {
    return !(*this == other);
}

} // namespace rdws::types
