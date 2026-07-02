#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/TtyDeviceRepository.h"
#include "adapters/TtyDeviceTableAdapter.h"
#include <iostream>

int TestTtyDeviceTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "TtyDeviceTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    TtyDeviceRepository repo(&db);
    TtyDeviceTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "TtyDeviceTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "TtyDeviceTableAdapter loaded "
              << adapter.RowCount()
              << " tty device rows."
              << std::endl;

    return adapter.RowCount();
}
