#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "../../src/services/orders/order_service.h"
#include "../../src/shared/controllers/order_controller.h"
#include "../mocks/mock_database.h"
#include "../mocks/mock_order_result_set.h"

class OrderServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockDb = std::make_shared<rdws::testing::MockDatabase>();
        orderService = std::make_unique<rdws::services::orders::OrderService>(mockDb);
        orderController = std::make_unique<rdws::controllers::OrderController>();

        // Configure mock database with default behaviors
        mockDb->reset();
    }

    void TearDown() override {
        mockDb->reset();
    }

    std::shared_ptr<rdws::testing::MockDatabase> mockDb;
    std::unique_ptr<rdws::services::orders::OrderService> orderService;
    std::unique_ptr<rdws::controllers::OrderController> orderController;
};

// Test that OrderService returns success result for getAllOrders
TEST_F(OrderServiceUnitTest, GetAllOrders_ServiceReturnsSuccessResult) {
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} },
        { {"id", "2"}, {"user_id", "2"}, {"product", "Mouse"}, {"amount", "150.00"}, {"status", "pending"}, {"created_at", "2023-01-02"} }
    };

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("SELECT"), testing::IsEmpty()))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(mockRows)));

    auto result = orderService->getAllOrders();

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(2, result.getData().size()) << "Should return 2 orders";
}

// Test that OrderController formats JSON correctly
TEST_F(OrderServiceUnitTest, GetAllOrders_ControllerFormatsJsonCorrectly) {
    std::vector<rdws::types::Order> orders = {
        rdws::types::Order(1, 1, "Laptop", 2500.00, "completed", "2023-01-01"),
        rdws::types::Order(2, 2, "Mouse", 150.00, "pending", "2023-01-02")
    };

    auto successResult = rdws::types::OrdersResult::success(orders);
    std::string json = orderController->formatOrdersResponse(successResult);

    EXPECT_FALSE(json.empty()) << "Controller should return non-empty JSON";
    EXPECT_TRUE(json.find("Laptop") != std::string::npos) << "JSON should contain order data";
    EXPECT_TRUE(json.find("success") != std::string::npos) << "JSON should indicate success";
}

// Test getOrderById with valid ID
TEST_F(OrderServiceUnitTest, GetOrderById_ValidId_ReturnsOrder) {
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Laptop"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2023-01-01"} }
    };

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("SELECT"), testing::ElementsAre("1")))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(mockRows)));

    auto result = orderService->getOrderById(1);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success for valid ID";
    EXPECT_EQ("Laptop", result.getData().product) << "Should return correct order";
}

// Test getOrderById with non-existent ID
TEST_F(OrderServiceUnitTest, GetOrderById_NonExistentId_ReturnsError) {
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> emptyRows = {};

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("SELECT"), testing::ElementsAre("999")))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(emptyRows)));

    auto result = orderService->getOrderById(999);

    EXPECT_FALSE(result.isSuccess()) << "Service should return error for non-existent ID";
    EXPECT_TRUE(result.getErrorMessage().find("not found") != std::string::npos)
        << "Error message should indicate order not found";
}

// Test getOrderCount
TEST_F(OrderServiceUnitTest, GetOrderCount_ReturnsCorrectCount) {
    using ::testing::Return;

    std::vector<std::map<std::string, std::string>> countRows = {
        { {"total", "5"} }
    };

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("COUNT"), testing::IsEmpty()))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(countRows)));

    auto result = orderService->getOrderCount();

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(5, result.getData()) << "Should return correct count";
}

// Test createOrder with valid JSON
TEST_F(OrderServiceUnitTest, CreateOrder_ValidData_CreatesOrder) {
    using ::testing::Return;
    using ::testing::_;

    std::string jsonData = R"({"userId": 1, "product": "New Product", "amount": 299.99, "status": "pending"})";

    // Mock the INSERT query with RETURNING clause
    std::vector<std::map<std::string, std::string>> mockRows = {
        { {"id", "4"}, {"user_id", "1"}, {"product", "New Product"}, {"amount", "299.99"}, {"status", "pending"}, {"created_at", "2023-01-04"} }
    };

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("INSERT INTO orders"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(mockRows)));

    auto result = orderService->createOrder(jsonData);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().product, "New Product") << "Should return created order";
}

// Test createOrder with invalid JSON
TEST_F(OrderServiceUnitTest, CreateOrder_InvalidJson_ReturnsError) {
    std::string invalidJson = "{invalid json}";

    auto result = orderService->createOrder(invalidJson);

    EXPECT_FALSE(result.isSuccess()) << "Service should return error";
    EXPECT_TRUE(result.getErrorMessage().find("Invalid JSON") != std::string::npos)
        << "Should return JSON error message";
}

// Test updateOrder
TEST_F(OrderServiceUnitTest, UpdateOrder_ValidData_UpdatesOrder) {
    using ::testing::Return;
    using ::testing::_;

    std::string jsonData = R"({"product": "Updated Product"})";

    // Mock finding existing order
    std::vector<std::map<std::string, std::string>> existingOrder = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Old Product"}, {"amount", "100.00"}, {"status", "pending"}, {"created_at", "2023-01-01"} }
    };

    EXPECT_CALL(*mockDb, execQuery("SELECT id, user_id, product, amount, status, created_at FROM orders WHERE id = $1",
                                   std::vector<std::string>{"1"}))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(existingOrder)));

    // Mock the UPDATE query with RETURNING clause
    std::vector<std::map<std::string, std::string>> updatedOrder = {
        { {"id", "1"}, {"user_id", "1"}, {"product", "Updated Product"}, {"amount", "100.00"}, {"status", "pending"}, {"created_at", "2023-01-01"} }
    };

    EXPECT_CALL(*mockDb, execQuery(testing::HasSubstr("UPDATE orders"), _))
        .WillOnce(Return(std::make_unique<rdws::testing::MockOrderResultSet>(updatedOrder)));

    auto result = orderService->updateOrder(1, jsonData);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().product, "Updated Product") << "Should return updated order";
}

// Test deleteOrder
TEST_F(OrderServiceUnitTest, DeleteOrder_ValidId_DeletesOrder) {
    using ::testing::Return;
    using ::testing::_;

    EXPECT_CALL(*mockDb, execCommand("DELETE FROM orders WHERE id = $1",
                                   std::vector<std::string>{"1"}))
        .WillOnce(Return(true));

    auto result = orderService->deleteOrder(1);

    EXPECT_TRUE(result.isSuccess()) << "Service should return success";
    EXPECT_EQ(result.getData().message, "Order deleted successfully") << "Should return success message";
}

// Test that controllers format error responses correctly
TEST_F(OrderServiceUnitTest, Controller_FormatsErrorResponseCorrectly) {
    auto errorResult = rdws::types::ServiceResult<rdws::types::Order>::error("Test error message");
    std::string json = orderController->formatOrderResponse(errorResult);

    EXPECT_FALSE(json.empty()) << "Controller should return non-empty JSON";
    EXPECT_TRUE(json.find("error") != std::string::npos) << "JSON should indicate error";
    EXPECT_TRUE(json.find("Test error message") != std::string::npos) << "JSON should contain error message";
}
