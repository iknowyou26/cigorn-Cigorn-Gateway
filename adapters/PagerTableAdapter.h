#ifndef PAGERTABLEADAPTER_H
#define PAGERTABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/PagerRepository.h"
#include <string>

class PagerTableAdapter
{
public:
    PagerTableAdapter(PagerRepository* repo);

    bool Load();
    int RowCount() const;

    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;
    bool GetBool(int row, int col) const;

private:
    PagerRepository* repository;
    DBResult result;
};

#endif
