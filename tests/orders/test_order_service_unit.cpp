#include <gtest/gtest.h>
#include "../../src/services/orders/order_service.h"
#include "../mocks/mock_database.h"
#include "../mocks/mock_order_result_set.h"
#include <gmock/gmock.h>
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
    // Prepare mock result set with 4 orders
    std::vector<std::map<std::string, std::string>> rows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"user_id", "2"}, {"product", "Mouse Logitech"}, {"amount", "150.00"}, {"status", "pending"}, {"created_at", "2023-01-02"} },
        { {"id", "3"}, {"user_id", "1"}, {"product", "Monitor LG"}, {"amount", "1200.00"}, {"status", "shipped"}, {"created_at", "2023-01-03"} },
        { {"id", "4"}, {"user_id", "3"}, {"product", "Teclado Redragon"}, {"amount", "200.00"}, {"status", "cancelled"}, {"created_at", "2023-01-04"} }
    };
    auto mockResultSet = std::make_unique<rdws::testing::MockOrderResultSet>(rows);
    // Set up Google Mock to return our result set
    EXPECT_CALL(*mockDb, execQuery(testing::_, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSet))));

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
    orderService->clearOrders();

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
    std::vector<std::map<std::string, std::string>> rows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} }
    };
    auto mockResultSet = std::make_unique<rdws::testing::MockOrderResultSet>(rows);
    std::string query = "SELECT id, user_id, product, amount, status, created_at FROM orders WHERE id = $1";
    std::vector<std::string> params = {"1"};
    EXPECT_CALL(*mockDb, execQuery(query, params))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSet))));
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
    std::vector<std::map<std::string, std::string>> rows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} },
        { {"id", "4"}, {"user_id", "1"}, {"product", "Teclado Mecânico"}, {"amount", "300.00"}, {"status", "shipped"}, {"created_at", "2023-01-04"} }
    };
    auto mockResultSet = std::make_unique<rdws::testing::MockOrderResultSet>(rows);
    std::string query = "SELECT id, user_id, product, amount, status, created_at FROM orders WHERE user_id = $1 ORDER BY created_at DESC";
    std::vector<std::string> params = {"1"};
    EXPECT_CALL(*mockDb, execQuery(query, params))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSet))));
    auto orders = orderService->getOrdersByUserId(1);
    EXPECT_EQ(2, orders.size());
    for (const auto& order : orders) {
        EXPECT_EQ(1, order.userId);
    }
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
    std::vector<std::map<std::string, std::string>> rows = {
        { {"id", "5"}, {"user_id", "1"}, {"product", "Test Product"}, {"amount", "199.99"}, {"status", "pending"}, {"created_at", "2023-01-05"} }
    };
    auto mockResultSet = std::make_unique<rdws::testing::MockOrderResultSet>(rows);
    EXPECT_CALL(*mockDb, execQuery(testing::_, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSet))));
    auto createdOrder = orderService->createOrder(newOrder);
    ASSERT_TRUE(createdOrder.has_value());
    EXPECT_GT(createdOrder->id, 0);
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
    // Mock select for getOrderById (before update)
    std::vector<std::map<std::string, std::string>> selectRows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} }
    };
    std::vector<std::map<std::string, std::string>> updateRows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2600.00"}, {"status", "delivered"}, {"created_at", "2023-01-01"} }
    };
    // Sequence: select (before), update, select (after)
    EXPECT_CALL(*mockDb, execQuery(testing::_, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(selectRows))))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(updateRows))))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(updateRows))));
    auto existingOrder = orderService->getOrderById(1);
    ASSERT_TRUE(existingOrder.has_value());
    existingOrder->status = "delivered";
    existingOrder->amount = 2600.00;
    auto updatedOrder = orderService->updateOrder(*existingOrder);
    ASSERT_TRUE(updatedOrder.has_value());
    EXPECT_EQ(1, updatedOrder->id);
    EXPECT_EQ("delivered", updatedOrder->status);
    EXPECT_DOUBLE_EQ(2600.00, updatedOrder->amount);
    auto verifyOrder = orderService->getOrderById(1);
    ASSERT_TRUE(verifyOrder.has_value());
    EXPECT_EQ("delivered", verifyOrder->status);
    EXPECT_DOUBLE_EQ(2600.00, verifyOrder->amount);
}

