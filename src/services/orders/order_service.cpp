#include "order_service.h"

#include "types/order.h"

#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <utility>

namespace rdws::services::orders {

OrderService::OrderService(std::shared_ptr<rdws::database::IDatabase> db)
    : orderRepository(std::move(db)) {}

rdws::types::OrdersResult OrderService::getAllOrders() {
    try {
        auto orders = orderRepository.findAll();
        return rdws::types::ServiceResult<std::vector<rdws::types::Order>>::success(orders);
    } catch (const std::exception& e) {
        std::cerr << "Error in getAllOrders: " << e.what() << std::endl;
        return rdws::types::ServiceResult<std::vector<rdws::types::Order>>::error(
            "Failed to retrieve orders: " + std::string(e.what()));
    }
}

rdws::types::OrderResult OrderService::getOrderById(int orderId) {
    try {
        if (orderId <= 0) {
            return rdws::types::ServiceResult<rdws::types::Order>::error("Invalid order ID");
        }

        auto order = orderRepository.findById(orderId);

        if (order.has_value()) {
            return rdws::types::ServiceResult<rdws::types::Order>::success(order.value());
        } else {
            return rdws::types::ServiceResult<rdws::types::Order>::error("Order not found");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in getOrderById: " << e.what() << std::endl;
        return rdws::types::ServiceResult<rdws::types::Order>::error("Failed to retrieve order: " +
                                                                     std::string(e.what()));
    }
}

rdws::types::OrdersResult OrderService::getOrdersByUserId(int userId) {
    try {
        if (userId <= 0) {
            return rdws::types::ServiceResult<std::vector<rdws::types::Order>>::error(
                "Invalid user ID");
        }

        auto orders = orderRepository.findByUserId(userId);
        return rdws::types::ServiceResult<std::vector<rdws::types::Order>>::success(orders);
    } catch (const std::exception& e) {
        std::cerr << "Error in getOrdersByUserId: " << e.what() << std::endl;
        return rdws::types::ServiceResult<std::vector<rdws::types::Order>>::error(
            "Failed to retrieve orders for user: " + std::string(e.what()));
    }
}

rdws::types::CountResult OrderService::getOrderCount() {
    try {
        int count = orderRepository.count();
        return rdws::types::ServiceResult<size_t>::success(static_cast<size_t>(count));
    } catch (const std::exception& e) {
        std::cerr << "Error in getOrderCount: " << e.what() << std::endl;
        return rdws::types::ServiceResult<size_t>::error("Failed to get order count: " +
                                                         std::string(e.what()));
    }
}

rdws::types::OrderResult OrderService::createOrder(const std::string& jsonData) {
    try {
        if (jsonData.empty()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Empty JSON data provided");
        }

        // Parse JSON
        rapidjson::Document doc;
        doc.Parse(jsonData.c_str());

        if (doc.HasParseError()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Invalid JSON format: " +
                std::string(rapidjson::GetParseError_En(doc.GetParseError())));
        }

        // Validate required fields
        if (!doc.HasMember("userId") || !doc["userId"].IsInt()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Missing or invalid userId field");
        }
        if (!doc.HasMember("product") || !doc["product"].IsString()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Missing or invalid product field");
        }
        if (!doc.HasMember("amount") || !doc["amount"].IsNumber()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Missing or invalid amount field");
        }
        if (!doc.HasMember("status") || !doc["status"].IsString()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Missing or invalid status field");
        }

        // Create Order object
        rdws::types::Order newOrder(doc["userId"].GetInt(), doc["product"].GetString(),
                                    doc["amount"].GetDouble(), doc["status"].GetString());

        // Save to database
        auto createdOrder = orderRepository.create(newOrder);

        if (createdOrder.has_value()) {
            return rdws::types::ServiceResult<rdws::types::Order>::success(createdOrder.value());
        } else {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Failed to create order in database");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in createOrder: " << e.what() << std::endl;
        return rdws::types::ServiceResult<rdws::types::Order>::error("Failed to create order: " +
                                                                     std::string(e.what()));
    }
}

rdws::types::OrderResult OrderService::updateOrder(int orderId, const std::string& jsonData) {
    try {
        if (orderId <= 0) {
            return rdws::types::ServiceResult<rdws::types::Order>::error("Invalid order ID");
        }

        if (jsonData.empty()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Empty JSON data provided");
        }

        // Parse JSON
        rapidjson::Document doc;
        doc.Parse(jsonData.c_str());

        if (doc.HasParseError()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Invalid JSON format: " +
                std::string(rapidjson::GetParseError_En(doc.GetParseError())));
        }

        // Get existing order first
        auto existingOrder = orderRepository.findById(orderId);

        if (!existingOrder.has_value()) {
            return rdws::types::ServiceResult<rdws::types::Order>::error("Order not found");
        }

        // Update fields if provided
        rdws::types::Order updatedOrder = existingOrder.value();

        if (doc.HasMember("product") && doc["product"].IsString()) {
            updatedOrder.product = doc["product"].GetString();
        }
        if (doc.HasMember("amount") && doc["amount"].IsNumber()) {
            updatedOrder.amount = doc["amount"].GetDouble();
        }
        if (doc.HasMember("status") && doc["status"].IsString()) {
            updatedOrder.status = doc["status"].GetString();
        }

        // Save updated order
        auto result = orderRepository.update(updatedOrder);

        if (result.has_value()) {
            return rdws::types::ServiceResult<rdws::types::Order>::success(result.value());
        } else {
            return rdws::types::ServiceResult<rdws::types::Order>::error(
                "Failed to update order in database");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in updateOrder: " << e.what() << std::endl;
        return rdws::types::ServiceResult<rdws::types::Order>::error("Failed to update order: " +
                                                                     std::string(e.what()));
    }
}

rdws::types::OperationResult OrderService::deleteOrder(int orderId) {
    try {
        if (orderId <= 0) {
            return rdws::types::ServiceResult<rdws::types::OperationStatus>::error(
                "Invalid order ID");
        }

        if (orderRepository.deleteById(orderId)) {
            return rdws::types::ServiceResult<rdws::types::OperationStatus>::success(
                rdws::types::OperationStatus::createSuccess("Order deleted successfully"));
        } else {
            return rdws::types::ServiceResult<rdws::types::OperationStatus>::error(
                "Failed to delete order");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in deleteOrder: " << e.what() << std::endl;
        return rdws::types::ServiceResult<rdws::types::OperationStatus>::error(
            "Failed to delete order: " + std::string(e.what()));
    }
}

rdws::types::CountResult OrderService::getOrderCountByUserId(int userId) {
    try {
        if (userId <= 0) {
            return rdws::types::ServiceResult<size_t>::error("Invalid user ID");
        }

        int count = orderRepository.countByUserId(userId);
        return rdws::types::ServiceResult<size_t>::success(static_cast<size_t>(count));
    } catch (const std::exception& e) {
        std::cerr << "Error in getOrderCountByUserId: " << e.what() << std::endl;
        return rdws::types::ServiceResult<size_t>::error("Failed to get order count for user: " +
                                                         std::string(e.what()));
    }
}

} // namespace rdws::services::orders
