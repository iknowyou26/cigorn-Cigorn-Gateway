#ifndef TTYDEVICETABLEADAPTER_H
#define TTYDEVICETABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/TtyDeviceRepository.h"
#include <string>

class TtyDeviceTableAdapter
{
public:
    TtyDeviceTableAdapter(TtyDeviceRepository* repo);

    bool Load();
    int RowCount() const;
    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;

private:
    TtyDeviceRepository* repository;
    DBResult result;
};

#endif
