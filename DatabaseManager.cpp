#include "DatabaseManager.h"
//#include "PostgresDatabase.h"
#include "database/DatabaseFactory.h"
DatabaseManager::DatabaseManager()
{
    db = DatabaseFactory::Create(DatabaseType::PostgreSQL);
}

DatabaseManager::~DatabaseManager()
{
    delete db;
}

bool DatabaseManager::Connect(const std::string& connInfo)
{
    return db->Connect(connInfo);
}

IDatabase* DatabaseManager::Database()
{
    return db;
}
