#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "DBResult.h"
#include "repositories/RouteRepository.h"
#include <iostream>

int TestRouteRepository()
{
    PostgresDatabase db;
    DBResult result;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "RouteRepository DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    RouteRepository repo(&db);

    if (!repo.LoadAll(result))
    {
        std::cout << "RouteRepository LoadAll failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    std::cout << "RouteRepository loaded "
              << result.RowCount()
              << " route rows." << std::endl;

    return result.RowCount();
}
