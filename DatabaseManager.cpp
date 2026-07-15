#include "DatabaseManager.h"

#include "database/DatabaseFactory.h"

DatabaseManager::DatabaseManager()
    : db(
        DatabaseFactory::Create(
            DatabaseType::PostgreSQL
        )
      )
{
}

DatabaseManager::~DatabaseManager()
{
    if (db != nullptr)
    {
        db->Disconnect();
        delete db;
        db = nullptr;
    }
}

bool DatabaseManager::SelectDatabase(
    const std::string& databaseType
)
{
    IDatabase* selectedDatabase =
        DatabaseFactory::Create(databaseType);

    if (selectedDatabase == nullptr)
    {
        return false;
    }

    if (db != nullptr)
    {
        db->Disconnect();
        delete db;
    }

    db = selectedDatabase;
    return true;
}

bool DatabaseManager::Connect(
    const std::string& connInfo
)
{
    if (db == nullptr)
    {
        return false;
    }

    return db->Connect(connInfo);
}

IDatabase* DatabaseManager::Database()
{
    return db;
}