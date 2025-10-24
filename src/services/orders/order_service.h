#pragma once

#include "../../shared/common/database/idatabase.h"
#include "../../shared/types/order.h"
#include <memory>
#include <vector>
#include <optional>

namespace rdws {
namespace services {
namespace orders {

/**
 * Service class for managing order operations
 * Provides business logic layer for order management with dependency injection
 */
class OrderService {
private:
    std::shared_ptr<rdws::database::IDatabase> db_;

public:
    /**
     * Constructor with dependency injection
     * @param db Database interface for order operations
     */
    explicit OrderService(std::shared_ptr<rdws::database::IDatabase> db);

    /**
     * Get all orders from the database
     * @return Vector of all orders
     */
    std::vector<types::Order> getAllOrders();

    /**
     * Get a specific order by ID
     * @param orderId ID of the order to retrieve
     * @return Optional containing the order if found, nullopt otherwise
     */
    std::optional<types::Order> getOrderById(int orderId);

    /**
     * Get all orders for a specific user
     * @param userId ID of the user whose orders to retrieve
     * @return Vector of orders for the specified user
     */
    std::vector<types::Order> getOrdersByUserId(int userId);

    /**
     * Create a new order
     * @param order Order object to create (ID will be auto-generated)
     * @return Optional containing the created order with ID, nullopt if creation failed
     */
    std::optional<types::Order> createOrder(const types::Order& order);

    /**
     * Update an existing order
     * @param order Order object with updated information
     * @return Optional containing the updated order if successful, nullopt otherwise
     */
    std::optional<types::Order> updateOrder(const types::Order& order);

    /**
     * Delete an order by ID
     * @param orderId ID of the order to delete
     * @return True if deletion was successful, false otherwise
     */
    bool deleteOrder(int orderId);

    /**
     * Get count of all orders
     * @return Total number of orders in the database
     */
    int getOrderCount();

    /**
     * Get count of orders for a specific user
     * @param userId ID of the user
     * @return Number of orders for the specified user
     */
    int getOrderCountByUserId(int userId);

    /**
     * Update order status
     * @param orderId ID of the order to update
     * @param newStatus New status for the order
     * @return True if update was successful, false otherwise
     */
    bool updateOrderStatus(int orderId, const std::string& newStatus);

    /**
     * Clear all orders (for unit test purposes)
     */
    void clearOrders();
};

} // namespace orders
} // namespace services
} // namespace rdws