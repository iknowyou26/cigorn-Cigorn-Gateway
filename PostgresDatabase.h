#ifndef POSTGRESDATABASE_H
#define POSTGRESDATABASE_H

#include "IDatabase.h"
#include "/usr/include/postgresql/libpq-fe.h"
#include <string>

class PostgresDatabase : public IDatabase
{
public:
    PGconn* GetConnection() const;
public:
    PostgresDatabase();
    virtual ~PostgresDatabase();

    bool Connect(const std::string& connInfo);
    void Disconnect();

    bool Execute(const std::string& sql);
    bool Query(const std::string& sql, DBResult& result);

    std::string LastError() const;

private:
    PGconn* conn;
    std::string lastError;
};

#endif
