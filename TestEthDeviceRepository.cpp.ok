#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "DBResult.h"
#include "repositories/EthDeviceRepository.h"
#include <iostream>

int TestEthDeviceRepository()
{
    PostgresDatabase db;
    DBResult result;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "EthDeviceRepository DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    EthDeviceRepository repo(&db);

    if (!repo.LoadAll(result))
    {
        std::cout << "EthDeviceRepository LoadAll failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "EthDeviceRepository loaded "
              << result.RowCount()
              << " ethernet device rows."
              << std::endl;

    return result.RowCount();
}
