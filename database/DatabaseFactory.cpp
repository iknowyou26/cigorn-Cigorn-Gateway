#include "DatabaseFactory.h"

#ifdef _WIN32
#include "sqlserver/SQLServerDatabase.h"
#else
#include "../PostgresDatabase.h"
#endif

IDatabase* DatabaseFactory::Create(DatabaseType type)
{
#ifdef _WIN32

    switch(type)
    {
        case DatabaseType::SQLServer:
            return new SQLServerDatabase();

        default:
            return nullptr;
    }

#else

    switch(type)
    {
        case DatabaseType::PostgreSQL:
            return new PostgresDatabase();

        case DatabaseType::SQLServer:
            return new SQLServerDatabase();

        default:
            return nullptr;
    }

#endif
}

IDatabase* DatabaseFactory::Create(const std::string& type)
{
#ifdef _WIN32

    return Create(DatabaseType::SQLServer);

#else

    if (type == "SQLServer")
        return Create(DatabaseType::SQLServer);

    return Create(DatabaseType::PostgreSQL);

#endif
}