#include "postgresql_database.h"
#include <stdexcept>


namespace rdws::database {

// PostgreSQLResultSet Implementation

PostgreSQLResultSet::PostgreSQLResultSet(pqxx::result res)
    : result(std::move(res)), currentRow(0) {}

bool PostgreSQLResultSet::next() {
    if (currentRow < (pqxx::result::size_type)result.size()) {
        ++currentRow;
        return currentRow <= (pqxx::result::size_type)result.size();
    }
    return false;
}

bool PostgreSQLResultSet::previous() {
    if (currentRow > 1) {
        --currentRow;
        return true;
    }
    return false;
}

void PostgreSQLResultSet::reset() {
    currentRow = 0;
}

std::string PostgreSQLResultSet::getString(const std::string& columnName) {
    if (currentRow == 0 || currentRow > (pqxx::result::size_type)result.size()) {
        throw std::runtime_error("Invalid row position");
    }
    return result[currentRow - 1][columnName].as<std::string>();
}

int PostgreSQLResultSet::getInt(const std::string& columnName) {
    if (currentRow == 0 || currentRow > (pqxx::result::size_type)result.size()) {
        throw std::runtime_error("Invalid row position");
    }
    return result[currentRow - 1][columnName].as<int>();
}

double PostgreSQLResultSet::getDouble(const std::string& columnName) {
    if (currentRow == 0 || currentRow > (pqxx::result::size_type)result.size()) {
        throw std::runtime_error("Invalid row position");
    }
    return result[currentRow - 1][columnName].as<double>();
}

bool PostgreSQLResultSet::getBool(const std::string& columnName) {
    if (currentRow == 0 || currentRow > (pqxx::result::size_type)result.size()) {
        throw std::runtime_error("Invalid row position");
    }
    return result[currentRow - 1][columnName].as<bool>();
}

bool PostgreSQLResultSet::isNull(const std::string& columnName) {
    if (currentRow == 0 || currentRow > (pqxx::result::size_type)result.size()) {
        throw std::runtime_error("Invalid row position");
    }
    return result[currentRow - 1][columnName].is_null();
}

size_t PostgreSQLResultSet::getColumnCount() {
    return result.columns();
}

std::vector<std::string> PostgreSQLResultSet::getColumnNames() {
    std::vector<std::string> names;
    for (auto i = 0; i < (pqxx::result::size_type)result.columns(); ++i) {
        names.emplace_back(result.column_name(i));
    }
    return names;
}

size_t PostgreSQLResultSet::getRowCount() {
    return result.size();
}

// PostgreSQLDatabase Implementation

PostgreSQLDatabase::PostgreSQLDatabase() {
    // Uses default Config constructor that loads from environment
    PostgreSQLDatabase::connect();
}

PostgreSQLDatabase::PostgreSQLDatabase(const rdws::Config& dbConfig)
    : config(dbConfig) {
    PostgreSQLDatabase::connect();
}

PostgreSQLDatabase::~PostgreSQLDatabase() {
    if (currentTransaction) {
        PostgreSQLDatabase::rollbackTransaction();
    }
    PostgreSQLDatabase::disconnect();
}

std::unique_ptr<IResultSet> PostgreSQLDatabase::execQuery(
    const std::string& query,
    const std::vector<std::string>& parameters
) {
    try {
        ensureConnection();

        if (currentTransaction) {
            pqxx::result result;
            if (parameters.empty()) {
                result = currentTransaction->exec(query);
            } else {
                // Use individual parameters for better compatibility
                if (parameters.size() == 1) {
                    result = currentTransaction->exec_params(query, parameters[0]);
                } else if (parameters.size() == 2) {
                    result = currentTransaction->exec_params(query, parameters[0], parameters[1]);
                } else if (parameters.size() == 3) {
                    result = currentTransaction->exec_params(query, parameters[0], parameters[1], parameters[2]);
                } else {
                    result = currentTransaction->exec_params(query, parameters);
                }
            }
            return std::make_unique<PostgreSQLResultSet>(std::move(result));
        } else {
            pqxx::work txn{*connection};
            pqxx::result result;
            if (parameters.empty()) {
                result = txn.exec(query);
            } else {
                // Use individual parameters for better compatibility
                if (parameters.size() == 1) {
                    result = txn.exec_params(query, parameters[0]);
                } else if (parameters.size() == 2) {
                    result = txn.exec_params(query, parameters[0], parameters[1]);
                } else if (parameters.size() == 3) {
                    result = txn.exec_params(query, parameters[0], parameters[1], parameters[2]);
                } else {
                    result = txn.exec_params(query, parameters);
                }
            }
            txn.commit();
            return std::make_unique<PostgreSQLResultSet>(std::move(result));
        }
    } catch (const std::exception& e) {
        lastError = e.what();
        throw std::runtime_error("Query execution failed: " + std::string(e.what()));
    }
}bool PostgreSQLDatabase::execCommand(
    const std::string& command,
    const std::vector<std::string>& parameters
) {
    try {
        ensureConnection();

        if (currentTransaction) {
            if (parameters.empty()) {
                currentTransaction->exec(command);
            } else {
                // Use individual parameters for better compatibility
                if (parameters.size() == 1) {
                    currentTransaction->exec_params(command, parameters[0]);
                } else if (parameters.size() == 2) {
                    currentTransaction->exec_params(command, parameters[0], parameters[1]);
                } else if (parameters.size() == 3) {
                    currentTransaction->exec_params(command, parameters[0], parameters[1], parameters[2]);
                } else {
                    currentTransaction->exec_params(command, parameters);
                }
            }
            return true;
        } else {
            pqxx::work txn{*connection};
            if (parameters.empty()) {
                txn.exec(command);
            } else {
                // Use individual parameters for better compatibility
                if (parameters.size() == 1) {
                    txn.exec_params(command, parameters[0]);
                } else if (parameters.size() == 2) {
                    txn.exec_params(command, parameters[0], parameters[1]);
                } else if (parameters.size() == 3) {
                    txn.exec_params(command, parameters[0], parameters[1], parameters[2]);
                } else {
                    txn.exec_params(command, parameters);
                }
            }
            txn.commit();
            return true;
        }
    } catch (const std::exception& e) {
        lastError = e.what();
        return false;
    }
}

bool PostgreSQLDatabase::execBatch(
    const std::vector<std::string>& commands,
    const std::vector<std::vector<std::string>>& parameterSets
) {
    if (commands.size() != parameterSets.size()) {
        lastError = "Commands and parameter sets size mismatch";
        return false;
    }

    try {
        ensureConnection();

        bool wasInTransaction = (currentTransaction != nullptr);
        if (!wasInTransaction) {
            beginTransaction();
        }

        for (size_t i = 0; i < commands.size(); ++i) {
            currentTransaction->exec_params(commands[i], parameterSets[i]);
        }

        if (!wasInTransaction) {
            commitTransaction();
        }

        return true;
    } catch (const std::exception& e) {
        lastError = e.what();
        if (currentTransaction) {
            rollbackTransaction();
        }
        return false;
    }
}

void PostgreSQLDatabase::beginTransaction() {
    ensureConnection();
    if (currentTransaction) {
        throw std::runtime_error("Transaction already in progress");
    }
    currentTransaction = std::make_unique<pqxx::work>(*connection);
}

void PostgreSQLDatabase::commitTransaction() {
    if (!currentTransaction) {
        throw std::runtime_error("No transaction in progress");
    }
    currentTransaction->commit();
    currentTransaction.reset();
}

void PostgreSQLDatabase::rollbackTransaction() {
    if (!currentTransaction) {
        throw std::runtime_error("No transaction in progress");
    }
    currentTransaction->abort();
    currentTransaction.reset();
}

bool PostgreSQLDatabase::isConnected() {
    return connection && connection->is_open();
}

void PostgreSQLDatabase::connect() {
    try {
        connection = std::make_unique<pqxx::connection>(config.getConnectionString());
        lastError.clear();
    } catch (const std::exception& e) {
        lastError = e.what();
        throw std::runtime_error("Failed to connect to database: " + std::string(e.what()));
    }
}

void PostgreSQLDatabase::disconnect() {
    if (currentTransaction) {
        rollbackTransaction();
    }
    if (connection) {
        connection->close();
        connection.reset();
    }
}

std::string PostgreSQLDatabase::getLastError() {
    return lastError;
}

void PostgreSQLDatabase::ensureConnection() {
    if (!isConnected()) {
        connect();
    }
}

} // namespace rdws::database

