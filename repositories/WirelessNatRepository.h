#ifndef WIRELESSNATREPOSITORY_H
#define WIRELESSNATREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"

class WirelessNatRepository : public IRepository
{
public:
    WirelessNatRepository(IDatabase* database);

    bool LoadAll(DBResult& result);

private:
    IDatabase* db;
};

#endif
