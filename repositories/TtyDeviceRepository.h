#ifndef TTYDEVICEREPOSITORY_H
#define TTYDEVICEREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"

class TtyDeviceRepository : public IRepository
{
public:
    TtyDeviceRepository(IDatabase* database);

    bool LoadAll(DBResult& result);

private:
    IDatabase* db;
};

#endif
