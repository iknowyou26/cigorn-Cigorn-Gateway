#ifndef PAGERREPOSITORY_H
#define PAGERREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"
#include "../PagerTable.h"
#include <vector>
class PagerRepository : public IRepository
{
public:
    PagerRepository(IDatabase* database);
    bool LoadEntries(std::vector<PagerTableEntry>& entries);
    bool LoadAll(DBResult& result);
    bool AddPager(int pagerNumber);
    bool DeletePager(int pagerNumber);

private:
    IDatabase* db;
};

#endif
