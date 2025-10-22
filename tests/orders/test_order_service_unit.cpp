#include <gtest/gtest.h>
#include "../../src/services/orders/order_service.h"
#include "../mocks/mock_database.h"
#include <memory>

class OrderServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockDb = std::make_shared<rdws::testing::MockDatabase>();
        orderService = std::make_unique<rdws::services::orders::OrderService>(mockDb);
    }

    void TearDown() override {
        mockDb->reset();
    }

    std::shared_ptr<rdws::testing::MockDatabase> mockDb;
    std::unique_ptr<rdws::services::orders::OrderService> orderService;
};

// Test 1: Get all orders successfully
TEST_F(OrderServiceTest, GetAllOrdersSuccess) {
    auto orders = orderService->getAllOrders();
    
    EXPECT_EQ(4, orders.size()); // 4 test orders initialized
    
    // Verify first order details (Laptop Dell)
    bool foundLaptop = false;
    for (const auto& order : orders) {
        if (order.product == "Laptop Dell") {
            foundLaptop = true;
            EXPECT_EQ(1, order.id);
            EXPECT_EQ(1, order.userId);
            EXPECT_DOUBLE_EQ(2500.00, order.amount);
            EXPECT_EQ("completed", order.status);
            break;
        }
    }
    EXPECT_TRUE(foundLaptop);
}

// Test 2: Get all orders with empty database
TEST_F(OrderServiceTest, GetAllOrdersEmpty) {
    mockDb->clearOrders();
    
    auto orders = orderService->getAllOrders();
    EXPECT_EQ(0, orders.size());
}

// Test 3: Get all orders with null database
TEST_F(OrderServiceTest, GetAllOrdersNullDatabase) {
    auto nullDbService = std::make_unique<rdws::services::orders::OrderService>(nullptr);
    
    auto orders = nullDbService->getAllOrders();
    EXPECT_EQ(0, orders.size());
}

// Test 4: Get order by ID successfully
TEST_F(OrderServiceTest, GetOrderByIdSuccess) {
    auto order = orderService->getOrderById(1);
    
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(1, order->id);
    EXPECT_EQ(1, order->userId);
    EXPECT_EQ("Laptop Dell", order->product);
    EXPECT_DOUBLE_EQ(2500.00, order->amount);
    EXPECT_EQ("completed", order->status);
}

// Test 5: Get order by ID not found
TEST_F(OrderServiceTest, GetOrderByIdNotFound) {
    auto order = orderService->getOrderById(999);
    EXPECT_FALSE(order.has_value());
}

// Test 6: Get order by invalid ID (zero)
TEST_F(OrderServiceTest, GetOrderByIdInvalidZero) {
    auto order = orderService->getOrderById(0);
    EXPECT_FALSE(order.has_value());
}

// Test 7: Get order by invalid ID (negative)
TEST_F(OrderServiceTest, GetOrderByIdInvalidNegative) {
    auto order = orderService->getOrderById(-1);
    EXPECT_FALSE(order.has_value());
}

// Test 8: Get orders by user ID successfully
TEST_F(OrderServiceTest, GetOrdersByUserIdSuccess) {
    auto orders = orderService->getOrdersByUserId(1);
    
    EXPECT_EQ(2, orders.size()); // User 1 has 2 orders
    
    // Verify all orders belong to user 1
    for (const auto& order : orders) {
        EXPECT_EQ(1, order.userId);
    }
    
    // Verify specific orders
    bool foundLaptop = false, foundKeyboard = false;
    for (const auto& order : orders) {
        if (order.product == "Laptop Dell") {
            foundLaptop = true;
            EXPECT_EQ("completed", order.status);
        } else if (order.product == "Teclado Mecânico") {
            foundKeyboard = true;
            EXPECT_EQ("shipped", order.status);
        }
    }
    EXPECT_TRUE(foundLaptop);
    EXPECT_TRUE(foundKeyboard);
}

// Test 9: Get orders by user ID with no orders
TEST_F(OrderServiceTest, GetOrdersByUserIdNoOrders) {
    auto orders = orderService->getOrdersByUserId(999);
    EXPECT_EQ(0, orders.size());
}

// Test 10: Get orders by invalid user ID
TEST_F(OrderServiceTest, GetOrdersByUserIdInvalid) {
    auto orders = orderService->getOrdersByUserId(-1);
    EXPECT_EQ(0, orders.size());
}

