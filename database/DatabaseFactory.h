#ifndef DATABASE_FACTORY_H
#define DATABASE_FACTORY_H

//#include "IDatabase.h"
#include "../IDatabase.h"
enum class DatabaseType
{
    PostgreSQL,
    SQLServer
};

class DatabaseFactory
{
public:
    static IDatabase* Create(DatabaseType type);
};

#endif
