#include "SQLServerDatabase.h"

SQLServerDatabase::SQLServerDatabase()
{
}

SQLServerDatabase::~SQLServerDatabase()
{
}

bool SQLServerDatabase::Connect()
{
    return false;     // TODO: ODBC connection
}

bool SQLServerDatabase::Disconnect()
{
    return true;
}

DBResult SQLServerDatabase::Query(const std::string& sql)
{
    DBResult result;
    return result;
}

bool SQLServerDatabase::Execute(const std::string& sql)
{
    return false;
}