// Test 11: Create order successfully
TEST_F(OrderServiceTest, CreateOrderSuccess) {
    rdws::types::Order newOrder(1, "Test Product", 199.99, "pending");
    
    auto createdOrder = orderService->createOrder(newOrder);
    
    ASSERT_TRUE(createdOrder.has_value());
    EXPECT_GT(createdOrder->id, 0); // Should have auto-generated ID
    EXPECT_EQ(1, createdOrder->userId);
    EXPECT_EQ("Test Product", createdOrder->product);
    EXPECT_DOUBLE_EQ(199.99, createdOrder->amount);
    EXPECT_EQ("pending", createdOrder->status);
}

// Test 12: Create order with invalid data (empty product)
TEST_F(OrderServiceTest, CreateOrderInvalidEmptyProduct) {
    rdws::types::Order invalidOrder(1, "", 199.99, "pending");
    
    auto result = orderService->createOrder(invalidOrder);
    EXPECT_FALSE(result.has_value());
}

// Test 13: Create order with invalid data (negative amount)
TEST_F(OrderServiceTest, CreateOrderInvalidNegativeAmount) {
    rdws::types::Order invalidOrder(1, "Test Product", -100.0, "pending");
    
    auto result = orderService->createOrder(invalidOrder);
    EXPECT_FALSE(result.has_value());
}

// Test 14: Update order successfully
TEST_F(OrderServiceTest, UpdateOrderSuccess) {
    // First get an existing order
    auto existingOrder = orderService->getOrderById(1);
    ASSERT_TRUE(existingOrder.has_value());
    
    // Modify it
    existingOrder->status = "delivered";
    existingOrder->amount = 2600.00;
    
    auto updatedOrder = orderService->updateOrder(*existingOrder);
    
    ASSERT_TRUE(updatedOrder.has_value());
    EXPECT_EQ(1, updatedOrder->id);
    EXPECT_EQ("delivered", updatedOrder->status);
    EXPECT_DOUBLE_EQ(2600.00, updatedOrder->amount);
}

// Test 15: Update order with invalid data
TEST_F(OrderServiceTest, UpdateOrderInvalidData) {
    rdws::types::Order invalidOrder(1, "", -100.0, "invalid_status");
    
    auto result = orderService->updateOrder(invalidOrder);
    EXPECT_FALSE(result.has_value());
}

// Test 16: Delete order successfully
TEST_F(OrderServiceTest, DeleteOrderSuccess) {
    // Verify order exists first
    auto order = orderService->getOrderById(1);
    ASSERT_TRUE(order.has_value());
    
    // Delete it
    bool result = orderService->deleteOrder(1);
    EXPECT_TRUE(result);
    
    // Verify it's gone
    auto deletedOrder = orderService->getOrderById(1);
    EXPECT_FALSE(deletedOrder.has_value());
}

// Test 17: Delete order with invalid ID
TEST_F(OrderServiceTest, DeleteOrderInvalidId) {
    bool result = orderService->deleteOrder(-1);
    EXPECT_FALSE(result);
}

// Test 18: Get order count
TEST_F(OrderServiceTest, GetOrderCountSuccess) {
    int count = orderService->getOrderCount();
    EXPECT_EQ(4, count); // 4 test orders initialized
    
    // Delete one order and check count again
    orderService->deleteOrder(1);
    count = orderService->getOrderCount();
    EXPECT_EQ(3, count);
}

// Test 19: Get order count by user ID
TEST_F(OrderServiceTest, GetOrderCountByUserIdSuccess) {
    int count = orderService->getOrderCountByUserId(1);
    EXPECT_EQ(2, count); // User 1 has 2 orders
    
    count = orderService->getOrderCountByUserId(2);
    EXPECT_EQ(1, count); // User 2 has 1 order
    
    count = orderService->getOrderCountByUserId(999);
    EXPECT_EQ(0, count); // Non-existent user
}

// Test 20: Update order status successfully
TEST_F(OrderServiceTest, UpdateOrderStatusSuccess) {
    bool result = orderService->updateOrderStatus(2, "confirmed");
    EXPECT_TRUE(result);
    
    // Verify the status was updated
    auto order = orderService->getOrderById(2);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ("confirmed", order->status);
}

// Test 21: Update order status with invalid status
TEST_F(OrderServiceTest, UpdateOrderStatusInvalidStatus) {
    bool result = orderService->updateOrderStatus(1, "invalid_status");
    EXPECT_FALSE(result);
}

// Test 22: Update order status with invalid order ID
TEST_F(OrderServiceTest, UpdateOrderStatusInvalidOrderId) {
    bool result = orderService->updateOrderStatus(-1, "confirmed");
    EXPECT_FALSE(result);
}