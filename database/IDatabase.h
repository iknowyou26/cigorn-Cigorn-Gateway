#ifndef IDATABASE_H
#define IDATABASE_H

#include <string>

class DBResult;

class IDatabase
{
public:
    virtual bool Connect(const std::string& connInfo) = 0;
    virtual void Disconnect() = 0;

    virtual bool Execute(const std::string& sql) = 0;
    virtual bool Query(const std::string& sql, DBResult& result) = 0;

    virtual std::string LastError() const = 0;

    virtual ~IDatabase() {}
};

#endif
