#include <gtest/gtest.h>
#include "../common/test_helpers.h"
#include <regex>
#include <chrono>
#include <algorithm>
#include <vector>

class OrdersServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Path to the orders service executable
        servicePath = ExecutableTestHelper::getServiceExecutablePath("orders");

        // Verify the executable exists
        ASSERT_FALSE(servicePath.empty()) << "Orders service executable not found";
    }

    std::string servicePath;

    // Helper function to count orders in JSON response
    int countOrdersInResponse(const std::string &json)
    {
        std::regex pattern(R"("id":\s*(\d+))");
        std::sregex_iterator iter(json.begin(), json.end(), pattern);
        std::sregex_iterator end;
        return std::distance(iter, end);
    }

    // Helper function to check if order exists in response
    bool orderExistsInResponse(const std::string &json, int orderId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(orderId);
        return json.find(searchPattern) != std::string::npos;
    }

    // Helper function to extract order product by ID
    std::string extractOrderProduct(const std::string &json, int orderId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(orderId);
        size_t pos = json.find(searchPattern);
        if (pos != std::string::npos)
        {
            // Find the product field after the id
            size_t productPos = json.find("\"product\":\"", pos);
            if (productPos != std::string::npos)
            {
                productPos += 11; // Skip '"product":"'
                size_t endPos = json.find("\"", productPos);
                if (endPos != std::string::npos)
                {
                    return json.substr(productPos, endPos - productPos);
                }
            }
        }
        return "";
    }

    // Helper function to extract order amount
    double extractOrderAmount(const std::string &json, int orderId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(orderId);
        size_t pos = json.find(searchPattern);
        if (pos != std::string::npos)
        {
            // Find the amount field after the id
            size_t amountPos = json.find("\"amount\":", pos);
            if (amountPos != std::string::npos)
            {
                amountPos += 9; // Skip '"amount":'
                size_t endPos = json.find(",", amountPos);
                if (endPos == std::string::npos)
                {
                    endPos = json.find("}", amountPos);
                }
                if (endPos != std::string::npos)
                {
                    std::string amountStr = json.substr(amountPos, endPos - amountPos);
                    try
                    {
                        return std::stod(amountStr);
                    }
                    catch (const std::exception &)
                    {
                        return 0.0;
                    }
                }
            }
        }
        return 0.0;
    }

    // Helper function to extract order status
    std::string extractOrderStatus(const std::string &json, int orderId)
    {
        std::string searchPattern = "\"id\":" + std::to_string(orderId);
        size_t pos = json.find(searchPattern);
        if (pos != std::string::npos)
        {
            // Find the status field after the id
            size_t statusPos = json.find("\"status\":\"", pos);
            if (statusPos != std::string::npos)
            {
                statusPos += 10; // Skip '"status":"'
                size_t endPos = json.find("\"", statusPos);
                if (endPos != std::string::npos)
                {
                    return json.substr(statusPos, endPos - statusPos);
                }
            }
        }
        return "";
    }
};

// ========== BASIC FUNCTIONALITY TESTS ==========

// Test: Service responds successfully
TEST_F(OrdersServiceTest, ServiceRespondsSuccessfully)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Service should return output";
    EXPECT_TRUE(output.find("{") != std::string::npos) << "Output should contain JSON";
    EXPECT_TRUE(output.find("}") != std::string::npos) << "Output should be valid JSON";
}

// Test: Returns valid JSON with complete expected structure
TEST_F(OrdersServiceTest, ReturnsValidJsonStructure)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if it contains all required keys
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "orders")) << "Should contain 'orders' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "total")) << "Should contain 'total' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "source")) << "Should contain 'source' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "endpoint")) << "Should contain 'endpoint' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "timestamp")) << "Should contain 'timestamp' key";

    // Verify JSON is properly formatted
    EXPECT_TRUE(output.find("[") != std::string::npos) << "Should contain array of orders";
    EXPECT_TRUE(output.find("]") != std::string::npos) << "Should close array of orders";
}

