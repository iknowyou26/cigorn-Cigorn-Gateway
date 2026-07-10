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
    lastError.clear();

    if (dbc == SQL_NULL_HDBC)
    {
        lastError = "SQL Server connection is not open";
        return false;
    }

    SQLHSTMT stmt = SQL_NULL_HSTMT;

    SQLRETURN ret = SQLAllocHandle(
        SQL_HANDLE_STMT,
        dbc,
        &stmt
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to allocate ODBC statement handle";
        return false;
    }

    ret = SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(
            const_cast<char*>(sql.c_str())
        ),
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(ret))
    {
        SQLCHAR state[7] = {0};
        SQLCHAR message[512] = {0};
        SQLINTEGER nativeError = 0;
        SQLSMALLINT messageLength = 0;

        SQLGetDiagRec(
            SQL_HANDLE_STMT,
            stmt,
            1,
            state,
            &nativeError,
            message,
            sizeof(message),
            &messageLength
        );

        lastError =
            "SQL Server Execute failed. SQLSTATE=" +
            std::string(reinterpret_cast<char*>(state)) +
            " NativeError=" +
            std::to_string(nativeError) +
            " Message=" +
            std::string(reinterpret_cast<char*>(message));

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return true;
}

bool SQLServerDatabase::Query(
    const std::string& sql,
    DBResult& result
)
{
    lastError.clear();
    result.Clear();

    if (dbc == SQL_NULL_HDBC)
    {
        lastError = "SQL Server connection is not open";
        return false;
    }

    SQLHSTMT stmt = SQL_NULL_HSTMT;

    SQLRETURN ret = SQLAllocHandle(
        SQL_HANDLE_STMT,
        dbc,
        &stmt
    );

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to allocate ODBC statement handle";
        return false;
    }

    ret = SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(
            const_cast<char*>(sql.c_str())
        ),
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(ret))
    {
        SQLCHAR state[7] = {0};
        SQLCHAR message[512] = {0};
        SQLINTEGER nativeError = 0;
        SQLSMALLINT messageLength = 0;

        SQLGetDiagRec(
            SQL_HANDLE_STMT,
            stmt,
            1,
            state,
            &nativeError,
            message,
            sizeof(message),
            &messageLength
        );

        lastError =
            "SQL Server Query failed. SQLSTATE=" +
            std::string(reinterpret_cast<char*>(state)) +
            " NativeError=" +
            std::to_string(nativeError) +
            " Message=" +
            std::string(reinterpret_cast<char*>(message));

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return false;
    }

    SQLSMALLINT columnCount = 0;

    ret = SQLNumResultCols(stmt, &columnCount);

    if (!SQL_SUCCEEDED(ret))
    {
        lastError = "Failed to get SQL Server result column count";
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return false;
    }

    for (SQLUSMALLINT column = 1;
         column <= static_cast<SQLUSMALLINT>(columnCount);
         ++column)
    {
        SQLCHAR columnName[256] = {0};
        SQLSMALLINT nameLength = 0;
        SQLSMALLINT dataType = 0;
        SQLULEN columnSize = 0;
        SQLSMALLINT decimalDigits = 0;
        SQLSMALLINT nullable = 0;

        ret = SQLDescribeCol(
            stmt,
            column,
            columnName,
            sizeof(columnName),
            &nameLength,
            &dataType,
            &columnSize,
            &decimalDigits,
            &nullable
        );

        if (!SQL_SUCCEEDED(ret))
        {
            lastError = "Failed to describe SQL Server result column";
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            result.Clear();
            return false;
        }

        result.AddColumn(
            std::string(
                reinterpret_cast<char*>(columnName),
                nameLength
            ),
            static_cast<int>(dataType)
        );
    }

    while ((ret = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (!SQL_SUCCEEDED(ret))
        {
            lastError = "Failed while fetching SQL Server result row";
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            result.Clear();
            return false;
        }

        std::vector<std::string> row;

        for (SQLUSMALLINT column = 1;
             column <= static_cast<SQLUSMALLINT>(columnCount);
             ++column)
        {
            std::string value;
            SQLLEN indicator = 0;
            char buffer[1024];

            do
            {
                buffer[0] = '\0';

                ret = SQLGetData(
                    stmt,
                    column,
                    SQL_C_CHAR,
                    buffer,
                    sizeof(buffer),
                    &indicator
                );

                if (indicator == SQL_NULL_DATA)
                {
                    value.clear();
                    break;
                }

                if (!SQL_SUCCEEDED(ret) &&
                    ret != SQL_SUCCESS_WITH_INFO)
                {
                    lastError =
                        "Failed to read SQL Server result column";

                    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                    result.Clear();
                    return false;
                }

                value += buffer;

            } while (ret == SQL_SUCCESS_WITH_INFO);

            row.push_back(value);
        }

        result.AddRow(row);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return true;
}
std::string SQLServerDatabase::LastError() const
{
    return lastError;
}
