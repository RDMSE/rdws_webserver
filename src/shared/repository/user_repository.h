#pragma once

#include "../common/database/idatabase.h"
#include "../types/user.h"
#include <vector>
#include <optional>
#include <functional>

namespace rdws {
namespace repository {

class UserRepository {
private:
    std::shared_ptr<rdws::database::IDatabase> db;

public:
    explicit UserRepository(std::shared_ptr<rdws::database::IDatabase> database);
    
    // Basic CRUD operations
    std::optional<rdws::types::User> findById(int id);
    std::vector<rdws::types::User> findAll();
    std::vector<rdws::types::User> findByEmail(const std::string& email);
    bool create(const rdws::types::User& user);
    bool update(const rdws::types::User& user);
    bool deleteById(int id);
    
    // Batch operations
    bool createBatch(const std::vector<rdws::types::User>& users);
    bool updateBatch(const std::vector<rdws::types::User>& users);
    bool deleteBatch(const std::vector<int>& ids);
    
    // Query with callback for large datasets
    void findAllWithCallback(std::function<void(const rdws::types::User&)> callback);
    void findByConditionWithCallback(
        const std::string& whereClause,
        const std::vector<std::string>& parameters,
        std::function<void(const rdws::types::User&)> callback
    );
    
    // Utility methods
    size_t count();
    bool exists(int id);
    bool existsByEmail(const std::string& email);

private:
    // Helper methods
    rdws::types::User mapResultToUser(rdws::database::IResultSet& result);
    std::string buildInsertQuery() const;
    std::string buildUpdateQuery() const;
    std::vector<std::string> userToParameters(const rdws::types::User& user) const;
    std::vector<std::string> userToParametersWithId(const rdws::types::User& user) const;
};

} // namespace repository
} // namespace rdws