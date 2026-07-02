#ifndef DEVICEREPOSITORY_H
#define DEVICEREPOSITORY_H

#include "../PostgresDatabase.h"
#include "../DBResult.h"
#include "../TableDefs.h"
#include "DeviceRepository.h"
class DeviceRepository
{
public:

    explicit DeviceRepository(PostgresDatabase& db);

    bool Load();

    DBResult& Data();

private:

    PostgresDatabase& db_;
    DBResult result_;
};

#endif
