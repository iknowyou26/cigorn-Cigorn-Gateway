#ifndef DEVICEREPOSITORY_H
#define DEVICEREPOSITORY_H
#include "../IDatabase.h"
//#include "../PostgresDatabase.h"
#include "../DBResult.h"
#include "../TableDefs.h"
#include "DeviceRepository.h"
class DeviceRepository
{
public:
    DeviceRepository(IDatabase* database);
    //explicit DeviceRepository(PostgresDatabase& db);

    bool Load();

    DBResult& Data();

private:
    IDatabase* db;
    //PostgresDatabase& db_;
    DBResult result_;
};

#endif
