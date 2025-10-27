#pragma once

#include "../common/utils/response_helper.h"
#include "../types/order.h"
#include "../types/service_result.h"
#include "base_controller.h"

#include <string>
#include <vector>

namespace rdws {
namespace controllers {

/**
 * Order Controller for handling JSON serialization and HTTP response formatting
 * Separates presentation concerns from business logic
 */
class OrderController: public rdws::controllers::BaseController {
  public:
    /**
     * Format a successful orders list response
     * @param result ServiceResult containing vector of orders
     * @return JSON string response
     */
    static std::string formatOrdersResponse(const rdws::types::OrdersResult& result) {
        if (result.isError()) {
            return formatErrorResponse(result.getErrorMessage(), result.getStatusCode());
        }

        const auto& orders = result.getData();

        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Create orders array
        rapidjson::Value ordersArray(rapidjson::kArrayType);
        for (const auto& order : orders) {
            ordersArray.PushBack(order.toJson(allocator), allocator);
        }

        // Build response object
        doc.AddMember("success", true, allocator);
        doc.AddMember("orders", ordersArray, allocator);
        doc.AddMember("total", static_cast<int>(orders.size()), allocator);
        doc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        doc.AddMember("endpoint", "/orders", allocator);
        doc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    /**
     * Format a successful single order response
     * @param result ServiceResult containing a single order
     * @return JSON string response
     */
    static std::string formatOrderResponse(const rdws::types::OrderResult& result) {
        if (result.isError()) {
            return formatErrorResponse(result.getErrorMessage(), result.getStatusCode());
        }

        const auto& order = result.getData();

        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Build response object
        doc.AddMember("success", true, allocator);
        doc.AddMember("order", order.toJson(allocator), allocator);
        doc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        doc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    /**
     * Format a count response
     * @param result ServiceResult containing count data
     * @return JSON string response
     */
    static std::string formatCountResponse(const rdws::types::CountResult& result) {
        if (result.isError()) {
            return formatErrorResponse(result.getErrorMessage(), result.getStatusCode());
        }

        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Build response object
        doc.AddMember("success", true, allocator);
        doc.AddMember("count", static_cast<int>(result.getData()), allocator);
        doc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        doc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

    /**
     * Format an operation response (for create, update, delete operations)
     * @param result ServiceResult containing operation status
     * @return JSON string response
     */
    static std::string formatOperationResponse(const rdws::types::OperationResult& result) {
        if (result.isError()) {
            return formatErrorResponse(result.getErrorMessage(), result.getStatusCode());
        }

        const auto& status = result.getData();

        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Build response object
        doc.AddMember("success", status.success, allocator);
        doc.AddMember("message", rapidjson::Value(status.message.c_str(), allocator), allocator);
        doc.AddMember("statusCode", status.statusCode, allocator);
        doc.AddMember("source", "orders_service C++ with clean architecture", allocator);
        doc.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }
};

} // namespace controllers
} // namespace rdws
