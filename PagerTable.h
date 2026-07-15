/*
 * File:   PagerTable.h
 * Author: Ryan Quang Le
 *
 * Created on 07/15/2026
 */

#ifndef PAGERTABLE_H
#define PAGERTABLE_H

#include "platform/thread/PlatformMutex.h"

#include <string>
#include <vector>

struct PagerTableEntry
{
    int pagerNumber;
    std::string pageDataType;
    int capCode;
    std::string otaProtocol;
    bool isGroup;
    bool isActive;
};

class PagerTable
{
public:
    static const int RESULT_NO_PAGER = -1;

    PagerTable();
    virtual ~PagerTable();

    void AddPager(const PagerTableEntry& entry);
    void ClearAll();
    PagerTableEntry GetPager(int pagerNumber);

private:
    std::vector<PagerTableEntry> pagerTable;
    cigorn::PlatformMutex tableLock;
};

#endif
