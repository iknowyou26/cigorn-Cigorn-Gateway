#include "EthDeviceTableAdapter.h"

EthDeviceTableAdapter::EthDeviceTableAdapter(EthDeviceRepository* repo)
{
    repository = repo;
}

bool EthDeviceTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int EthDeviceTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string EthDeviceTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int EthDeviceTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}
