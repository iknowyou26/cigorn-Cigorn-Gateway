#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "DBResult.h"
#include "repositories/TtyDeviceRepository.h"
#include <iostream>

int TestTtyDeviceRepository()
{
    PostgresDatabase db;
    DBResult result;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "TtyDeviceRepository DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    TtyDeviceRepository repo(&db);

    if (!repo.LoadAll(result))
    {
        std::cout << "TtyDeviceRepository LoadAll failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "TtyDeviceRepository loaded "
              << result.RowCount()
              << " tty device rows."
              << std::endl;

    return result.RowCount();
}