// ========== DATA VALIDATION TESTS ==========

// Test: Returns orders with correct count
TEST_F(OrdersServiceTest, ReturnsCorrectOrderCount)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if the total is greater than 0
    std::string total = ExecutableTestHelper::extractJsonValue(output, "total");
    EXPECT_FALSE(total.empty()) << "Total should not be empty";

    int totalCount = std::stoi(total);
    EXPECT_GT(totalCount, 0) << "Should have at least one order";
    EXPECT_EQ(totalCount, 4) << "Should have exactly 4 orders as per mock data";

    // Verify actual count matches reported total
    int actualCount = countOrdersInResponse(output);
    EXPECT_EQ(actualCount, totalCount) << "Actual order count should match total field";
}

// Test: Orders have all required fields
TEST_F(OrdersServiceTest, OrdersHaveRequiredFields)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check if orders have all required fields
    EXPECT_TRUE(output.find("\"id\":") != std::string::npos) << "Orders should have id field";
    EXPECT_TRUE(output.find("\"userId\":") != std::string::npos) << "Orders should have userId field";
    EXPECT_TRUE(output.find("\"product\":") != std::string::npos) << "Orders should have product field";
    EXPECT_TRUE(output.find("\"amount\":") != std::string::npos) << "Orders should have amount field";
    EXPECT_TRUE(output.find("\"status\":") != std::string::npos) << "Orders should have status field";

    // Check for specific orders from mock data
    EXPECT_TRUE(orderExistsInResponse(output, 1)) << "Should contain order with ID 1";
    EXPECT_TRUE(orderExistsInResponse(output, 2)) << "Should contain order with ID 2";
    EXPECT_TRUE(orderExistsInResponse(output, 4)) << "Should contain order with ID 4";
}

// Test: Validates order data integrity and business logic
TEST_F(OrdersServiceTest, ValidatesOrderDataIntegrity)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check specific order data
    std::string order1Product = extractOrderProduct(output, 1);
    EXPECT_EQ(order1Product, "Laptop Dell") << "Order 1 should be Laptop Dell";

    double order1Amount = extractOrderAmount(output, 1);
    EXPECT_EQ(order1Amount, 2500.00) << "Order 1 amount should be 2500.00";

    std::string order1Status = extractOrderStatus(output, 1);
    EXPECT_EQ(order1Status, "completed") << "Order 1 should have completed status";

    // Verify no empty or null values in critical fields
    EXPECT_FALSE(output.find("\"product\":\"\"") != std::string::npos) << "Should not have empty products";
    EXPECT_FALSE(output.find("\"amount\":0") != std::string::npos) << "Should not have zero amounts";
    EXPECT_FALSE(output.find("\"status\":\"\"") != std::string::npos) << "Should not have empty status";
}

// Test: Validates order amounts are positive numbers
TEST_F(OrdersServiceTest, ValidatesOrderAmounts)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check various order amounts
    double order2Amount = extractOrderAmount(output, 2);
    EXPECT_EQ(order2Amount, 150.00) << "Order 2 amount should be 150.00";

    double order3Amount = extractOrderAmount(output, 3);
    EXPECT_EQ(order3Amount, 400.00) << "Order 3 amount should be 400.00";

    double order4Amount = extractOrderAmount(output, 4);
    EXPECT_EQ(order4Amount, 1200.00) << "Order 4 amount should be 1200.00";

    // All amounts should be positive
    EXPECT_GT(order2Amount, 0) << "All order amounts should be positive";
    EXPECT_GT(order3Amount, 0) << "All order amounts should be positive";
    EXPECT_GT(order4Amount, 0) << "All order amounts should be positive";
}

