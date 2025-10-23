#include "../../shared/types/lambda_event.h"
#include "../../shared/types/lambda_context.h"
#include "../../shared/common/database/postgresql_database.h"
#include "order_service.h"

#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

using namespace rdws::types;
using namespace rdws::database;
using namespace rdws::services::orders;

int main(int argc, char* argv[]) {
    try {
        LambdaEvent event("GET", "/", ""); // Initialize with defaults
        LambdaContext context("unknown", "orders-service"); // Initialize with defaults

        // Check if we have JSON parameters (new API Gateway approach)
        if (argc >= 3) {
            // New approach: API Gateway passes JSON strings
            std::string eventJson = argv[1];
            std::string contextJson = argv[2];
            
            try {
                event = LambdaEvent::fromJson(eventJson);
                context = LambdaContext::fromJson(contextJson);
            } catch (const std::exception& e) {
                // Fallback to old approach if JSON parsing fails
                event = LambdaEvent(argc, argv);
                context = LambdaContext(event.getRequestContext().requestId, "orders-service");
            }
        } else {
            // Fallback to old approach for backwards compatibility
            event = LambdaEvent(argc, argv);
            context = LambdaContext(event.getRequestContext().requestId, "orders-service");
        }

        context.log("Function started", "INFO");

        // Initialize database connection
        auto db = std::make_shared<rdws::database::PostgreSQLDatabase>();
        if (!db->isConnected()) {
            context.log("Failed to connect to database", "ERROR");
            std::cerr << "{\"error\":\"Failed to connect to database\"}" << std::endl;
            return 1;
        }

        // Initialize order service
        OrderService orderService(db);

        // Extract path parameters for routes like /orders/{id} or /users/{userId}/orders
        if (event.pathMatches("/orders/{id}")) {
            event.extractPathParameters("/orders/{id}");
        } else if (event.pathMatches("/users/{userId}/orders")) {
            event.extractPathParameters("/users/{userId}/orders");
        }

        context.log("Processing " + event.getHttpMethod() + " request to " + event.getPath(), "INFO");

        // Process request based on method and path
        if (event.isGet()) {
            if (event.pathMatches("/orders") || event.pathMatches("/")) {
                // List all orders
                context.log("Fetching all orders", "INFO");
                auto orders = orderService.getAllOrders();
                
                // Convert to JSON response format similar to original
                std::ostringstream oss;
                oss << "{\"orders\":[";
                for (size_t i = 0; i < orders.size(); ++i) {
                    rapidjson::Document doc;
                    doc.SetObject();
                    auto orderJson = orders[i].toJson(doc.GetAllocator());
                    
                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    orderJson.Accept(writer);
                    
                    oss << buffer.GetString();
                    if (i < orders.size() - 1) {
                        oss << ",";
                    }
                }
                oss << "],\"total\":" << orders.size() 
                    << ",\"source\":\"orders_service C++ executable\",\"endpoint\":\"/orders\""
                    << ",\"timestamp\":" << std::time(nullptr) << "}";
                
                std::cout << oss.str() << std::endl;
                return 0;
                
            } else if (event.pathMatches("/orders/{id}")) {
                // Fetch specific order
                std::string idParam = event.getPathParameter("id");
                
                try {
                    int orderId = std::stoi(idParam);
                    context.log("Fetching order with ID: " + std::to_string(orderId), "INFO");
                    
                    auto order = orderService.getOrderById(orderId);
                    if (order.has_value()) {
                        rapidjson::Document doc;
                        doc.SetObject();
                        auto orderJson = order->toJson(doc.GetAllocator());
                        
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        orderJson.Accept(writer);
                        
                        std::ostringstream oss;
                        oss << "{\"order\":" << buffer.GetString()
                            << ",\"source\":\"orders_service C++ executable\""
                            << ",\"endpoint\":\"" << event.getPath() << "\""
                            << ",\"timestamp\":" << std::time(nullptr) << "}";
                        
                        std::cout << oss.str() << std::endl;
                        return 0;
                    } else {
                        context.log("Order not found: " + std::to_string(orderId), "WARN");
                        std::cout << "{\"error\":\"Order not found\",\"orderId\":" << orderId
                                  << ",\"source\":\"orders_service C++ executable\"}" << std::endl;
                        return 1;
                    }
                } catch (const std::exception& e) {
                    context.log("Invalid order ID: " + idParam, "ERROR");
                    std::cout << "{\"error\":\"Invalid order ID\",\"path\":\"" << event.getPath()
                              << "\",\"source\":\"orders_service C++ executable\"}" << std::endl;
                    return 1;
                }
                
            } else if (event.pathMatches("/users/{userId}/orders")) {
                // Handle /users/{userId}/orders - Get orders for a specific user
                std::string userIdParam = event.getPathParameter("userId");
                
                try {
                    int userId = std::stoi(userIdParam);
                    context.log("Fetching orders for user ID: " + std::to_string(userId), "INFO");
                    
                    auto userOrders = orderService.getOrdersByUserId(userId);
                    
                    std::ostringstream oss;
                    oss << "{\"userId\":" << userId << ",\"orders\":[";
                    
                    for (size_t i = 0; i < userOrders.size(); ++i) {
                        rapidjson::Document doc;
                        doc.SetObject();
                        auto orderJson = userOrders[i].toJson(doc.GetAllocator());
                        
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        orderJson.Accept(writer);
                        
                        oss << buffer.GetString();
                        if (i < userOrders.size() - 1) {
                            oss << ",";
                        }
                    }
                    
                    oss << "],\"total\":" << userOrders.size()
                        << ",\"source\":\"orders_service C++ executable\",\"endpoint\":\"" << event.getPath()
                        << "\",\"timestamp\":" << std::time(nullptr) << "}";
                    
                    std::cout << oss.str() << std::endl;
                    return 0;
                } catch (const std::exception& e) {
                    context.log("Invalid user ID: " + userIdParam, "ERROR");
                    std::cout << "{\"error\":\"Invalid user ID or path format\",\"path\":\""
                              << event.getPath() << "\",\"source\":\"orders_service C++ executable\"}" << std::endl;
                    return 1;
                }
            }
        } else if (event.isPost()) {
            if (event.pathMatches("/orders") || event.pathMatches("/")) {
                // Create order
                std::string jsonData = event.getBody();
                
                if (jsonData.empty()) {
                    context.log("No JSON data provided for order creation", "ERROR");
                    std::cout << "{\"error\":\"No JSON data provided for order creation\"}" << std::endl;
                    return 1;
                }

                context.log("Creating new order", "INFO");
                
                // Parse JSON and create order
                if (event.hasJsonBody()) {
                    const auto& json = event.getJsonBody();
                    if (json.HasMember("userId") && json.HasMember("product") && 
                        json.HasMember("amount") && json.HasMember("status")) {
                        
                        Order newOrder(
                            json["userId"].GetInt(),
                            json["product"].GetString(),
                            json["amount"].GetDouble(),
                            json["status"].GetString()
                        );
                        
                        auto createdOrder = orderService.createOrder(newOrder);
                        if (createdOrder.has_value()) {
                            rapidjson::Document doc;
                            doc.SetObject();
                            auto orderJson = createdOrder->toJson(doc.GetAllocator());
                            
                            rapidjson::StringBuffer buffer;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                            orderJson.Accept(writer);
                            
                            std::cout << "{\"order\":" << buffer.GetString()
                                      << ",\"source\":\"orders_service C++ executable\"}" << std::endl;
                            return 0;
                        }
                    }
                }
                
                std::cout << "{\"error\":\"Failed to create order\"}" << std::endl;
                return 1;
            }
        }

        // Method not supported
        context.log("Method not allowed: " + event.getHttpMethod() + " " + event.getPath(), "WARN");
        std::cout << "{\"error\":\"Method not allowed\",\"method\":\"" << event.getHttpMethod()
                  << "\",\"path\":\"" << event.getPath() << "\",\"source\":\"orders_service C++ executable\"}"
                  << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "{\"error\":\"Service error: " << e.what() << "\"}" << std::endl;
        return 1;
    }
}
