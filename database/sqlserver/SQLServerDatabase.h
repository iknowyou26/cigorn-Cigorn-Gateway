#ifndef SQLSERVERDATABASE_H
#define SQLSERVERDATABASE_H

#include "../IDatabase.h"

class SQLServerDatabase : public IDatabase
{
public:
    SQLServerDatabase();
    virtual ~SQLServerDatabase();

    bool Connect() override;
    bool Disconnect() override;

    DBResult Query(const std::string& sql) override;
    bool Execute(const std::string& sql) override;
};

#endif
