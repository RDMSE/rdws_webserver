#include "mock_database.h"
#include <sstream>
#include <algorithm>

namespace rdws {
namespace testing {

// MockResultSet implementation
MockResultSet::MockResultSet(const std::vector<std::map<std::string, std::string>>& data, 
                             const std::vector<std::string>& columns)
    : rows(data), currentRowIndex(static_cast<size_t>(-1)), columnNames(columns) {
}

bool MockResultSet::next() {
    if (currentRowIndex + 1 < rows.size()) {
        currentRowIndex++;
        return true;
    }
    return false;
}

bool MockResultSet::previous() {
    if (currentRowIndex > 0) {
        currentRowIndex--;
        return true;
    }
    return false;
}

void MockResultSet::reset() {
    currentRowIndex = static_cast<size_t>(-1);
}

std::string MockResultSet::getString(const std::string& columnName) {
    if (currentRowIndex < rows.size() && rows[currentRowIndex].count(columnName)) {
        return rows[currentRowIndex].at(columnName);
    }
    return "";
}

int MockResultSet::getInt(const std::string& columnName) {
    std::string value = getString(columnName);
    return value.empty() ? 0 : std::stoi(value);
}

double MockResultSet::getDouble(const std::string& columnName) {
    std::string value = getString(columnName);
    return value.empty() ? 0.0 : std::stod(value);
}

bool MockResultSet::getBool(const std::string& columnName) {
    std::string value = getString(columnName);
    return value == "true" || value == "1";
}

bool MockResultSet::isNull(const std::string& columnName) {
    return getString(columnName).empty();
}

size_t MockResultSet::getColumnCount() {
    return columnNames.size();
}

std::vector<std::string> MockResultSet::getColumnNames() {
    return columnNames;
}

size_t MockResultSet::getRowCount() {
    return rows.size();
}

// MockDatabase implementation
MockDatabase::MockDatabase() : nextUserId(4), nextOrderId(5), connected(true) {
    initializeTestData();
}

void MockDatabase::initializeTestData() {
    users[1] = {{"id", "1"}, {"name", "John Doe"}, {"email", "john@example.com"}};
    users[2] = {{"id", "2"}, {"name", "Jane Smith"}, {"email", "jane@example.com"}};
    users[3] = {{"id", "3"}, {"name", "Bob Johnson"}, {"email", "bob@example.com"}};
    
    orders[1] = {{"id", "1"}, {"user_id", "1"}, {"product", "Laptop Dell"}, {"amount", "2500.00"}, {"status", "completed"}, {"created_at", "2024-01-01 10:00:00"}};
    orders[2] = {{"id", "2"}, {"user_id", "2"}, {"product", "Mouse Logitech"}, {"amount", "150.00"}, {"status", "pending"}, {"created_at", "2024-01-02 11:00:00"}};
    orders[3] = {{"id", "3"}, {"user_id", "1"}, {"product", "Teclado Mecânico"}, {"amount", "400.00"}, {"status", "shipped"}, {"created_at", "2024-01-03 12:00:00"}};
    orders[4] = {{"id", "4"}, {"user_id", "3"}, {"product", "Monitor 4K"}, {"amount", "1200.00"}, {"status", "completed"}, {"created_at", "2024-01-04 13:00:00"}};
}

std::vector<std::map<std::string, std::string>> MockDatabase::convertUsersToRows() {
    std::vector<std::map<std::string, std::string>> rows;
    for (const auto& user : users) {
        rows.push_back(user.second);
    }
    return rows;
}

std::vector<std::map<std::string, std::string>> MockDatabase::convertOrdersToRows() {
    std::vector<std::map<std::string, std::string>> rows;
    for (const auto& order : orders) {
        rows.push_back(order.second);
    }
    return rows;
}

std::unique_ptr<rdws::database::IResultSet> MockDatabase::execQuery(
    const std::string& query, 
    const std::vector<std::string>& parameters) {
    
    // Handle ORDER queries
    if (query.find("FROM orders") != std::string::npos || query.find("INSERT INTO orders") != std::string::npos || query.find("UPDATE orders") != std::string::npos) {
        // Handle INSERT with RETURNING
        if (query.find("INSERT INTO orders") != std::string::npos && query.find("RETURNING") != std::string::npos && parameters.size() >= 4) {
            int userId = std::stoi(parameters[0]);
            std::string product = parameters[1];
            double amount = std::stod(parameters[2]);
            std::string status = parameters[3];
            
            int createdId = nextOrderId; // Save the ID that will be used
            addOrder(createdId, userId, product, amount, status);
            
            std::vector<std::map<std::string, std::string>> orderData = {orders[createdId]};
            return std::make_unique<MockResultSet>(orderData, std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
        }
        
        // Handle UPDATE with RETURNING
        if (query.find("UPDATE orders") != std::string::npos && query.find("RETURNING") != std::string::npos && parameters.size() >= 5) {
            int userId = std::stoi(parameters[0]);
            std::string product = parameters[1];
            std::string amount = parameters[2];
            std::string status = parameters[3];
            int orderId = std::stoi(parameters[4]);
            
            if (orders.count(orderId)) {
                orders[orderId]["user_id"] = std::to_string(userId);
                orders[orderId]["product"] = product;
                orders[orderId]["amount"] = amount;
                orders[orderId]["status"] = status;
                
                std::vector<std::map<std::string, std::string>> orderData = {orders[orderId]};
                return std::make_unique<MockResultSet>(orderData, std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
            } else {
                return std::make_unique<MockResultSet>(std::vector<std::map<std::string, std::string>>{}, 
                                                      std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
            }
        }
        
        if (query.find("COUNT") != std::string::npos) {
            // Count orders
            if (query.find("WHERE user_id = $1") != std::string::npos && !parameters.empty()) {
                int userId = std::stoi(parameters[0]);
                int count = 0;
                for (const auto& order : orders) {
                    if (std::stoi(order.second.at("user_id")) == userId) {
                        count++;
                    }
                }
                std::vector<std::map<std::string, std::string>> countData = {
                    {{"total", std::to_string(count)}}
                };
                return std::make_unique<MockResultSet>(countData, std::vector<std::string>{"total"});
            } else {
                std::vector<std::map<std::string, std::string>> countData = {
                    {{"total", std::to_string(orders.size())}}
                };
                return std::make_unique<MockResultSet>(countData, std::vector<std::string>{"total"});
            }
        } else if (query.find("WHERE id = $1") != std::string::npos && !parameters.empty()) {
            // Find order by ID
            int id = std::stoi(parameters[0]);
            if (orders.count(id)) {
                std::vector<std::map<std::string, std::string>> orderData = {orders[id]};
                return std::make_unique<MockResultSet>(orderData, std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
            } else {
                return std::make_unique<MockResultSet>(std::vector<std::map<std::string, std::string>>{}, 
                                                      std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
            }
        } else if (query.find("WHERE user_id = $1") != std::string::npos && !parameters.empty()) {
            // Find orders by user ID
            int userId = std::stoi(parameters[0]);
            std::vector<std::map<std::string, std::string>> userOrders;
            for (const auto& order : orders) {
                if (std::stoi(order.second.at("user_id")) == userId) {
                    userOrders.push_back(order.second);
                }
            }
            return std::make_unique<MockResultSet>(userOrders, std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
        } else if (query.find("SELECT") != std::string::npos && query.find("FROM orders") != std::string::npos) {
            // Return all orders
            return std::make_unique<MockResultSet>(convertOrdersToRows(), std::vector<std::string>{"id", "user_id", "product", "amount", "status", "created_at"});
        }
    }
    
    // Handle USER queries (existing logic)
    if (query.find("COUNT") != std::string::npos && query.find("FROM users") != std::string::npos) {
        std::vector<std::map<std::string, std::string>> countData = {
            {{"total", std::to_string(users.size())}}
        };
        return std::make_unique<MockResultSet>(countData, std::vector<std::string>{"total"});
    }
    
    if (query.find("WHERE id = $1") != std::string::npos && !parameters.empty()) {
        int id = std::stoi(parameters[0]);
        if (users.count(id)) {
            std::vector<std::map<std::string, std::string>> userData = {users[id]};
            return std::make_unique<MockResultSet>(userData, std::vector<std::string>{"id", "name", "email"});
        } else {
            return std::make_unique<MockResultSet>(std::vector<std::map<std::string, std::string>>{}, 
                                                  std::vector<std::string>{"id", "name", "email"});
        }
    }
    
    // Default: return all users
    return std::make_unique<MockResultSet>(convertUsersToRows(), std::vector<std::string>{"id", "name", "email"});
}

bool MockDatabase::execCommand(
    const std::string& command, 
    const std::vector<std::string>& parameters) {
    
    // Handle ORDER commands
    if (command.find("INSERT INTO orders") != std::string::npos && parameters.size() >= 4) {
        // Insert order: user_id, product, amount, status
        int userId = std::stoi(parameters[0]);
        std::string product = parameters[1];
        double amount = std::stod(parameters[2]);
        std::string status = parameters[3];
        
        addOrder(nextOrderId, userId, product, amount, status);
        nextOrderId++;
        return true;
    }
    
    if (command.find("UPDATE orders") != std::string::npos) {
        if (command.find("SET status = $1") != std::string::npos && parameters.size() >= 2) {
            // Update order status
            std::string newStatus = parameters[0];
            int orderId = std::stoi(parameters[1]);
            if (orders.count(orderId)) {
                orders[orderId]["status"] = newStatus;
                return true;
            }
        } else if (parameters.size() >= 5) {
            // Full update: user_id, product, amount, status, id
            int userId = std::stoi(parameters[0]);
            std::string product = parameters[1];
            std::string amount = parameters[2];
            std::string status = parameters[3];
            int orderId = std::stoi(parameters[4]);
            
            if (orders.count(orderId)) {
                orders[orderId]["user_id"] = std::to_string(userId);
                orders[orderId]["product"] = product;
                orders[orderId]["amount"] = amount;
                orders[orderId]["status"] = status;
                return true;
            }
        }
        return true;
    }
    
    if (command.find("DELETE FROM orders") != std::string::npos && !parameters.empty()) {
        int id = std::stoi(parameters[0]);
        orders.erase(id);
        return true;
    }
    
    // Handle USER commands (existing logic)
    if (command.find("INSERT INTO users") != std::string::npos && parameters.size() >= 2) {
        // Extract name and email from parameters and add user
        std::string name = parameters[0];
        std::string email = parameters[1];
        addUser(nextUserId, name, email);
        nextUserId++;
        return true;
    }
    
    if (command.find("UPDATE users") != std::string::npos && parameters.size() >= 3) {
        // Update user: name, email, id
        std::string name = parameters[0];
        std::string email = parameters[1];
        int userId = std::stoi(parameters[2]);
        
        if (users.count(userId)) {
            users[userId]["name"] = name;
            users[userId]["email"] = email;
            return true;
        }
        return false; // User not found
    }
    
    if (command.find("DELETE FROM users") != std::string::npos && !parameters.empty()) {
        int id = std::stoi(parameters[0]);
        users.erase(id);
        return true;
    }
    
    return true;
}

bool MockDatabase::execBatch(
    const std::vector<std::string>& commands,
    const std::vector<std::vector<std::string>>& parameterSets) {
    return true;
}

void MockDatabase::beginTransaction() {
    // Mock implementation
}

void MockDatabase::commitTransaction() {
    // Mock implementation
}

void MockDatabase::rollbackTransaction() {
    // Mock implementation
}

bool MockDatabase::isConnected() {
    return connected;
}

void MockDatabase::connect() {
    connected = true;
}

void MockDatabase::disconnect() {
    connected = false;
}

std::string MockDatabase::getLastError() {
    return lastError;
}

// Test helper methods
void MockDatabase::reset() {
    users.clear();
    orders.clear();
    nextUserId = 1;
    nextOrderId = 1;
    connected = true;
    lastError = "";
    initializeTestData();
}

void MockDatabase::setConnectionStatus(bool status) {
    connected = status;
}

void MockDatabase::setLastError(const std::string& error) {
    lastError = error;
}

void MockDatabase::addUser(int id, const std::string& name, const std::string& email) {
    users[id] = {{"id", std::to_string(id)}, {"name", name}, {"email", email}};
    if (id >= nextUserId) {
        nextUserId = id + 1;
    }
}

bool MockDatabase::userExists(int id) {
    return users.count(id) > 0;
}

size_t MockDatabase::getUserCount() {
    return users.size();
}

void MockDatabase::clearUsers() {
    users.clear();
    nextUserId = 1;
}

// Order helper methods
void MockDatabase::addOrder(int id, int userId, const std::string& product, double amount, const std::string& status) {
    orders[id] = {
        {"id", std::to_string(id)}, 
        {"user_id", std::to_string(userId)}, 
        {"product", product}, 
        {"amount", std::to_string(amount)}, 
        {"status", status},
        {"created_at", "2024-01-01 10:00:00"}
    };
    if (id >= nextOrderId) {
        nextOrderId = id + 1;
    }
}

bool MockDatabase::orderExists(int id) {
    return orders.count(id) > 0;
}

size_t MockDatabase::getOrderCount() {
    return orders.size();
}

void MockDatabase::clearOrders() {
    orders.clear();
    nextOrderId = 1;
}

} // namespace testing
} // namespace rdws