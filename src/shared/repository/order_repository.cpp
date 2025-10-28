#include "order_repository.h"
#include <stdexcept>
#include <sstream>

namespace rdws {
namespace services {
namespace orders {

OrderRepository::OrderRepository(std::shared_ptr<rdws::database::IDatabase> db) : db_(std::move(db)) {}

types::Order OrderRepository::resultToOrder(rdws::database::IResultSet& result) const {
    types::Order order;
    order.id = result.getInt("id");
    order.userId = result.getInt("user_id");
    order.product = result.getString("product");
    order.amount = result.getDouble("amount");
    order.status = result.getString("status");
    order.createdAt = result.getString("created_at");
    return order;
}

std::vector<types::Order> OrderRepository::findAll() {
    std::vector<types::Order> orders;
    
    if (!db_) return orders;
    
    std::string query = "SELECT id, user_id, product, amount, status, created_at FROM orders ORDER BY created_at DESC";
    auto result = db_->execQuery(query);
    
    if (!result) return orders;
    
    while (result->next()) {
        orders.push_back(resultToOrder(*result));
    }
    
    return orders;
}

std::optional<types::Order> OrderRepository::findById(int orderId) {
    if (!db_) return std::nullopt;
    
    std::string query = "SELECT id, user_id, product, amount, status, created_at FROM orders WHERE id = $1";
    auto result = db_->execQuery(query, {std::to_string(orderId)});
    
    if (!result || !result->next()) {
        return std::nullopt;
    }
    
    return resultToOrder(*result);
}

std::vector<types::Order> OrderRepository::findByUserId(int userId) {
    std::vector<types::Order> orders;
    
    if (!db_) return orders;
    
    std::string query = "SELECT id, user_id, product, amount, status, created_at FROM orders WHERE user_id = $1 ORDER BY created_at DESC";
    auto result = db_->execQuery(query, {std::to_string(userId)});
    
    if (!result) return orders;
    
    while (result->next()) {
        orders.push_back(resultToOrder(*result));
    }
    
    return orders;
}

std::optional<types::Order> OrderRepository::create(const types::Order& order) {
    if (!db_) return std::nullopt;
    
    std::string query = "INSERT INTO orders (user_id, product, amount, status) VALUES ($1, $2, $3, $4) RETURNING id, user_id, product, amount, status, created_at";
    
    std::vector<std::string> params = {
        std::to_string(order.userId),
        order.product,
        std::to_string(order.amount),
        order.status
    };
    
    auto result = db_->execQuery(query, params);
    
    if (!result || !result->next()) {
        return std::nullopt;
    }
    
    return resultToOrder(*result);
}

std::optional<types::Order> OrderRepository::update(const types::Order& order) {
    if (!db_) return std::nullopt;
    
    std::string query = "UPDATE orders SET user_id = $1, product = $2, amount = $3, status = $4 WHERE id = $5 RETURNING id, user_id, product, amount, status, created_at";
    
    std::vector<std::string> params = {
        std::to_string(order.userId),
        order.product,
        std::to_string(order.amount),
        order.status,
        std::to_string(order.id)
    };
    
    auto result = db_->execQuery(query, params);
    
    if (!result || !result->next()) {
        return std::nullopt;
    }
    
    return resultToOrder(*result);
}

bool OrderRepository::deleteById(int orderId) {
    if (!db_) return false;
    
    std::string query = "DELETE FROM orders WHERE id = $1";
    return db_->execCommand(query, {std::to_string(orderId)});
}

int OrderRepository::count() {
    if (!db_) return 0;
    
    std::string query = "SELECT COUNT(*) as total FROM orders";
    auto result = db_->execQuery(query);
    
    if (!result || !result->next()) {
        return 0;
    }
    
    return result->getInt("total");
}

int OrderRepository::countByUserId(int userId) {
    if (!db_) return 0;
    
    std::string query = "SELECT COUNT(*) as total FROM orders WHERE user_id = $1";
    auto result = db_->execQuery(query, {std::to_string(userId)});
    
    if (!result || !result->next()) {
        return 0;
    }
    
    return result->getInt("total");
}

bool OrderRepository::updateStatus(int orderId, const std::string& newStatus) {
    if (!db_) return false;
    
    std::string query = "UPDATE orders SET status = $1 WHERE id = $2";
    return db_->execCommand(query, {newStatus, std::to_string(orderId)});
}

} // namespace orders
} // namespace services
} // namespace rdws