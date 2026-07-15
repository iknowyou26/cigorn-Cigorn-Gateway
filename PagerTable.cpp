/* 
/*
 * File:   PagerTable.cpp
 * Author: Ryan Quang Le
 *
 * Created on 07/15/2026
 */

#include "PagerTable.h"
#include "platform/thread/PlatformLockGuard.h"

PagerTable::PagerTable()
{
}

PagerTable::~PagerTable()
{
}

void PagerTable::AddPager(
    const PagerTableEntry& entry
)
{
    cigorn::PlatformLockGuard lock(tableLock);
    pagerTable.push_back(entry);
}

void PagerTable::ClearAll()
{
    cigorn::PlatformLockGuard lock(tableLock);
    pagerTable.clear();
}

PagerTableEntry PagerTable::GetPager(
    int pagerNumber
)
{
    cigorn::PlatformLockGuard lock(tableLock);

    for (const PagerTableEntry& entry : pagerTable)
    {
        if (entry.pagerNumber == pagerNumber)
        {
            return entry;
        }
    }

    PagerTableEntry badPager{};
    badPager.capCode = RESULT_NO_PAGER;

    return badPager;
}
