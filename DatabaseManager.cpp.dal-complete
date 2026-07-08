#include "DatabaseManager.h"
#include "PostgresDatabase.h"

DatabaseManager::DatabaseManager()
{
    db = new PostgresDatabase();
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
