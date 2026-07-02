#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "DBResult.h"
#include "repositories/SettingsRepository.h"
#include <iostream>

int TestSettingsRepository()
{
    PostgresDatabase db;
    DBResult result;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "SettingsRepository DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    SettingsRepository repo(&db);

    if (!repo.LoadAll(result))
    {
        std::cout << "SettingsRepository LoadAll failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "SettingsRepository loaded "
              << result.RowCount()
              << " siteconfig rows."
              << std::endl;

    return result.RowCount();
}
