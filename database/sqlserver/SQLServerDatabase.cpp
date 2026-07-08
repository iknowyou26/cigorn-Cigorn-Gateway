#include "SQLServerDatabase.h"

SQLServerDatabase::SQLServerDatabase() {}
SQLServerDatabase::~SQLServerDatabase() {}

bool SQLServerDatabase::Connect(const std::string& connInfo)
{
    lastError = "SQL Server backend not implemented yet";
    return false;
}

void SQLServerDatabase::Disconnect()
{
}

bool SQLServerDatabase::Execute(const std::string& sql)
{
    lastError = "SQL Server Execute not implemented yet";
    return false;
}

bool SQLServerDatabase::Query(const std::string& sql, DBResult& result)
{
    lastError = "SQL Server Query not implemented yet";
    return false;
}

std::string SQLServerDatabase::LastError() const
{
    return lastError;
}
