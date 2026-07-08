#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "DBResult.h"
#include "repositories/WirelessNatRepository.h"
#include <iostream>

int TestWirelessNatRepository()
{
    PostgresDatabase db;
    DBResult result;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "WirelessNatRepository DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    WirelessNatRepository repo(&db);

    if (!repo.LoadAll(result))
    {
        std::cout << "WirelessNatRepository LoadAll failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "WirelessNatRepository loaded "
              << result.RowCount()
              << " wnat rows."
              << std::endl;

    return result.RowCount();
}
