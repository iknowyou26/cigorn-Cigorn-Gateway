/* 
 * File:   PagerTable.cpp
 * Author: Adam Hickerson
 * 
 * Created on May 6, 2013, 7:55 AM
 */

#include <ios>
#include "PagerTable.h"

PagerTable::PagerTable() {
}

void PagerTable::AddPager(PagerTableEntry entry){
    pthread_mutex_lock(&tableLock);
    pagerTable.push_back(entry);
    pthread_mutex_unlock(&tableLock);
}

void PagerTable::ClearAll(){
    pthread_mutex_lock(&tableLock);
    pagerTable.clear();
    pthread_mutex_unlock(&tableLock);
}

PagerTableEntry PagerTable::GetPager(int pagerNumber){
    pthread_mutex_lock(&tableLock);
    for(vector<PagerTableEntry>::iterator it = pagerTable.begin(); it != pagerTable.end(); it++){
        if(it->pagerNumber == pagerNumber){
            pthread_mutex_unlock(&tableLock);
            return (*it);
        }
    }
    pthread_mutex_unlock(&tableLock);
    
    // No pager found
    PagerTableEntry badPager;
    badPager.capCode = RESULT_NO_PAGER;
    return badPager;
}

PagerTable::~PagerTable() {
}

