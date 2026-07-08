#include "RouteRepository.h"

RouteRepository::RouteRepository(IDatabase* database)
{
    db = database;
}

bool RouteRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM routes;", result);
}
