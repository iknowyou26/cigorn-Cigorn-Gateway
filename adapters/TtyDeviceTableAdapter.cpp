#include "TtyDeviceTableAdapter.h"

TtyDeviceTableAdapter::TtyDeviceTableAdapter(TtyDeviceRepository* repo)
{
    repository = repo;
}

bool TtyDeviceTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int TtyDeviceTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string TtyDeviceTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int TtyDeviceTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}
