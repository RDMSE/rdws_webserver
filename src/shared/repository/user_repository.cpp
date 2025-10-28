#include "user_repository.h"
#include <stdexcept>


namespace rdws::repository {

UserRepository::UserRepository(std::shared_ptr<rdws::database::IDatabase> database) 
    : db(std::move(database)) {
    if (!db) {
        throw std::invalid_argument("Database instance cannot be null");
    }
}

std::optional<rdws::types::User> UserRepository::findById(int id) const {
    try {
        auto result = db->execQuery("SELECT id, name, email, created_at FROM users WHERE id = $1", {std::to_string(id)});
        
        if (result && result->next()) {
            return mapResultToUser(*result);
        }
        
        return std::nullopt;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by ID: " + std::string(e.what()));
    }
}

std::vector<rdws::types::User> UserRepository::findAll() const {
    try {
        std::vector<rdws::types::User> users;

        if (auto result = db->execQuery("SELECT id, name, email, created_at FROM users ORDER BY id")) {
            while (result->next()) {
                users.push_back(mapResultToUser(*result));
            }
        }
        
        return users;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find all users: " + std::string(e.what()));
    }
}

std::vector<rdws::types::User> UserRepository::findByEmail(const std::string& email) const {
    try {
        std::vector<rdws::types::User> users;

        if (auto result = db->execQuery("SELECT id, name, email, created_at FROM users WHERE email = $1", {email})) {
            while (result->next()) {
                users.push_back(mapResultToUser(*result));
            }
        }
        
        return users;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find users by email: " + std::string(e.what()));
    }
}

bool UserRepository::create(const rdws::types::User& user) const {
    try {
        const auto parameters = userToParameters(user);
        return db->execCommand(buildInsertQuery(), parameters);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create user: " + std::string(e.what()));
    }
}

bool UserRepository::update(const rdws::types::User& user) const {
    try {
        const auto parameters = userToParametersWithId(user);
        return db->execCommand(buildUpdateQuery(), parameters);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update user: " + std::string(e.what()));
    }
}

bool UserRepository::deleteById(int id) const {
    try {
        return db->execCommand("DELETE FROM users WHERE id = $1", {std::to_string(id)});
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete user: " + std::string(e.what()));
    }
}

bool UserRepository::createBatch(const std::vector<rdws::types::User>& users) const {
    if (users.empty()) {
        return true;
    }
    
    try {
        std::vector<std::string> queries;
        std::vector<std::vector<std::string>> parameterSets;
        
        for (const auto& user : users) {
            queries.push_back(buildInsertQuery());
            parameterSets.push_back(userToParameters(user));
        }
        
        return db->execBatch(queries, parameterSets);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create users in batch: " + std::string(e.what()));
    }
}

bool UserRepository::updateBatch(const std::vector<rdws::types::User>& users) const {
    if (users.empty()) {
        return true;
    }
    
    try {
        std::vector<std::string> queries;
        std::vector<std::vector<std::string>> parameterSets;
        
        for (const auto& user : users) {
            queries.push_back(buildUpdateQuery());
            parameterSets.push_back(userToParametersWithId(user));
        }
        
        return db->execBatch(queries, parameterSets);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update users in batch: " + std::string(e.what()));
    }
}

bool UserRepository::deleteBatch(const std::vector<int>& ids) const {
    if (ids.empty()) {
        return true;
    }
    
    try {
        std::vector<std::string> queries;
        std::vector<std::vector<std::string>> parameterSets;
        
        for (const int id : ids) {
            queries.emplace_back("DELETE FROM users WHERE id = $1");
            parameterSets.push_back({std::to_string(id)});
        }
        
        return db->execBatch(queries, parameterSets);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete users in batch: " + std::string(e.what()));
    }
}

void UserRepository::findAllWithCallback(const std::function<void(const rdws::types::User&)>& callback) const {
    try {
        if (const auto result = db->execQuery("SELECT id, name, email, created_at FROM users ORDER BY id")) {
            while (result->next()) {
                auto user = mapResultToUser(*result);
                callback(user);
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to process users with callback: " + std::string(e.what()));
    }
}

void UserRepository::findByConditionWithCallback(
    const std::string& whereClause,
    const std::vector<std::string>& parameters,
    const std::function<void(const rdws::types::User&)>& callback
) const {
    try {
        std::string query = "SELECT id, name, email, created_at FROM users WHERE " + whereClause + " ORDER BY id";

        if (auto result = db->execQuery(query, parameters)) {
            while (result->next()) {
                auto user = mapResultToUser(*result);
                callback(user);
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to process users with condition callback: " + std::string(e.what()));
    }
}

size_t UserRepository::count() const {
    try {
        if (auto result = db->execQuery("SELECT COUNT(*) as total FROM users"); result && result->next()) {
            return static_cast<size_t>(result->getInt("total"));
        }
        
        return 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to count users: " + std::string(e.what()));
    }
}

bool UserRepository::exists(const int id) const {
    try {
        const auto result = db->execQuery("SELECT 1 FROM users WHERE id = $1 LIMIT 1", {std::to_string(id)});
        return result && result->next();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check user existence: " + std::string(e.what()));
    }
}

bool UserRepository::existsByEmail(const std::string& email) const {
    try {
        const auto result = db->execQuery("SELECT 1 FROM users WHERE email = $1 LIMIT 1", {email});
        return result && result->next();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check user existence by email: " + std::string(e.what()));
    }
}

// Private helper methods

rdws::types::User UserRepository::mapResultToUser(rdws::database::IResultSet& result) {
    rdws::types::User user;
    user.id = result.getInt("id");
    user.name = result.getString("name");
    user.email = result.getString("email");
    user.created_at = result.getString("created_at");
    return user;
}

std::string UserRepository::buildInsertQuery() {
    return "INSERT INTO users (name, email) VALUES ($1, $2) RETURNING id";
}

std::string UserRepository::buildUpdateQuery() {
    return "UPDATE users SET name = $1, email = $2 WHERE id = $3";
}

std::vector<std::string> UserRepository::userToParameters(const rdws::types::User& user) {
    return {user.name, user.email};
}

std::vector<std::string> UserRepository::userToParametersWithId(const rdws::types::User& user) {
    return {user.name, user.email, std::to_string(user.id)};
}

} // namespace rdws::repository
