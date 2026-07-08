#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/PagerRepository.h"
#include "adapters/PagerTableAdapter.h"
#include <iostream>

int TestPagerTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "PagerTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    PagerRepository repo(&db);
    PagerTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "PagerTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "PagerTableAdapter loaded "
              << adapter.RowCount()
              << " pager rows."
              << std::endl;

    return adapter.RowCount();
}
