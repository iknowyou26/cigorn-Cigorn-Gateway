#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "repositories/RouteRepository.h"
#include "adapters/RouteTableAdapter.h"
#include <iostream>

int TestRouteTableAdapter()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "RouteTableAdapter DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    RouteRepository repo(&db);
    RouteTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        std::cout << "RouteTableAdapter load failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "RouteTableAdapter loaded "
              << adapter.RowCount()
              << " route rows."
              << std::endl;

    return adapter.RowCount();
}
