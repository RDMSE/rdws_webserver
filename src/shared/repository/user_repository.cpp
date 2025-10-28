#include "user_repository.h"
#include <stdexcept>

namespace rdws::repository {

UserRepository::UserRepository(std::shared_ptr<rdws::database::IDatabase> database) 
    : db(std::move(database)) {
    if (!db) {
        throw std::invalid_argument("Database instance cannot be null");
    }
}

std::optional<rdws::types::User> UserRepository::findById(const int id) const {
    try {
        const auto query = "SELECT id, name, email, created_at FROM users WHERE id = $1";

        if (const auto result = db->execQuery(query, {std::to_string(id)}); result && result->next()) {
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
        const auto query = "SELECT id, name, email, created_at FROM users ORDER BY id";

        if (const auto result = db->execQuery(query)) {
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
        const auto query = "SELECT id, name, email, created_at FROM users WHERE email = $1";

        if (const auto result = db->execQuery(query, {email})) {
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
        const auto query = "INSERT INTO users (name, email) VALUES ($1, $2) RETURNING id";
        return db->execCommand(query, {user.name, user.email});
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create user: " + std::string(e.what()));
    }
}

bool UserRepository::update(const rdws::types::User& user) const {
    try {
        const auto query = "UPDATE users SET name = $1, email = $2 WHERE id = $3";
        return db->execCommand(query, {user.name, user.email, std::to_string(user.id)});
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update user: " + std::string(e.what()));
    }
}

bool UserRepository::deleteById(const int id) const {
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

        for (const auto& [id, name, email, created_at] : users) {
            queries.emplace_back("INSERT INTO users (name, email) VALUES ($1, $2) RETURNING id");
            parameterSets.push_back({name, email});
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
        
        for (const auto& [id, name, email, created_at] : users) {
            queries.emplace_back("UPDATE users SET name = $1, email = $2 WHERE id = $3");
            parameterSets.push_back({name, email, std::to_string(id)});
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
        const auto query = "SELECT id, name, email, created_at FROM users ORDER BY id";
        if (const auto result = db->execQuery(query)) {
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
        const auto query = "SELECT id, name, email, created_at FROM users WHERE " + whereClause + " ORDER BY id";

        if (const auto result = db->execQuery(query, parameters)) {
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
        const auto query = "SELECT COUNT(*) as total FROM users";
        if (const auto result = db->execQuery(query); result && result->next()) {
            return static_cast<size_t>(result->getInt("total"));
        }
        
        return 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to count users: " + std::string(e.what()));
    }
}

bool UserRepository::exists(const int id) const {
    try {
        const auto query = "SELECT 1 FROM users WHERE id = $1 LIMIT 1";
        const auto result = db->execQuery(query, {std::to_string(id)});
        return result && result->next();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check user existence: " + std::string(e.what()));
    }
}

bool UserRepository::existsByEmail(const std::string& email) const {
    try {
        const auto query = "SELECT 1 FROM users WHERE email = $1 LIMIT 1";
        const auto result = db->execQuery(query, {email});
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
} // namespace rdws::repository
