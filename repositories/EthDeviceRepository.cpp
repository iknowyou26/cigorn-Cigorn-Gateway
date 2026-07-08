#include "EthDeviceRepository.h"

EthDeviceRepository::EthDeviceRepository(IDatabase* database)
{
    db = database;
}

bool EthDeviceRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM ethdevdes;", result);
}
