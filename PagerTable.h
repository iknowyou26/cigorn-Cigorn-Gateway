/* 
 * File:   PagerTable.h
 * Author: Adam Hickerson
 *
 * Created on May 6, 2013, 7:55 AM
 */
#include <vector>
#include <string>

using namespace std;

#ifndef PAGERTABLE_H
#define	PAGERTABLE_H

struct PagerTableEntry{
    int pagerNumber;
    string pageDataType;
    int capCode;
    string otaProtocol;
    bool isGroup;
    bool isActive;
};

class PagerTable {
public:
    static const int RESULT_NO_PAGER = -1;
    
    PagerTable();
    virtual ~PagerTable();

    void AddPager(PagerTableEntry entry);
    void ClearAll();
    PagerTableEntry GetPager(int pagerNumber);
    
private:
    vector<PagerTableEntry> pagerTable;
    pthread_mutex_t tableLock;
};

#endif	/* PAGERTABLE_H */