// Test: Validates order status values
TEST_F(OrdersServiceTest, ValidatesOrderStatuses)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check different order statuses
    std::string order2Status = extractOrderStatus(output, 2);
    EXPECT_EQ(order2Status, "pending") << "Order 2 should be pending";

    std::string order3Status = extractOrderStatus(output, 3);
    EXPECT_EQ(order3Status, "shipped") << "Order 3 should be shipped";

    std::string order4Status = extractOrderStatus(output, 4);
    EXPECT_EQ(order4Status, "completed") << "Order 4 should be completed";

    // Verify valid status values
    std::vector<std::string> validStatuses = {"pending", "shipped", "completed", "cancelled"};
    for (const auto &status : {order2Status, order3Status, order4Status})
    {
        bool isValid = std::find(validStatuses.begin(), validStatuses.end(), status) != validStatuses.end();
        EXPECT_TRUE(isValid) << "Status '" << status << "' should be a valid order status";
    }
}

// ========== METADATA AND ENDPOINT TESTS ==========

// Test: Returns correct service metadata
TEST_F(OrdersServiceTest, ReturnsCorrectMetadata)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    std::string source = ExecutableTestHelper::extractJsonValue(output, "source");
    std::string endpoint = ExecutableTestHelper::extractJsonValue(output, "endpoint");
    std::string timestamp = ExecutableTestHelper::extractJsonValue(output, "timestamp");

    EXPECT_EQ(source, "orders_service C++ executable") << "Source should identify the service";
    EXPECT_EQ(endpoint, "/orders") << "Endpoint should match request path";
    EXPECT_FALSE(timestamp.empty()) << "Should include timestamp";
    EXPECT_GT(std::stoll(timestamp), 0) << "Timestamp should be valid";
}

// ========== INDIVIDUAL ORDER TESTS ==========

// Test: Handles get order by valid ID
TEST_F(OrdersServiceTest, HandlesGetOrderByValidId)
{
    std::string command = servicePath + " \"GET\" \"/orders/1\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for valid order ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "order")) << "Should contain 'order' key for single order";
    EXPECT_TRUE(output.find("\"id\":1") != std::string::npos) << "Should return order with ID 1";
    EXPECT_TRUE(output.find("Laptop Dell") != std::string::npos) << "Should return correct product name";
}

// Test: Handles get order by another valid ID
TEST_F(OrdersServiceTest, HandlesGetOrderByValidIdThree)
{
    std::string command = servicePath + " \"GET\" \"/orders/3\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for valid order ID 3";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "order")) << "Should contain 'order' key";
    EXPECT_TRUE(output.find("\"id\":3") != std::string::npos) << "Should return order with ID 3";
    EXPECT_TRUE(output.find("Teclado Mecânico") != std::string::npos) << "Should return correct product name";
    EXPECT_TRUE(output.find("shipped") != std::string::npos) << "Should return correct status";
}

// Test: Handles non-existent order ID
TEST_F(OrdersServiceTest, HandlesNonExistentOrderId)
{
    std::string command = servicePath + " \"GET\" \"/orders/999\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error response for non-existent order";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("Order not found") != std::string::npos) << "Should indicate order not found";
    EXPECT_TRUE(output.find("999") != std::string::npos) << "Should include requested order ID";
}

// Test: Handles invalid order ID format
TEST_F(OrdersServiceTest, HandlesInvalidOrderIdFormat)
{
    std::string command = servicePath + " \"GET\" \"/orders/invalid\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error response for invalid ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("Invalid order ID") != std::string::npos) << "Should indicate invalid ID format";
}

// ========== METHOD AND PATH TESTS ==========

// Test: Handles unsupported HTTP methods
TEST_F(OrdersServiceTest, HandlesUnsupportedMethods)
{
    std::string command = servicePath + " \"PUT\" \"/orders/1\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error for unsupported method";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("Method not allowed") != std::string::npos) << "Should indicate method not allowed";
}

// Test: Handles root path
TEST_F(OrdersServiceTest, HandlesRootPath)
{
    std::string command = servicePath + " \"GET\" \"/\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should handle root path";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "orders")) << "Should return orders list for root path";
}

// ========== BUSINESS LOGIC TESTS ==========

