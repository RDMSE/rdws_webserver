#pragma once

#include "idatabase.h"
#include "../config/config.h"
#include <pqxx/pqxx>
#include <memory>

namespace rdws {
namespace database {

class PostgreSQLResultSet : public IResultSet {
private:
    pqxx::result result;
    size_t currentRow;
    
public:
    explicit PostgreSQLResultSet(pqxx::result res);
    
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

class PostgreSQLDatabase : public IDatabase {
private:
    rdws::Config config;
    std::unique_ptr<pqxx::connection> connection;
    std::unique_ptr<pqxx::work> currentTransaction;
    std::string lastError;
    
public:
    PostgreSQLDatabase(); // Default constructor
    explicit PostgreSQLDatabase(const rdws::Config& dbConfig);
    ~PostgreSQLDatabase();
    
    // Query execution
    std::unique_ptr<IResultSet> execQuery(
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

private:
    void ensureConnection();
    std::string formatQuery(const std::string& query, const std::vector<std::string>& parameters);
};

} // namespace database
} // namespace rdws