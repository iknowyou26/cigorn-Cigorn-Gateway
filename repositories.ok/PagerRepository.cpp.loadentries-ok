#include "PagerRepository.h"
#include <sstream>

PagerRepository::PagerRepository(IDatabase* database)
{
    db = database;
}

bool PagerRepository::LoadAll(DBResult& result)
{
    return db->Query("SELECT * FROM pagers;", result);
}

bool PagerRepository::AddPager(int pagerNumber)
{
    std::stringstream ss;
    ss << "INSERT INTO pagers (pagernumber) VALUES (" << pagerNumber << ");";
    return db->Execute(ss.str());
}

bool PagerRepository::DeletePager(int pagerNumber)
{
    std::stringstream ss;
    ss << "DELETE FROM pagers WHERE pagernumber=" << pagerNumber << ";";
    return db->Execute(ss.str());
}
bool PagerRepository::LoadEntries(std::vector<PagerTableEntry>& entries)
{
    DBResult result;

    entries.clear();

    if (!LoadAll(result))
        return false;

    for (int i = 0; i < result.RowCount(); i++)
    {
        PagerTableEntry entry;

        entry.pagerNumber = result.GetInt(i, 0);
        entry.pageDataType = result.GetString(i, 1);
        entry.capCode = result.GetInt(i, 2);
        entry.otaProtocol = result.GetString(i, 3);
        entry.isGroup = (result.GetString(i, 4) == "t" || result.GetString(i, 4) == "true" || result.GetString(i, 4) == "1");
        entry.isActive = (result.GetString(i, 5) == "t" || result.GetString(i, 5) == "true" || result.GetString(i, 5) == "1");

        entries.push_back(entry);
    }

    return true;
}
