#pragma once

#include "../../shared/common/database/idatabase.h"
#include "../../shared/repository/user_repository.h"
#include "../../shared/types/user.h"
#include "../../shared/common/utils/response_helper.h"
#include <memory>
#include <string>

namespace rdws {
namespace users {

class UserService {
private:
    rdws::repository::UserRepository userRepository;

public:
    explicit UserService(std::shared_ptr<rdws::database::IDatabase> db);
    
    std::string getAllUsers();
    std::string getUserById(int id);
    std::string getUsersCount();
    std::string createUser(const std::string& jsonData);
    std::string updateUser(int id, const std::string& jsonData);
    std::string deleteUser(int id);
};

} // namespace users
} // namespace rdws