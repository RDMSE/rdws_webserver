#include "order_service.h"
#include "order_repository.h"
#ifdef UNIT_TEST
#include "tests/mocks/mock_database.h"
#endif
#include <iostream>

namespace rdws {
namespace services {
namespace orders {

OrderService::OrderService(std::shared_ptr<rdws::database::IDatabase> db) : orderRepository_(db) {}

std::vector<types::Order> OrderService::getAllOrders() {
    try {
        return orderRepository_.findAll();
    } catch (const std::exception& e) {
        std::cerr << "Error getting all orders: " << e.what() << std::endl;
        return {};
    }
}

std::optional<types::Order> OrderService::getOrderById(int orderId) {
    if (orderId <= 0) {
        std::cerr << "Invalid order ID: " << orderId << std::endl;
        return std::nullopt;
    }

    try {
        return orderRepository_.findById(orderId);
    } catch (const std::exception& e) {
        std::cerr << "Error getting order by ID " << orderId << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<types::Order> OrderService::getOrdersByUserId(int userId) {
    if (userId <= 0) {
        std::cerr << "Invalid user ID: " << userId << std::endl;
        return {};
    }

    try {
        return orderRepository_.findByUserId(userId);
    } catch (const std::exception& e) {
        std::cerr << "Error getting orders for user " << userId << ": " << e.what() << std::endl;
        return {};
    }
}

std::optional<types::Order> OrderService::createOrder(const types::Order& order) {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return std::nullopt;
    }

    if (!order.isValid()) {
        std::cerr << "Invalid order data: " << order.toString() << std::endl;
        return std::nullopt;
    }

    try {
        OrderRepository repository(db_);
        return repository.create(order);
    } catch (const std::exception& e) {
        std::cerr << "Error creating order: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<types::Order> OrderService::updateOrder(const types::Order& order) {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return std::nullopt;
    }

    if (!order.isValid() || order.id <= 0) {
        std::cerr << "Invalid order data for update: " << order.toString() << std::endl;
        return std::nullopt;
    }

    try {
        OrderRepository repository(db_);
        return repository.update(order);
    } catch (const std::exception& e) {
        std::cerr << "Error updating order: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool OrderService::deleteOrder(int orderId) {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return false;
    }

    if (orderId <= 0) {
        std::cerr << "Invalid order ID for deletion: " << orderId << std::endl;
        return false;
    }

    try {
        OrderRepository repository(db_);
        return repository.deleteById(orderId);
    } catch (const std::exception& e) {
        std::cerr << "Error deleting order " << orderId << ": " << e.what() << std::endl;
        return false;
    }
}

int OrderService::getOrderCount() {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return 0;
    }

    try {
        OrderRepository repository(db_);
        return repository.count();
    } catch (const std::exception& e) {
        std::cerr << "Error getting order count: " << e.what() << std::endl;
        return 0;
    }
}

int OrderService::getOrderCountByUserId(int userId) {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return 0;
    }

    if (userId <= 0) {
        std::cerr << "Invalid user ID: " << userId << std::endl;
        return 0;
    }

    try {
        OrderRepository repository(db_);
        return repository.countByUserId(userId);
    } catch (const std::exception& e) {
        std::cerr << "Error getting order count for user " << userId << ": " << e.what() << std::endl;
        return 0;
    }
}

bool OrderService::updateOrderStatus(int orderId, const std::string& newStatus) {
    if (!db_) {
        std::cerr << "Database connection is null" << std::endl;
        return false;
    }

    if (orderId <= 0) {
        std::cerr << "Invalid order ID: " << orderId << std::endl;
        return false;
    }

    if (newStatus.empty() || 
        (newStatus != "pending" && newStatus != "confirmed" && 
         newStatus != "shipped" && newStatus != "delivered" && newStatus != "cancelled")) {
        std::cerr << "Invalid order status: " << newStatus << std::endl;
        return false;
    }

    try {
        OrderRepository repository(db_);
        return repository.updateStatus(orderId, newStatus);
    } catch (const std::exception& e) {
        std::cerr << "Error updating order status: " << e.what() << std::endl;
        return false;
    }
}

// Método de teste para limpar pedidos
void OrderService::clearOrders() {
    #ifdef UNIT_TEST
    if (db_) {
        auto mockPtr = std::dynamic_pointer_cast<rdws::testing::MockDatabase>(db_);
        if (mockPtr) {
            mockPtr->clearOrders();
        }
    }
    #endif
}

} // namespace orders
} // namespace services
} // namespace rdws