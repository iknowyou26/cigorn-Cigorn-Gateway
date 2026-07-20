/* 
 * File:   datarow.cpp
 * Author: john
 * 
 * Created on September 15, 2010, 9:01 PM
 */

#include "datarow.h"

#include <stdlib.h>   // Required by malloc()
#include <queue>
#include <queue>
#include <string>
#include <map>
#include <libpq-fe.h>
#include "functions.h"

using namespace std;

datarow::datarow() {
    // MyTable(mytable);

}

datarow::datarow(const datarow& orig) {
    
}

datarow::~datarow() {
}

//{Unchanged, Added, Deleted, Modified };
// Return a text version of the row state.
string  datarow::RowState(void){

    switch (rowstate){
        case Unchanged:
            return "Unchanged";
        case Added:
            return "Added";
        case Deleted:
            return "Deleted";
        case Modified :
            return "Modified ";
    }
    return "Unknown";
}


