#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace rdws {
namespace database {

class IResultSet {
public:
    virtual ~IResultSet() = default;
    
    // Navigation
    virtual bool next() = 0;
    virtual bool previous() = 0;
    virtual void reset() = 0;
    
    // Data access
    virtual std::string getString(const std::string& columnName) = 0;
    virtual int getInt(const std::string& columnName) = 0;
    virtual double getDouble(const std::string& columnName) = 0;
    virtual bool getBool(const std::string& columnName) = 0;
    virtual bool isNull(const std::string& columnName) = 0;
    
    // Metadata
    virtual size_t getColumnCount() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual size_t getRowCount() = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    
    // Query execution
    virtual std::unique_ptr<IResultSet> execQuery(
        const std::string& query, 
        const std::vector<std::string>& parameters = {}
    ) = 0;
    
    // Command execution (INSERT, UPDATE, DELETE)
    virtual bool execCommand(
        const std::string& command, 
        const std::vector<std::string>& parameters = {}
    ) = 0;
    
    // Batch operations
    virtual bool execBatch(
        const std::vector<std::string>& commands,
        const std::vector<std::vector<std::string>>& parameterSets
    ) = 0;
    
    // Transaction management
    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;
    
    // Connection management
    virtual bool isConnected() = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    
    // Utility
    virtual std::string getLastError() = 0;
};

} // namespace database
} // namespace rdws