// Test 15: Update order with invalid data
TEST_F(OrderServiceTest, UpdateOrderInvalidData) {
    rdws::types::Order invalidOrder(1, "", -100.0, "invalid_status");

    auto result = orderService->updateOrder(invalidOrder);
    EXPECT_FALSE(result.has_value());
}

// Test 16: Delete order successfully
TEST_F(OrderServiceTest, DeleteOrderSuccess) {
    // Mock select for getOrderById (before delete)
    std::vector<std::map<std::string, std::string>> selectRows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} }
    };
    std::vector<std::map<std::string, std::string>> emptyRows = {};
    // Sequence: select (before), delete, select (after)
    EXPECT_CALL(*mockDb, execQuery(testing::_, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(selectRows))))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(emptyRows))));
    EXPECT_CALL(*mockDb, execCommand(testing::_, testing::_))
        .WillOnce(testing::Return(true));
    auto order = orderService->getOrderById(1);
    ASSERT_TRUE(order.has_value());
    bool result = orderService->deleteOrder(1);
    EXPECT_TRUE(result);
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
    std::vector<std::map<std::string, std::string>> rows = {
        { {"total", "4"} }
    };
    auto mockResultSet = std::make_unique<rdws::testing::MockOrderResultSet>(rows);
    std::string query = "SELECT COUNT(*) as total FROM orders";
    EXPECT_CALL(*mockDb, execQuery(query, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSet))));
    int count = orderService->getOrderCount();
    EXPECT_EQ(4, count);
    // After delete, mock delete and count again
    std::string deleteQuery = "DELETE FROM orders WHERE id = $1";
    std::vector<std::string> deleteParams = {"1"};
    EXPECT_CALL(*mockDb, execCommand(deleteQuery, deleteParams))
        .WillOnce(testing::Return(true));
    std::vector<std::map<std::string, std::string>> rowsAfterDelete = {
        { {"total", "3"} }
    };
    auto mockResultSetAfterDelete = std::make_unique<rdws::testing::MockOrderResultSet>(rowsAfterDelete);
    EXPECT_CALL(*mockDb, execQuery(query, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::move(mockResultSetAfterDelete))));
    orderService->deleteOrder(1);
    count = orderService->getOrderCount();
    EXPECT_EQ(3, count);
}

// Test 19: Get order count by user ID
TEST_F(OrderServiceTest, GetOrderCountByUserIdSuccess) {
    std::vector<std::map<std::string, std::string>> rowsUser1 = { { {"total", "2"} } };
    std::vector<std::map<std::string, std::string>> rowsUser2 = { { {"total", "1"} } };
    std::vector<std::map<std::string, std::string>> rowsUser999 = { { {"total", "0"} } };
    std::string query = "SELECT COUNT(*) as total FROM orders WHERE user_id = $1";
    EXPECT_CALL(*mockDb, execQuery(query, std::vector<std::string>{"1"}))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(rowsUser1))));
    EXPECT_CALL(*mockDb, execQuery(query, std::vector<std::string>{"2"}))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(rowsUser2))));
    EXPECT_CALL(*mockDb, execQuery(query, std::vector<std::string>{"999"}))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(rowsUser999))));
    int count = orderService->getOrderCountByUserId(1);
    EXPECT_EQ(2, count);
    count = orderService->getOrderCountByUserId(2);
    EXPECT_EQ(1, count);
    count = orderService->getOrderCountByUserId(999);
    EXPECT_EQ(0, count);
}

// Test 20: Update order status successfully
TEST_F(OrderServiceTest, UpdateOrderStatusSuccess) {
    // Sequence: update, select (after)
    EXPECT_CALL(*mockDb, execCommand(testing::_, testing::_))
        .WillOnce(testing::Return(true));
    std::vector<std::map<std::string, std::string>> selectRows = {
        { {"id", "2"}, {"user_id", "2"}, {"product", "Mouse Logitech"}, {"amount", "150.00"}, {"status", "confirmed"}, {"created_at", "2023-01-02"} }
    };
    EXPECT_CALL(*mockDb, execQuery(testing::_, testing::_))
        .WillOnce(testing::Return(testing::ByMove(std::make_unique<rdws::testing::MockOrderResultSet>(selectRows))));
    bool result = orderService->updateOrderStatus(2, "confirmed");
    EXPECT_TRUE(result);
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
