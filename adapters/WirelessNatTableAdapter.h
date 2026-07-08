#ifndef WIRELESSNATTABLEADAPTER_H
#define WIRELESSNATTABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/WirelessNatRepository.h"
#include <string>

class WirelessNatTableAdapter
{
public:
    explicit WirelessNatTableAdapter(WirelessNatRepository* repo);

    bool Load();

    int RowCount() const;

    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;
    bool GetBool(int row, int col) const;

private:
    WirelessNatRepository* repository;
    DBResult result;
};

#endif
