#include "WirelessNatRepository.h"

WirelessNatRepository::WirelessNatRepository(IDatabase* database)
{
    db = database;
}

bool WirelessNatRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM wnat;", result);
}
