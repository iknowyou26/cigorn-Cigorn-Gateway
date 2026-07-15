/* 
 * File:   database.h
 * Author: john
 *
 * Created on September 14, 2010, 1:24 AM
 */

#ifndef _DATABASE_H
#define	_DATABASE_H

#include <stdlib.h>   // Required by malloc()
#include <queue>
#include <queue>
#include <string>
#include <map>
#include "DatabaseManager.h"
#include "datarow.h"

using namespace std;

#define MAXQUERYLEN   2000    //The maximum length of a query string
#define MAXROWCOUNT    1000000   // For postgre lets arbitrarily limit it here.
/*
Added	The DataRow object has been added to the DataRowCollection object, but DataRow::AcceptChanges has not been called.
Deleted	The DataRow object—belonging to a DataRowCollection—has been deleted using the DataRow::Delete method.
Detached	Either the DataRow object has not been added to the collection or it has been removed via either the DataRowCollection::Remove or DataRowCollection::RemoveAt method.
Modified	The DataRow object—belonging to a DataRowCollection—has been edited, but DataRow::AcceptChanges has not been called.
Unchanged	The DataRow object—belonging to a DataRowCollection—has not changed since the last time DataRow::AcceptChanges was called.
*/



#define TIMEDATENULL   "1/1/1900 12:00:00"

typedef std::map<int, int> IntMap;
typedef std::map<int,int>::iterator IntMapIterator;


// table  devstats
struct devstats{
    int    ID;
    long   countTo;
    long   countToD;
    long   countToM;
    long   countFm;
    long   countFmD;
    long   countFmM;
    rowstates rowstate;
};


class database {
public:
    database();
    database(const database& orig);
    virtual ~database();
    bool connect(string, string, string, string);
    bool close(void);
    IDatabase* GetDAL();
    int ExecuteQuery(string);
    int LoadTable(datatable* dt);
    int PushUpdatesToDB(datatable* dt);
    int PushChangesToDB(datatable* dt);
    int StoreTableToDB(datatable* dt);

    string FormatForSQL(string , string);
    string FormatForSQL(string ,int);
    int GetTypes(void);
    string TypeDesription(int );
    int GetIndexList(datatable* , IntMap&);
    string OurDefaults(int);
    bool IsValidType(int );
    bool ValidateType(int, string);

    map<int, string> SQLtypes;           // a list of all types supported by the DB
    bool ConnectionOK;
    bool AutoAddRows;
    
    // Some database statistics
    int NumOfUpdates;  // the number of times we do an upate with the SQL server
    string lastUpdate;
    int rowsadded;
    int rowsdeleted;
    string LastConnInfo;
    DatabaseManager dbManager;  // our connection to the dbase

private:

};


// The postgre type codes
#define BOOLOID                 16
#define BYTEAOID                17
#define CHAROID                 18
#define NAMEOID                 19
#define INT8OID                 20
#define INT2OID                 21
#define INT2VECTOROID           22
#define INT4OID                 23
#define REGPROCOID              24
#define TEXTOID                 25
#define OIDOID                  26
#define TIDOID                  27
#define XIDOID                  28
#define CIDOID                  29
#define OIDVECTOROID            30
#define POINTOID                600
#define LSEGOID                 601
#define PATHOID                 602
#define BOXOID                  603
#define POLYGONOID              604
#define LINEOID                 628
#define FLOAT4OID               700
#define FLOAT8OID               701
#define ABSTIMEOID              702
#define RELTIMEOID              703
#define TINTERVALOID            704
#define UNKNOWNOID              705
#define CIRCLEOID               718
#define CASHOID                 790
#define MACADDROID              829
#define INETOID                 869
#define CIDROID                 650
#define BPCHAROID               1042
#define VARCHAROID              1043
#define DATEOID                 1082
#define TIMEOID                 1083
#define TIMESTAMPOID            1114
#define TIMESTAMPTZOID          1184
#define INTERVALOID             1186
#define TIMETZOID               1266
#define BITOID                  1560
#define VARBITOID               1562
#define NUMERICOID              1700
#define REFCURSOROID            1790

#define ColTypeUnknown          "UNKNOWN"

#endif	/* _DATABASE_H */

