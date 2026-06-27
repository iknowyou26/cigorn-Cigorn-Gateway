/* 
 * File:   datatable.cpp
 * Author: john
 * 
 * Created on September 15, 2010, 9:28 PM
 */

#include "datatable.h"
#include "functions.h"

// Need for debug
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "limits.h"
#include <string>


datatable::datatable(string name, string s) {
    AutoAddRows = true;
    IndexCol = s;
    tablename = name;
}

datatable::datatable(const datatable& orig) {
}

datatable::~datatable() {
}

// Find a field in the table, given the row and column it is in.
string  datatable::GetItem(const int& r, const int& c){

    if (r < 0)
        return NULL;   // No IDs in the table are negative

    if (c >= colname.size())
        return NULL;
    return rows[r].col[c];
}

// String column name overload
string  datatable::GetItem(const int& r, string c){
    int i;

    if (r < 0)
        return NULL;   // do not add negative/NUL IDs to the table.

    for (i=0; i< colname.size(); i++){
        if (colname[i] == c){
            return GetItem(r,i);
        }
    }
    return "";
}



// Find a field in the table, given the row and column it is in.
int datatable::GetIntItem(const int& r, const int& c){

     if (r < 0)
        return 0;   // no negative rows

     if (c >= colname.size())
        return NULL;

     return StringToInt(rows[r].col[c]);

}

// String column name overload
int  datatable::GetIntItem(const int& r, string c){
    int i;
    if (r < 0)
        return 0;   // no negative row numbers

    for (i=0; i< colname.size(); i++){
        if (colname[i] == c){
            return GetIntItem(r,i);
        }
    }
    return NULL;
}


//
bool  datatable::GetBoolItem(const int& r, string c, bool def){
    int i;

    if (r < 0)
        return def;   // no negative rows


    for (i=0; i< colname.size(); i++){
        // find the column
        if (colname[i] == c){
            return IsStringTrue(GetItem(r,i));
        }
    }
    return def;
}

// add an integer value n to row r column c
int datatable::AddToItem(const int& r, string c, const int& n){
    int i;

    if (r < 0)
        return 0;   // do not add negative/NUL IDs to the table.

    if (!RowPresent(r)){
        // Item is not in our database
        if (AutoAddRows)
            AddNewRow(r);
        else
            return 0;

    }
    for (i=0; i< colname.size(); i++){
        if (colname[i] == c){
            if (rows[r].rowstate == Deleted)
                return 0;  // fail. No need to update deleted rows
            
            int newval = StringToInt(rows[r].col[i]) + n;
            rows[r].col[i] = intToString(newval);
            // {Unchanged, Added, Deleted, Modified, Detached }
            if (rows[r].rowstate == Unchanged)
                rows[r].rowstate = Modified;  // change the row state to modified.
            return newval;
        }
    }

}

// Store an update to a field. It probably came from another Chief Site.
// If the field is read-only, then it will fail false.
 bool datatable::StoreMSupdate(int r, const string& fieldname, const string&  t){
    int i;

    if (r < 0)
        return false;   // do not add negative/NUL IDs to the table.

    // cout << "Time:" << r << " " << c << " " << t << endl;
    i = GetColumnIndex(fieldname);
    
    if ( i<0 )
        return false;  // invalid column

    if (msupdate[i] == NULL)
        return false;

    if (msupdate[i] == false)
        return false;

    //cout << "Here!" << r << " " << fieldname << " " << t << endl;

    // It seems like this update is OK to save to our local database.  Update the table with the values.
    if (rows[r].rowstate == Deleted)
        return 0;  // fail. No need to update deleted rows
    // cout << "Time:" << r << " " << c << " " << t << endl;

    // OK. Do the  modification
    rows[r].col[i] = t;

    // {Unchanged, Added, Deleted, Modified, Detached }
    if (rows[r].rowstate == Unchanged)
       rows[r].rowstate = Modified;  // change the row state to modified.

    return true;

 }



