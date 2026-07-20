/* 
 * File:   datatable.h
 * Author: john
 *
 * Created on September 15, 2010, 9:28 PM
 */

#ifndef _DATATABLE_H
#define	_DATATABLE_H

#include <stdlib.h>   // Required by malloc()
#include <queue>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <libpq-fe.h>
#include "datarow.h"
#include "database.h"

typedef map<int, datarow>   t_datarow;      // datarow type

using namespace std;

    class datatable {
    public:
        datatable(string, string);
        datatable(const datatable& orig);
        virtual ~datatable();

        string GetItem(const int&, const int&);     // get a field by index
        string GetItem(const int&, string);         // get a field by name
        int GetIntItem(const int&, const int&);
        int GetIntItem(const int&, string);           // get an interger data value from row int column string
        bool GetBoolItem(const int&, string, bool);   // get an bool data value from row int column string
        int AddToItem(const int& , string, const int&);   // add an interger value to row int column string
        bool RowPresent(const int&);                  // is a row with given index present in the table?
        bool AddNewRow(const int&);
        bool DeleteRow(const int&);
        int CountRowState(rowstates);                 // Count how many rows have this state
        int GetColumnIndex(const string&);
        int StoreTime(const int& ,const string& , const string& );
        bool SetRW(const string& , const bool& );
        bool SetMSupdate(const string& , const bool& );
        bool ChangeField(const string&, const string&, const string&, const string&);
        bool StoreMSupdate(int , const string& , const string&);
        int BuildIndexList(vector<int>&, int, int);
        bool StoreUpdate(const int&, const string&, const string&);        // store an update to a field that came from a Chief site.
        bool StoreUpdate(const int&, const string&, const int&);           // store an update to a field that came from a Chief site.
        string LookupData(const string&, string);
        int  LookupIntData(const string& , string);   // get the number from an integer field in the table
        bool EntryExists(const string&, string);
        int  Import(datatable*);
        int  Clear(void);  // erase the table

        string tablename;
        bool AutoAddRows;
        string IndexCol;                 // The name of the column that is the index
        database* parentdb;              // pointer to the parent that this table belongs to

        t_datarow rows;           // rows of fields in our table
        map<int, datarow>::iterator dit; // an iterator object to step through the table

        map<int, string> colname;        // column names
        map<int, string> defaultval;     // the default value for this column
        map<int, bool> readonly;         // true if this column is read-only and we do not push updates back to the SQL server.
        map<int, bool> msupdate;         // true if this column is Ok to be updated by the Master Site.
        map<int, int> type;              // The type of information stored in this column

    private:


    };

#endif	/* _DATATABLE_H */


