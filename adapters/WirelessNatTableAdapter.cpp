#include "WirelessNatTableAdapter.h"

WirelessNatTableAdapter::WirelessNatTableAdapter(WirelessNatRepository* repo)
{
    repository = repo;
}

bool WirelessNatTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int WirelessNatTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string WirelessNatTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int WirelessNatTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}

bool WirelessNatTableAdapter::GetBool(int row, int col) const
{
    return result.GetBool(row, col);
}
