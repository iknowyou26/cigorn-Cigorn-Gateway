#include "PagerTableAdapter.h"

PagerTableAdapter::PagerTableAdapter(PagerRepository* repo)
{
    repository = repo;
}

bool PagerTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int PagerTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string PagerTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int PagerTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}

bool PagerTableAdapter::GetBool(int row, int col) const
{
    return result.GetBool(row, col);
}
