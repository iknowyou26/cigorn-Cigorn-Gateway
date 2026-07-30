#include "TtyDeviceRepository.h"

TtyDeviceRepository::TtyDeviceRepository(IDatabase* database)
{
    db = database;
}

bool TtyDeviceRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT designator, device, interface, channel, settings, baudrate, comments FROM ttydevdes;", result);
}

