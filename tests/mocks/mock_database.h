#pragma once

#include "../../src/shared/common/database/idatabase.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace rdws {
namespace testing {

// Mock implementation of IResultSet for testing
class MockResultSet : public rdws::database::IResultSet {
private:
    std::vector<std::map<std::string, std::string>> rows;
    size_t currentRowIndex;
    std::vector<std::string> columnNames;

public:
    MockResultSet(const std::vector<std::map<std::string, std::string>>& data, 
                  const std::vector<std::string>& columns);

    // Navigation
    bool next() override;
    bool previous() override;
    void reset() override;
    
    // Data access
    std::string getString(const std::string& columnName) override;
    int getInt(const std::string& columnName) override;
    double getDouble(const std::string& columnName) override;
    bool getBool(const std::string& columnName) override;
    bool isNull(const std::string& columnName) override;
    
    // Metadata
    size_t getColumnCount() override;
    std::vector<std::string> getColumnNames() override;
    size_t getRowCount() override;
};

// Mock implementation of IDatabase for testing
class MockDatabase : public rdws::database::IDatabase {
private:
    // Simulated database tables
    std::map<int, std::map<std::string, std::string>> users; // id -> user data
    std::map<int, std::map<std::string, std::string>> orders; // id -> order data
    int nextUserId;
    int nextOrderId;
    bool connected;
    std::string lastError;

    // Helper methods
    void initializeTestData();
    std::vector<std::map<std::string, std::string>> convertUsersToRows();
    std::vector<std::map<std::string, std::string>> convertOrdersToRows();

public:
    MockDatabase();
    virtual ~MockDatabase() = default;

    // Query execution
    std::unique_ptr<rdws::database::IResultSet> execQuery(
        const std::string& query, 
        const std::vector<std::string>& parameters = {}
    ) override;
    
    // Command execution
    bool execCommand(
        const std::string& command, 
        const std::vector<std::string>& parameters = {}
    ) override;
    
    // Batch operations
    bool execBatch(
        const std::vector<std::string>& commands,
        const std::vector<std::vector<std::string>>& parameterSets
    ) override;
    
    // Transaction management
    void beginTransaction() override;
    void commitTransaction() override;
    void rollbackTransaction() override;
    
    // Connection management
    bool isConnected() override;
    void connect() override;
    void disconnect() override;
    
    // Utility
    std::string getLastError() override;

    // Test helper methods
    void reset();
    void setConnectionStatus(bool status);
    void setLastError(const std::string& error);
    void addUser(int id, const std::string& name, const std::string& email);
    bool userExists(int id);
    size_t getUserCount();
    void clearUsers();
    
    // Order helper methods
    void addOrder(int id, int userId, const std::string& product, double amount, const std::string& status);
    bool orderExists(int id);
    size_t getOrderCount();
    void clearOrders();
};

} // namespace testing
} // namespace rdws