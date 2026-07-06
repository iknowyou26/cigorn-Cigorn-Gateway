#include "SettingsTableAdapter.h"

SettingsTableAdapter::SettingsTableAdapter(SettingsRepository* repo)
{
    repository = repo;
}

bool SettingsTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int SettingsTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string SettingsTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int SettingsTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}

bool SettingsTableAdapter::GetBool(int row, int col) const
{
    return result.GetBool(row, col);
}
