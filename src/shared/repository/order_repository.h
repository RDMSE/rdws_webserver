#pragma once

#include "common/database/idatabase.h"
#include "types/order.h"
#include <memory>
#include <vector>
#include <optional>

namespace rdws {
namespace services {
namespace orders {

/**
 * Repository class for order database operations
 * Handles all database interactions for order entities
 */
class OrderRepository {
private:
    std::shared_ptr<rdws::database::IDatabase> db_;

    /**
     * Convert database result row to Order object
     * @param result Database result containing order data
     * @return Order object created from database row
     */
    types::Order resultToOrder(rdws::database::IResultSet& result) const;

public:
    /**
     * Constructor with database dependency injection
     * @param db Database interface for order operations
     */
    explicit OrderRepository(std::shared_ptr<rdws::database::IDatabase> db);

    /**
     * Find all orders in the database
     * @return Vector of all orders
     */
    std::vector<types::Order> findAll();

    /**
     * Find order by ID
     * @param orderId ID of the order to find
     * @return Optional containing the order if found, nullopt otherwise
     */
    std::optional<types::Order> findById(int orderId);

    /**
     * Find all orders for a specific user
     * @param userId ID of the user whose orders to find
     * @return Vector of orders for the specified user
     */
    std::vector<types::Order> findByUserId(int userId);

    /**
     * Create a new order
     * @param order Order object to create (ID will be auto-generated)
     * @return Optional containing the created order with ID, nullopt if creation failed
     */
    std::optional<types::Order> create(const types::Order& order);

    /**
     * Update an existing order
     * @param order Order object with updated information
     * @return Optional containing the updated order if successful, nullopt otherwise
     */
    std::optional<types::Order> update(const types::Order& order);

    /**
     * Delete order by ID
     * @param orderId ID of the order to delete
     * @return True if deletion was successful, false otherwise
     */
    bool deleteById(int orderId);

    /**
     * Get total count of orders
     * @return Total number of orders in the database
     */
    int count();

    /**
     * Get count of orders for a specific user
     * @param userId ID of the user
     * @return Number of orders for the specified user
     */
    int countByUserId(int userId);

    /**
     * Update order status
     * @param orderId ID of the order to update
     * @param newStatus New status for the order
     * @return True if update was successful, false otherwise
     */
    bool updateStatus(int orderId, const std::string& newStatus);
};

} // namespace orders
} // namespace services
} // namespace rdws