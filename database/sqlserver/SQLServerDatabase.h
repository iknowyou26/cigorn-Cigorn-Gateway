#ifndef SQLSERVERDATABASE_H
#define SQLSERVERDATABASE_H

#include <string>
#include "../../IDatabase.h"
#include "../../DBResult.h"

class SQLServerDatabase : public IDatabase
{
public:
    SQLServerDatabase();
    virtual ~SQLServerDatabase();

    bool Connect(const std::string& connInfo) override;
    void Disconnect() override;

    bool Execute(const std::string& sql) override;
    bool Query(const std::string& sql, DBResult& result) override;

    std::string LastError() const override;

private:
    std::string lastError;
};

#endif
