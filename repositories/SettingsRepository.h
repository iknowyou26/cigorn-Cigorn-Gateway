#ifndef SETTINGSREPOSITORY_H
#define SETTINGSREPOSITORY_H

#include "IRepository.h"
#include "../IDatabase.h"
#include "../DBResult.h"

class SettingsRepository : public IRepository
{
public:
    SettingsRepository(IDatabase* database);

    bool LoadAll(DBResult& result);

private:
    IDatabase* db;
};

#endif
