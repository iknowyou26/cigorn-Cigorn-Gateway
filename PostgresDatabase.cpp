#include "PostgresDatabase.h"
#include "DBResult.h"
#include <iostream>
#include <vector>
PostgresDatabase::PostgresDatabase()
{
    conn = NULL;
}

PostgresDatabase::~PostgresDatabase()
{
    Disconnect();
}

bool PostgresDatabase::Connect(const std::string& connInfo)
{
    conn = PQconnectdb(connInfo.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        lastError = PQerrorMessage(conn);
        return false;
    }

    return true;
}

void PostgresDatabase::Disconnect()
{
    if (conn != NULL)
    {
        PQfinish(conn);
        conn = NULL;
    }
}

bool PostgresDatabase::Execute(const std::string& sql)
{
    PGresult* res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        lastError = PQerrorMessage(conn);
        PQclear(res);
        return false;
    }
    
    PQclear(res);
    return true;
}

bool PostgresDatabase::Query(const std::string& sql, DBResult& result)
{
    result.Clear();

    PGresult* res = PQexec(conn, sql.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        lastError = PQerrorMessage(conn);
        PQclear(res);
        return false;
    }

    int nrows = PQntuples(res);
    int ncols = PQnfields(res);
    for (int c = 0; c < ncols; c++)
{
    result.AddColumn(PQfname(res, c), PQftype(res, c));
}
    for (int r = 0; r < nrows; r++)
    {
        std::vector<std::string> row;

        for (int c = 0; c < ncols; c++)
        {
            char* value = PQgetvalue(res, r, c);
            row.push_back(value ? value : "");
        }

        result.AddRow(row);
    }

    PQclear(res);
    return true;
}

std::string PostgresDatabase::LastError() const
{
    return lastError;
}
PGconn* PostgresDatabase::GetConnection() const
{
    return conn;
}
