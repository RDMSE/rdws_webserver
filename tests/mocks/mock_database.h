#pragma once

#include "../../src/shared/common/database/idatabase.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "../../src/shared/types/user.h"

#include <gmock/gmock.h>

namespace rdws {
namespace testing {

class MockDatabase : public rdws::database::IDatabase {
public:
    MOCK_METHOD(std::unique_ptr<rdws::database::IResultSet>, execQuery, (const std::string& query, const std::vector<std::string>& parameters), (override));
    MOCK_METHOD(bool, execCommand, (const std::string& command, const std::vector<std::string>& parameters), (override));
    MOCK_METHOD(bool, execBatch, (const std::vector<std::string>& commands, const std::vector<std::vector<std::string>>& parameterSets), (override));
    MOCK_METHOD(void, beginTransaction, (), (override));
    MOCK_METHOD(void, commitTransaction, (), (override));
    MOCK_METHOD(void, rollbackTransaction, (), (override));
    MOCK_METHOD(bool, isConnected, (), (override));
    MOCK_METHOD(void, connect, (), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(std::string, getLastError, (), (override));
    // Métodos auxiliares para compatibilidade com testes
    void reset() {}
    void setConnectionStatus(bool) {}
    size_t getUserCount() { return 3; }
    void clearUsers() {}
};

} // namespace testing
} // namespace rdws
