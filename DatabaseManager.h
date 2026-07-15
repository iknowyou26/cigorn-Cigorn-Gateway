#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "IDatabase.h"

#include <string>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool SelectDatabase(
        const std::string& databaseType
    );

    bool Connect(
        const std::string& connInfo
    );

    IDatabase* Database();

private:
    IDatabase* db;
};

#endif