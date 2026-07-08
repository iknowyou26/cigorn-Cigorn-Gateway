#include "DeviceRepository.h"

DeviceRepository::DeviceRepository(PostgresDatabase& db)
    : db_(db)
{
}

bool DeviceRepository::Load()
{
    return db_.Query(
    "SELECT * FROM wdevice;",
    result_);
}

DBResult& DeviceRepository::Data()
{
    return result_;
}
