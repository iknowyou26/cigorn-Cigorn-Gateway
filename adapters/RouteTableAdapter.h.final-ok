#ifndef ROUTETABLEADAPTER_H
#define ROUTETABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/RouteRepository.h"

class RouteTableAdapter
{
public:
    RouteTableAdapter(RouteRepository* repo);

    bool Load();
    int RowCount() const;
    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;

private:
    RouteRepository* repository;
    DBResult result;
};

#endif
