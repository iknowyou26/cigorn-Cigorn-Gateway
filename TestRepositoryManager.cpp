#include "Cigorn.h"
#include "PostgresDatabase.h"
#include "RepositoryManager.h"
#include <iostream>

int TestRepositoryManager()
{
    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        std::cout << "RepositoryManager DB connect failed: "
                  << db.LastError() << std::endl;
        return -1;
    }

    RepositoryManager repos(&db);

    DBResult routes;

    if (!repos.Routes().LoadAll(routes))
    {
        std::cout << "RouteRepository failed." << std::endl;
        return -1;
    }

    std::cout << "RepositoryManager loaded "
              << routes.RowCount()
              << " route rows."
              << std::endl;

    return routes.RowCount();
}
