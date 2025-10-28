#pragma once

#include "../common/database/idatabase.h"
#include "../types/user.h"
#include <vector>
#include <optional>
#include <functional>


namespace rdws::repository {

class UserRepository {
private:
    std::shared_ptr<rdws::database::IDatabase> db;

public:
    explicit UserRepository(std::shared_ptr<rdws::database::IDatabase> database);
    
    // Basic CRUD operations
    [[nodiscard]] std::optional<rdws::types::User> findById(int id) const;
    [[nodiscard]] std::vector<rdws::types::User> findAll() const;
    [[nodiscard]] std::vector<rdws::types::User> findByEmail(const std::string& email) const;
    [[nodiscard]] bool create(const rdws::types::User& user) const;
    [[nodiscard]] bool update(const rdws::types::User& user) const;
    [[nodiscard]] bool deleteById(int id) const;
    
    // Batch operations
    [[nodiscard]] bool createBatch(const std::vector<rdws::types::User>& users) const;
    [[nodiscard]] bool updateBatch(const std::vector<rdws::types::User>& users) const;
    [[nodiscard]] bool deleteBatch(const std::vector<int>& ids) const;
    
    // Query with callback for large datasets
    void findAllWithCallback(const std::function<void(const rdws::types::User&)>& callback) const;
    void findByConditionWithCallback(
        const std::string& whereClause,
        const std::vector<std::string>& parameters,
        const std::function<void(const rdws::types::User&)>& callback
    ) const;
    
    // Utility methods
    [[nodiscard]] size_t count() const;
    [[nodiscard]] bool exists(int id) const;
    [[nodiscard]] bool existsByEmail(const std::string& email) const;

private:
    // Helper methods
    static rdws::types::User mapResultToUser(rdws::database::IResultSet& result);
    [[nodiscard]]static std::string buildInsertQuery() ;
    [[nodiscard]] static std::string buildUpdateQuery() ;
    [[nodiscard]] static std::vector<std::string> userToParameters(const rdws::types::User& user) ;
    [[nodiscard]] static std::vector<std::string> userToParametersWithId(const rdws::types::User& user) ;
};

} // namespace rdws::repository
