#include "TtyDeviceRepository.h"

TtyDeviceRepository::TtyDeviceRepository(IDatabase* database)
{
    db = database;
}

bool TtyDeviceRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM ttydevdes;", result);
}
