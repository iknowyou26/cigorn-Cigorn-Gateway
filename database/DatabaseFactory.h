#ifndef DATABASE_FACTORY_H
#define DATABASE_FACTORY_H

#include <string>
#include "IDatabase.h"

enum class DatabaseType
{
    PostgreSQL,
    SQLServer
};

class DatabaseFactory
{
public:
    static IDatabase* Create(DatabaseType type);
    static IDatabase* Create(const std::string& type);
};

#endif
