#include "DeviceRepository.h"

DeviceRepository::DeviceRepository(IDatabase* database)
{
    db = database;
}

bool DeviceRepository::Load()
{
    return db->Query("SELECT * FROM wdevice;", result_);
}

DBResult& DeviceRepository::Data()
{
    return result_;
}
