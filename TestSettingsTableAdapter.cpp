#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/SettingsRepository.h"
#include "adapters/SettingsTableAdapter.h"
#include <iostream>

int TestSettingsTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "SettingsTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    SettingsRepository repo(&db);
    SettingsTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "SettingsTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "SettingsTableAdapter loaded "
              << adapter.RowCount()
              << " siteconfig rows."
              << std::endl;

    return adapter.RowCount();
}
