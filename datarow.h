/* 
 * File:   datarow.h
 * Author: john
 *
 * Created on September 15, 2010, 9:01 PM
 */

#ifndef _DATAROW_H
#define	_DATAROW_H


#include <stdlib.h>   // Required by malloc()
#include <queue>
#include <queue>
#include <string>
#include <map>
#include "/usr/include/postgresql/libpq-fe.h"

enum rowstates {Unchanged, Added, Deleted, Modified };  // were not using the detached state


using namespace std;


class datatable; // Forward declaration


class datarow {
public:
    datarow();
    datarow(const datarow& orig);
    virtual ~datarow();


    map<int, string> col;         // our columns
    rowstates rowstate;

    string RowState(void);        // text version of the row state

protected:
    

private:

};

#endif	/* _DATAROW_H */

