#pragma once

#include "common/database/idatabase.h"
#include "repository/user_repository.h"
#include "types/service_result.h"

#include <memory>
#include <string>

namespace rdws::users {

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
    rdws::types::UsersResult getAllUsers() const;
    rdws::types::UserResult getUserById(int id) const;
    rdws::types::CountResult getUsersCount() const;
    rdws::types::UserResult createUser(const std::string& jsonData) const;
    rdws::types::UserResult updateUser(int id, const std::string& jsonData) const;
    rdws::types::OperationResult deleteUser(int id) const;
};

} // namespace rdws::users
