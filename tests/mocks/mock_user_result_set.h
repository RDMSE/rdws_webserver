#pragma once
#include "../../src/shared/common/database/idatabase.h"
#include <vector>
#include <map>
#include <string>

namespace rdws {
namespace testing {

class MockUserResultSet : public rdws::database::IResultSet {
public:
    MockUserResultSet(const std::vector<std::map<std::string, std::string>>& rows)
        : rows_(rows), currentRow_(-1) {}

    bool next() override {
        if (currentRow_ + 1 < static_cast<int>(rows_.size())) {
            ++currentRow_;
            return true;
        }
        return false;
    }
    bool previous() override {
        if (currentRow_ > 0) {
            --currentRow_;
            return true;
        }
        return false;
    }
    void reset() override { currentRow_ = -1; }

    std::string getString(const std::string& columnName) override {
        return rows_[currentRow_][columnName];
    }
    int getInt(const std::string& columnName) override {
        return std::stoi(rows_[currentRow_][columnName]);
    }
    double getDouble(const std::string& columnName) override {
        return std::stod(rows_[currentRow_][columnName]);
    }
    bool getBool(const std::string& columnName) override {
        return rows_[currentRow_][columnName] == "true";
    }
    bool isNull(const std::string& columnName) override {
        return rows_[currentRow_].find(columnName) == rows_[currentRow_].end();
    }
    size_t getColumnCount() override {
        return rows_.empty() ? 0 : rows_[0].size();
    }
    std::vector<std::string> getColumnNames() override {
        if (rows_.empty()) return {};
        std::vector<std::string> names;
        for (const auto& kv : rows_[0]) names.push_back(kv.first);
        return names;
    }
    size_t getRowCount() override { return rows_.size(); }

private:
    std::vector<std::map<std::string, std::string>> rows_;
    int currentRow_;
};

} // namespace testing
} // namespace rdws
