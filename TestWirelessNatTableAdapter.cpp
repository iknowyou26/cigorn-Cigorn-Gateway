#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/WirelessNatRepository.h"
#include "adapters/WirelessNatTableAdapter.h"
#include <iostream>

int TestWirelessNatTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "WirelessNatTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    WirelessNatRepository repo(&db);
    WirelessNatTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "WirelessNatTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "WirelessNatTableAdapter loaded "
              << adapter.RowCount()
              << " WNAT rows."
              << std::endl;

    return adapter.RowCount();
}
