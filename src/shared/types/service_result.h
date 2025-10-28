#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Forward declarations

namespace rdws::types {
class User;
class Order;
} // namespace rdws::types

namespace rdws::types {

/**
 * Generic result type for service operations
 * Encapsulates success/error states without JSON serialization
 */
template <typename T> class ServiceResult {
  public:
    // Success constructors
    static ServiceResult<T> success(const T& data) {
        return ServiceResult<T>(data, true, "", 200);
    }

    static ServiceResult<T> success(T&& data) {
        return ServiceResult<T>(std::move(data), true, "", 200);
    }

    // Error constructors
    static ServiceResult<T> error(const std::string& message, int statusCode = 500) {
        return ServiceResult<T>(std::nullopt, false, message, statusCode);
    }

    // Accessors
    [[nodiscard]] bool isSuccess() const {
        return success_;
    }
    [[nodiscard]] bool isError() const {
        return !success_;
    }

    const T& getData() const {
        if (!data_.has_value()) {
            throw std::runtime_error("No data available in error result");
        }
        return data_.value();
    }

    [[nodiscard]] const std::string& getErrorMessage() const {
        return errorMessage_;
    }
    [[nodiscard]] int getStatusCode() const {
        return statusCode_;
    }

    // Optional-like access
    [[nodiscard]] bool hasData() const {
        return data_.has_value();
    }
    const std::optional<T>& getOptionalData() const {
        return data_;
    }

  private:
    ServiceResult(std::optional<T> data, const bool success, std::string errorMessage,
                  const int statusCode)
        : data_(std::move(data)), success_(success), errorMessage_(std::move(errorMessage)),
          statusCode_(statusCode) {}

    std::optional<T> data_;
    bool success_;
    std::string errorMessage_;
    int statusCode_;
};

// Specialized result types for Users
using UserResult = ServiceResult<rdws::types::User>;
using UsersResult = ServiceResult<std::vector<rdws::types::User>>;

// Specialized result types for Orders
using OrderResult = ServiceResult<rdws::types::Order>;
using OrdersResult = ServiceResult<std::vector<rdws::types::Order>>;

// General result types
using CountResult = ServiceResult<size_t>;

// For operations that don't return data (like delete)
struct OperationStatus {
    bool success;
    std::string message;
    int statusCode;

    static OperationStatus createSuccess(const std::string& message = "") {
        return {.success = true, .message = message, .statusCode = 200};
    }

    static OperationStatus createError(const std::string& message, const int statusCode = 500) {
        return {.success = false, .message = message, .statusCode = statusCode};
    }
};

using OperationResult = ServiceResult<OperationStatus>;

} // namespace rdws::types
