#ifndef ETHDEVICEREPOSITORY_H
#define ETHDEVICEREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"

class EthDeviceRepository : public IRepository
{
public:
    EthDeviceRepository(IDatabase* database);

    bool LoadAll(DBResult& result);

private:
    IDatabase* db;
};

#endif
