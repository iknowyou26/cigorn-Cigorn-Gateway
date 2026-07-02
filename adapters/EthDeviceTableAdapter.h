#ifndef ETHDEVICETABLEADAPTER_H
#define ETHDEVICETABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/EthDeviceRepository.h"
#include <string>

class EthDeviceTableAdapter
{
public:
    EthDeviceTableAdapter(EthDeviceRepository* repo);

    bool Load();
    int RowCount() const;
    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;

private:
    EthDeviceRepository* repository;
    DBResult result;
};

#endif