// Store an update to a field.
// If the field is read-only, then it will fail false.
// idxfiled is the field we search for the index value.  The new value will be stored in field fieldname.
 bool datatable::ChangeField(const string& idxfld, const string&  idxval, const string& fieldname, const string&  newval){
    int i,r, p;


   i = GetColumnIndex(idxfld);      // The column in the table that is the index
   p = GetColumnIndex(fieldname);   // The column in the table that holds the data we changing

   cout << i << " " << p << endl;

   if ((i<0 ) || (p<0))
     return false;  // invalid columns

   if (readonly[p] == true)
       return false;

    // It seems like this update is OK to save to our local database.  Update the table with the values.
    if (rows[r].rowstate == Deleted)
        return 0;  // fail. No need to update deleted rows

   //cout << "Here!" << r << " " << fieldname << " " << endl;

   // Loop through the table using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       // cout << i <<  dit->second.col[i] << " " << dit->second.StringItem(c) << endl;
       if (StringToUpper(dit->second.col[i]) == StringToUpper(idxval)){
           // We found an entry for this index value
           if (dit->second.rowstate != Deleted){
               dit->second.col[p] =  newval;           // get the data in the specified column
               if (dit->second.rowstate == Unchanged)
                    dit->second.rowstate = Modified;
               return true;
           }
       }
   }
   return false;

 }



// Set a timestamp field   fld_tmLastMsg
// add an integer value n to row r column c
int datatable::StoreTime(const int& r, const string& c, const string& t){
    int i;

    if (r < 0)
        return 0;   // do not add negative/NUL IDs to the table.

    // cout << "Time:" << r << " " << c << " " << t << endl;
    i = GetColumnIndex(c);
    if (i >= 0){
        if (rows[r].rowstate == Deleted)
            return 0;  // fail. No need to update deleted rows
        // cout << "Time:" << r << " " << c << " " << t << endl;

        rows[r].col[i] = t;
        // {Unchanged, Added, Deleted, Modified, Detached }
        if (rows[r].rowstate == Unchanged)
            rows[r].rowstate = Modified;  // change the row state to modified.
        return 1;
    }
    return 0;

}
// Set a timestamp field   fld_tmLastMsg

bool datatable::StoreUpdate(const int& keyval, const string& col, const int& value){

    StoreUpdate(keyval, col, intToString(value));
}


// store an update to a field in one record of the table.
// key value must be int type.
bool datatable::StoreUpdate(const int& keyval, const string& col, const string& value){
    int i;

    if (keyval < 0)
        return false;   // key value must be type int for this

    dit = rows.find(keyval);
    if (dit == rows.end())
        return false;

    i = GetColumnIndex(col);


    if (i >= 0){
        if (rows[keyval].rowstate == Deleted)
            return false;  // fail. No need to update deleted rows
        // cout << "Time:" << r << " " << c << " " << t << endl;

        rows[keyval].col[i] = value;
        // {Unchanged, Added, Deleted, Modified, Detached }
        if (rows[keyval].rowstate == Unchanged)
            rows[keyval].rowstate = Modified;  // change the row state to modified.
        return true;
    }
    return false;

}
// Set a timestamp field   fld_tmLastMsg



// Count how many rows have this rs state
int  datatable::CountRowState(rowstates rs){
   int i=0;

   // Loop through the table using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       if (dit->second.rowstate == rs)
           i++;
   }
   return i;
}


// Add a new row with index r.  Index colmn is c
bool datatable::AddNewRow(const int& r){
    int i;
    bool found = false;

    if (r < 0)
        return false;   // do not add negative/NUL IDs to the table.

    for (i=0; i< colname.size(); i++){
        if (colname[i] == IndexCol){
            rows[r].col[i] = intToString(r);  // store the index
            rows[r].rowstate = Added;         // flag it is a new row.
            found = true;
        }else{
            rows[r].col[i] = defaultval[i];   // assign the default value for this data type.
        }

    }
    return found;

}

// Insert all index values into a list that fall between betwen two values
// return the number inserted.  This does not clear out v before loading it.
int datatable::BuildIndexList(vector<int>& v, int X, int Y){
    int i=0;

    // Loop through the table of our Wireless Devices using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       if ((dit->first >= X) && (dit->first <= Y)){
           v.insert(v.end(),dit->first);  // insert the ID into the list
           i++;
       }
    }

   return i;

}


// Delete a new row with index r.  Index colmn is c
bool datatable::DeleteRow(const int& r){
  
    if (r < 0)
        return false;   // no negative rows

    if (RowPresent(r)){
        rows.erase(r);
        return true;
    }
    else{
        return false;
    }

}

//Is an item in the database?
bool datatable::RowPresent(const int& r){
    int i;

    if (r < 0)
        return false;   // no negative rows

    map<int, datarow>::iterator it; // an iterator object to step through the table

    it = rows.find(r);

    if (it == rows.end())
        // Item is not in our database
        return false;
    else
        return true;

}

