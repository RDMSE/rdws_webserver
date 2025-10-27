#pragma once

#include "common/database/idatabase.h"
#include "types/order.h"
#include "types/service_result.h"
#include "order_repository.h"

#include <memory>
#include <optional>
#include <vector>

namespace rdws {
namespace services {
namespace orders {

/**
 * Service class for managing order operations
 * Provides business logic layer for order management with dependency injection
 */
class OrderService {
  private:
    OrderRepository orderRepository;

  public:
    /**
     * Constructor with dependency injection
     * @param db Database interface for order operations
     */
    explicit OrderService(std::shared_ptr<rdws::database::IDatabase> db);

    /**
     * Get all orders from the database
     * @return ServiceResult containing vector of all orders
     */
    rdws::types::OrdersResult getAllOrders();

    /**
     * Get a specific order by ID
     * @param orderId ID of the order to retrieve
     * @return ServiceResult containing the order if found, error otherwise
     */
    rdws::types::OrderResult getOrderById(int orderId);

    /**
     * Get all orders for a specific user
     * @param userId ID of the user whose orders to retrieve
     * @return ServiceResult containing vector of orders for the specified user
     */
    rdws::types::OrdersResult getOrdersByUserId(int userId);

    /**
     * Create a new order from JSON data
     * @param jsonData JSON string containing order data
     * @return ServiceResult containing the created order with ID, error if creation failed
     */
    rdws::types::OrderResult createOrder(const std::string& jsonData);

    /**
     * Update an existing order
     * @param orderId ID of the order to update
     * @param jsonData JSON string containing updated order data
     * @return ServiceResult containing the updated order if successful, error otherwise
     */
    rdws::types::OrderResult updateOrder(int orderId, const std::string& jsonData);

    /**
     * Delete an order by ID
     * @param orderId ID of the order to delete
     * @return ServiceResult containing operation status
     */
    rdws::types::OperationResult deleteOrder(int orderId);

    /**
     * Get count of all orders
     * @return ServiceResult containing total number of orders in the database
     */
    rdws::types::CountResult getOrderCount();

    /**
     * Get count of orders for a specific user
     * @param userId ID of the user
     * @return ServiceResult containing number of orders for the specified user
     */
    rdws::types::CountResult getOrderCountByUserId(int userId);
};

} // namespace orders
} // namespace services
} // namespace rdws
