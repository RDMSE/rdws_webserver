#pragma once

#include "common/database/idatabase.h"
#include "repository/user_repository.h"
#include "types/user.h"
#include "types/service_result.h"
#include <memory>
#include <string>

namespace rdws {
namespace users {

/**
 * UserService - Pure business logic layer
 * Returns structured data instead of JSON for better separation of concerns
 */
class UserService {
private:
    rdws::repository::UserRepository userRepository;

public:
    explicit UserService(std::shared_ptr<rdws::database::IDatabase> db);

    // Business logic methods returning structured data
    rdws::types::UsersResult getAllUsers();
    rdws::types::UserResult getUserById(int id);
    rdws::types::CountResult getUsersCount();
    rdws::types::UserResult createUser(const std::string& jsonData);
    rdws::types::UserResult updateUser(int id, const std::string& jsonData);
    rdws::types::OperationResult deleteUser(int id);
};

} // namespace users
} // namespace rdws