int datatable::GetColumnIndex(const string& s){
    int i;

    for (i=0; i< colname.size(); i++){
        if (colname[i] == s){
            return i;
        }
    }
    return -1;
}

// Set/clear the readonly flag for this column
bool datatable::SetRW(const string& s, const bool& b){
    int i;

    for (i=0; i< colname.size(); i++){
        if (colname[i] == s){
            readonly[i] = b;
            return true;
        }
    }
    return false;

}

// Set the msupdate flag to indicate if it is OK to get updates for this column from the Chief site
bool datatable::SetMSupdate(const string& s, const bool& b){
    int i;

    for (i=0; i< colname.size(); i++){
        if (colname[i] == s){
            msupdate[i] = b;
            return true;
        }
    }
    return false;

}

// Get the data from column c from row with the index value v
string  datatable::LookupData(const string& v, string c){

   int i;
   int p;

   i = GetColumnIndex(IndexCol);      // The column in the table that is the index for looking up stuff
   p = GetColumnIndex(c);             // The column in the table that holds the data we are looking for

   if ((v.size() <= 0) || (p < 0) || (i < 0))
        return "";   // no index parameter

   // Loop through the table using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       // cout << i <<  dit->second.col[i] << " " << dit->second.StringItem(c) << endl;
       if (StringToUpper(dit->second.col[i]) == StringToUpper(v)){
           // We found an entry for this index value
           return dit->second.col[p];           // get the data in the specified column
       }
   }
   return "";

}

// Get the data from column c from row with the index value v
int  datatable::LookupIntData(const string& v, string c){

   int i;
   int p;

   i = GetColumnIndex(IndexCol);      // The column in the table that is the index
   p = GetColumnIndex(c);             // The column in the table that holds the data we are looking for

   if ((v.size() <= 0) || (p < 0) || (i < 0))
        return -1;   // no index parameter

   // Loop through the table using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       // cout << i <<  dit->second.col[i] << " " << dit->second.StringItem(c) << endl;
       if (StringToUpper(dit->second.col[i]) == StringToUpper(v)){
           // We found an entry for this index value
           return StringToInt(dit->second.col[p]);           // get the data in the specified column
       }
   }
   return -1;

}


// Get the data from column c from row with the index value v
// Assumes the table has the same column information as the imported table
// Not working yet...
int  datatable::Import(datatable* Newtab){

   int i;
   int x;
   int idxCol = 0;
   t_datarow dr;
   int indexval;

   i = 0;      //

   rows.clear();  // erase our rows

   idxCol = GetColumnIndex(IndexCol);
   if (idxCol <0)
      return 0;


   //cout << "Copy table.." <<  Newtab->tablename << " to " << tablename << endl;
   // Loop through the imported table using the map iterator (dit)
   for (Newtab->dit = Newtab->rows.begin(); Newtab->dit != Newtab->rows.end(); Newtab->dit++){
       //cout << i <<  " col " <<  endl;
       //Newtab->dit->second;
       if (Newtab->dit->second.col.size() > 0){  // make sure there are columns

           
           // Make a unique index to put this row into our dt map structure
           if (Newtab->type[idxCol] == TEXTOID)
                indexval = i;  // for text indexes, use the row number as the index into our table
//          else
//                indexval =  Newtab->dit->second.col[idxCol]   // atoi(PQgetvalue(res, i,ix_ID));    // get the ID for this row (index val)
        rows[i] = Newtab->dit->second;
       }
       i++;
   }
   return i;

}

int  datatable::Clear(void){  // erase the table contents

        rows.clear();           // rows of fields in our table

}


// See if column c from row with the index value v exists
bool  datatable::EntryExists(const string& v, string c){

   int i;
   int p;

   i = GetColumnIndex(IndexCol);      // The column in the table that is the index
   p = GetColumnIndex(c);             // The column in the table that holds the data we are looking for

   if ((v.size() <= 0) || (p < 0) || (i < 0))
        return false;   // no index parameter

   // Loop through the table using the map iterator (dit)
   for (dit = rows.begin(); dit != rows.end(); dit++){
       // cout << i <<  dit->second.col[i] << " " << dit->second.StringItem(c) << endl;
       if (StringToUpper(dit->second.col[i]) == StringToUpper(v)){
           // We found an entry for this index value
           if (dit->second.col[p].size() > 0)           // get the data in the specified column
               return true;
           else
               return false;
       }
   }
  return false;
}
