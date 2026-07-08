#include "DatabaseFactory.h"
#include "postgres/PostgresDatabase.h"
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
