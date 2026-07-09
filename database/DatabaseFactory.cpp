#include "DatabaseFactory.h"
#include "../PostgresDatabase.h"
#include "sqlserver/SQLServerDatabase.h"

IDatabase* DatabaseFactory::Create(DatabaseType type)
{
    switch(type)
    {
        case DatabaseType::PostgreSQL:
            return new PostgresDatabase();

        case DatabaseType::SQLServer:
            return new SQLServerDatabase();

        default:
            return nullptr;
    }
}
IDatabase* DatabaseFactory::Create(const std::string& type)
{
    if (type == "SQLServer")
        return Create(DatabaseType::SQLServer);

    return Create(DatabaseType::PostgreSQL);
}
