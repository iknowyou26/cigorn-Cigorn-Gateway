#include "RouteTableAdapter.h"

RouteTableAdapter::RouteTableAdapter(RouteRepository* repo)
{
    repository = repo;
}

bool RouteTableAdapter::Load()
{
    return repository->LoadAll(result);
}

int RouteTableAdapter::RowCount() const
{
    return result.RowCount();
}

std::string RouteTableAdapter::GetString(int row, int col) const
{
    return result.GetString(row, col);
}

int RouteTableAdapter::GetInt(int row, int col) const
{
    return result.GetInt(row, col);
}
