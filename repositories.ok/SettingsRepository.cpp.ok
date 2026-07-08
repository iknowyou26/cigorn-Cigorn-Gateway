#include "SettingsRepository.h"

SettingsRepository::SettingsRepository(IDatabase* database)
{
    db = database;
}

bool SettingsRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM siteconfig;", result);
}