// Test: Validates user relationships in orders
TEST_F(OrdersServiceTest, ValidatesUserRelationships)
{
    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    // Check that orders have valid user IDs
    EXPECT_TRUE(output.find("\"userId\":1") != std::string::npos) << "Should have orders for user 1";
    EXPECT_TRUE(output.find("\"userId\":2") != std::string::npos) << "Should have orders for user 2";
    EXPECT_TRUE(output.find("\"userId\":3") != std::string::npos) << "Should have orders for user 3";

    // User IDs should be positive integers
    std::regex userIdPattern(R"("userId":\s*(\d+))");
    std::sregex_iterator iter(output.begin(), output.end(), userIdPattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter)
    {
        int userId = std::stoi(iter->str().substr(iter->str().find(':') + 1));
        EXPECT_GT(userId, 0) << "User IDs should be positive";
    }
}

// ========== USER-ORDERS INTEGRATION TESTS ==========

// Test: Handles get orders by user ID
TEST_F(OrdersServiceTest, HandlesGetOrdersByUserId)
{
    std::string command = servicePath + " \"GET\" \"/users/1/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for valid user ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "userId")) << "Should contain 'userId' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "orders")) << "Should contain 'orders' key";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "total")) << "Should contain 'total' key";

    // User 1 should have 2 orders (ID 1 and 3)
    EXPECT_TRUE(output.find("\"userId\":1") != std::string::npos) << "Should show correct user ID";
    EXPECT_TRUE(output.find("\"total\":2") != std::string::npos) << "User 1 should have 2 orders";

    // Should contain both orders for user 1
    EXPECT_TRUE(output.find("Laptop Dell") != std::string::npos) << "Should contain user's first order";
    EXPECT_TRUE(output.find("Teclado Mecânico") != std::string::npos) << "Should contain user's second order";
}

// Test: Handles get orders by another user ID
TEST_F(OrdersServiceTest, HandlesGetOrdersByUserIdTwo)
{
    std::string command = servicePath + " \"GET\" \"/users/2/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for user ID 2";
    EXPECT_TRUE(output.find("\"userId\":2") != std::string::npos) << "Should show correct user ID";
    EXPECT_TRUE(output.find("\"total\":1") != std::string::npos) << "User 2 should have 1 order";
    EXPECT_TRUE(output.find("Mouse Logitech") != std::string::npos) << "Should contain user's order";
}

// Test: Handles user with no orders
TEST_F(OrdersServiceTest, HandlesUserWithNoOrders)
{
    std::string command = servicePath + " \"GET\" \"/users/999/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return response for user with no orders";
    EXPECT_TRUE(output.find("\"userId\":999") != std::string::npos) << "Should show correct user ID";
    EXPECT_TRUE(output.find("\"total\":0") != std::string::npos) << "Should show zero orders";
    EXPECT_TRUE(output.find("\"orders\":[]") != std::string::npos) << "Should have empty orders array";
}

// Test: Handles invalid user ID in user orders path
TEST_F(OrdersServiceTest, HandlesInvalidUserIdInOrdersPath)
{
    std::string command = servicePath + " \"GET\" \"/users/invalid/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should return error for invalid user ID";
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "error")) << "Should contain error message";
    EXPECT_TRUE(output.find("Invalid user ID") != std::string::npos) << "Should indicate invalid user ID";
}

// ========== EDGE CASES AND ERROR HANDLING ==========

// Test: Handles no arguments
TEST_F(OrdersServiceTest, HandlesNoArguments)
{
    std::string command = servicePath;
    std::string output = ExecutableTestHelper::executeCommand(command);

    EXPECT_FALSE(output.empty()) << "Should handle default arguments";
    // Should default to GET /orders
    EXPECT_TRUE(ExecutableTestHelper::jsonContainsKey(output, "orders")) << "Should default to orders list";
}

// Test: Performance - service responds quickly
TEST_F(OrdersServiceTest, ServicePerformance)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::string command = servicePath + " \"GET\" \"/orders\"";
    std::string output = ExecutableTestHelper::executeCommand(command);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_FALSE(output.empty()) << "Should return response";
    EXPECT_LT(duration.count(), 1000) << "Should respond within 1 second";
}
