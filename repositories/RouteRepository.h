#ifndef ROUTEREPOSITORY_H
#define ROUTEREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"

class RouteRepository : public IRepository
{
public:
    RouteRepository(IDatabase* database);

    bool LoadAll(DBResult& result);

private:
    IDatabase* db;
};

#endif
