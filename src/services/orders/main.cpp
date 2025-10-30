#include "common/database/postgresql_database.h"
#include "controllers/order_controller.h"
#include "order_service.h"
#include "types/lambda_context.h"
#include "types/lambda_event.h"

#include <iostream>
#include <memory>
#include <string>

#include "common/utils/lambda_params_helper.h"

using namespace rdws::types;
using namespace rdws::database;
using namespace rdws::services::orders;
using namespace rdws::controllers;

int main(int argc, char* argv[]) {
    try {
        if (rdws::utils::LambdaParamsHelper::checkParams(argc, argv)) {
            std::cerr << OrderController::formatUsageError() << std::endl;
        }

        rdws::utils::LambdaParams params{ .eventJson = argv[1], .contextJson = argv[2] };
        LambdaEvent event = LambdaEvent::fromJson(params.eventJson);
        // LambdaContext context = LambdaContext::fromJson(params.contextJson);

        context.log("Function started", "INFO");

        // Initialize database connection
        auto db = std::make_shared<rdws::database::PostgreSQLDatabase>();
        if (!db->isConnected()) {
            context.log("Failed to connect to database", "ERROR");
            std::cerr << OrderController::formatDatabaseError() << std::endl;
            return 1;
        }

        // Initialize order service and controller
        OrderService orderService(db);

        // Extract path parameters for routes like /orders/{id} or /users/{userId}/orders
        if (event.pathMatches("/orders/{id}") || event.pathMatches("/orders/{action}")) {
            event.extractPathParameters("/orders/{id}");
        } else if (event.pathMatches("/users/{userId}/orders")) {
            event.extractPathParameters("/users/{userId}/orders");
        }

        context.log("Processing " + event.getHttpMethod() + " request to " + event.getPath(),
                    "INFO");

        // Process request based on method and path
        if (event.isGet()) {
            if (event.pathMatches("/orders") || event.pathMatches("/")) {
                // List all orders
                context.log("Fetching all orders", "INFO");
                auto result = orderService.getAllOrders();
                std::cout << OrderController::formatOrdersResponse(result) << std::endl;
                return result.isSuccess() ? 0 : 1;
            } else if (event.pathMatches("/orders/{id}")) {
                // Fetch specific order or handle special actions
                std::string idParam = event.getPathParameter("id");

                if (idParam == "count") {
                    context.log("Getting order count", "INFO");
                    auto result = orderService.getOrderCount();
                    std::cout << OrderController::formatCountResponse(result) << std::endl;
                    return result.isSuccess() ? 0 : 1;
                }

                try {
                    int orderId = std::stoi(idParam);
                    context.log("Fetching order with ID: " + std::to_string(orderId), "INFO");
                    auto result = orderService.getOrderById(orderId);
                    std::cout << OrderController::formatOrderResponse(result) << std::endl;
                    return result.isSuccess() ? 0 : 1;
                } catch (...) {
                    context.log("Invalid order ID: " + idParam, "ERROR");
                    std::cout << OrderController::formatError("Invalid order ID", 400) << std::endl;
                    return 1;
                }
            } else if (event.pathMatches("/users/{userId}/orders")) {
                // Fetch orders for specific user
                std::string userIdParam = event.getPathParameter("userId");

                try {
                    int userId = std::stoi(userIdParam);
                    context.log("Fetching orders for user ID: " + std::to_string(userId), "INFO");
                    auto result = orderService.getOrdersByUserId(userId);
                    std::cout << OrderController::formatOrdersResponse(result) << std::endl;
                    return result.isSuccess() ? 0 : 1;
                } catch (...) {
                    context.log("Invalid user ID: " + userIdParam, "ERROR");
                    std::cout << OrderController::formatError("Invalid user ID", 400) << std::endl;
                    return 1;
                }
            }
        } else if (event.isPost()) {
            if (event.pathMatches("/orders") || event.pathMatches("/")) {
                // Create order
                const std::string& jsonData = event.getBody();

                if (jsonData.empty()) {
                    context.log("No JSON data provided for order creation", "ERROR");
                    std::cout << OrderController::formatNoDataProvidedError("order creation")
                              << std::endl;
                    return 1;
                }

                context.log("Creating new order", "INFO");
                auto result = orderService.createOrder(jsonData);
                std::cout << OrderController::formatOrderResponse(result) << std::endl;
                return result.isSuccess() ? 0 : 1;
            }
        } else if (event.isPut()) {
            if (event.pathMatches("/orders/{id}")) {
                std::string idParam = event.getPathParameter("id");

                try {
                    int orderId = std::stoi(idParam);
                    const std::string& jsonData = event.getBody();

                    if (jsonData.empty()) {
                        context.log("No JSON data provided for order update", "ERROR");
                        std::cout << OrderController::formatNoDataProvidedError("order update")
                                  << std::endl;
                        return 1;
                    }

                    context.log("Updating order with ID: " + std::to_string(orderId), "INFO");
                    auto result = orderService.updateOrder(orderId, jsonData);
                    std::cout << OrderController::formatOrderResponse(result) << std::endl;
                    return result.isSuccess() ? 0 : 1;
                } catch (...) {
                    context.log("Invalid order ID: " + idParam, "ERROR");
                    std::cout << OrderController::formatError("Invalid order ID", 400) << std::endl;
                    return 1;
                }
            }
        } else if (event.isDelete()) {
            if (event.pathMatches("/orders/{id}")) {
                std::string idParam = event.getPathParameter("id");

                try {
                    int orderId = std::stoi(idParam);
                    context.log("Deleting order with ID: " + std::to_string(orderId), "INFO");
                    auto result = orderService.deleteOrder(orderId);
                    std::cout << OrderController::formatOperationResponse(result) << std::endl;
                    return result.isSuccess() ? 0 : 1;
                } catch (...) {
                    context.log("Invalid order ID: " + idParam, "ERROR");
                    std::cout << OrderController::formatError("Invalid order ID", 400) << std::endl;
                    return 1;
                }
            }
        }

        // Method not supported
        context.log("Method not allowed: " + event.getHttpMethod() + " " + event.getPath(), "WARN");
        std::cout << OrderController::formatMethodNotAllowedError(event.getHttpMethod(),
                                                                  event.getPath())
                  << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << OrderController::formatServiceError(e.what()) << std::endl;
        return 1;
    }
}
