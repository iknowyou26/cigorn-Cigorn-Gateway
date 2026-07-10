#include "SQLServerDatabase.h"

SQLServerDatabase::SQLServerDatabase()
    : env(SQL_NULL_HENV),
      dbc(SQL_NULL_HDBC)
{
}

SQLServerDatabase::~SQLServerDatabase()
{
    Disconnect();
}

bool SQLServerDatabase::Connect(const std::string& connInfo)
{
    Disconnect();
    lastError.clear();

    SQLRETURN ret = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &env
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to allocate ODBC environment handle";
        return false;
    }

    ret = SQLSetEnvAttr(
        env,
        SQL_ATTR_ODBC_VERSION,
        reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),
        0
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to configure ODBC version";
        Disconnect();
        return false;
    }

    ret = SQLAllocHandle(
        SQL_HANDLE_DBC,
        env,
        &dbc
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to allocate ODBC connection handle";
        Disconnect();
        return false;
    }

    ret = SQLDriverConnect(
        dbc,
        nullptr,
        reinterpret_cast<SQLCHAR*>(
            const_cast<char*>(connInfo.c_str())
        ),
        SQL_NTS,
        nullptr,
        0,
        nullptr,
        SQL_DRIVER_NOPROMPT
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "SQL Server ODBC connection failed";
        Disconnect();
        return false;
    }

    return true;
}

void SQLServerDatabase::Disconnect()
{
    if (dbc != SQL_NULL_HDBC)
    {
        SQLDisconnect(dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
        dbc = SQL_NULL_HDBC;
    }

    if (env != SQL_NULL_HENV)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, env);
        env = SQL_NULL_HENV;
    }
}

bool SQLServerDatabase::Execute(const std::string& sql)
{
    lastError = "SQL Server Execute not implemented yet";
    return false;
}

bool SQLServerDatabase::Query(
    const std::string& sql,
    DBResult& result
)
{
    lastError = "SQL Server Query not implemented yet";
    return false;
}

std::string SQLServerDatabase::LastError() const
{
    return lastError;
}
