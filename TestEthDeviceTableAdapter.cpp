#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/EthDeviceRepository.h"
#include "adapters/EthDeviceTableAdapter.h"
#include <iostream>

int TestEthDeviceTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "EthDeviceTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    EthDeviceRepository repo(&db);
    EthDeviceTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "EthDeviceTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "EthDeviceTableAdapter loaded "
              << adapter.RowCount()
              << " ethernet rows."
              << std::endl;

    return adapter.RowCount();
}
