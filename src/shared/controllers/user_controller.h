#pragma once

#include "../common/utils/response_helper.h"
#include "../types/service_result.h"
#include "../types/user.h"
#include "base_controller.h"

#include <string>

namespace rdws::controllers {

/**
 * UserController - Handles JSON serialization and HTTP response formatting
 * Separates presentation logic from business logic
 */
class UserController : public rdws::controllers::BaseController {
  public:
    /**
     * Convert UsersResult to JSON response
     */
    static std::string formatUsersResponse(const rdws::types::UsersResult& result) {
        if (result.isSuccess()) {
            return rdws::utils::ResponseHelper::returnEntities(result.getData(), "users");
        } else {
            return rdws::utils::ResponseHelper::returnError(result.getErrorMessage(),
                                                            result.getStatusCode());
        }
    }

    /**
     * Convert UserResult to JSON response
     */
    static std::string formatUserResponse(const rdws::types::UserResult& result) {
        if (result.isSuccess()) {
            return rdws::utils::ResponseHelper::returnEntity(result.getData(), "user");
        } else {
            return rdws::utils::ResponseHelper::returnError(result.getErrorMessage(),
                                                            result.getStatusCode());
        }
    }

    /**
     * Convert CountResult to JSON response
     */
    static std::string formatCountResponse(const rdws::types::CountResult& result) {
        if (result.isSuccess()) {
            ::rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();

            ::rapidjson::Value countData(::rapidjson::kObjectType);
            countData.AddMember("count", ::rapidjson::Value(static_cast<int>(result.getData())),
                                allocator);

            return rdws::utils::ResponseHelper::returnData(countData);
        } else {
            return rdws::utils::ResponseHelper::returnError(result.getErrorMessage(),
                                                            result.getStatusCode());
        }
    }

    /**
     * Convert OperationResult to JSON response
     */
    static std::string formatOperationResponse(const rdws::types::OperationResult& result) {
        if (result.isSuccess()) {
            const auto& status = result.getData();
            return rdws::utils::ResponseHelper::returnSuccess(status.message);
        } else {
            return rdws::utils::ResponseHelper::returnError(result.getErrorMessage(),
                                                            result.getStatusCode());
        }
    }
};

} // namespace rdws::controllers
