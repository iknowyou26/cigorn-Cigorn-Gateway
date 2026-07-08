#ifndef SETTINGSTABLEADAPTER_H
#define SETTINGSTABLEADAPTER_H

#include "../DBResult.h"
#include "../repositories/SettingsRepository.h"
#include <string>

class SettingsTableAdapter
{
public:
    explicit SettingsTableAdapter(SettingsRepository* repo);

    bool Load();
    int RowCount() const;

    std::string GetString(int row, int col) const;
    int GetInt(int row, int col) const;
    bool GetBool(int row, int col) const;

private:
    SettingsRepository* repository;
    DBResult result;
};

#endif
