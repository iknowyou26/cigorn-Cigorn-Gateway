/* 
 * File:   dataupdate.cpp
 * Author: john
 * 
 * Created on October 21, 2010, 6:16 AM
 */

#include "datatable.h"
#include "functions.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "limits.h"

#include "dataupdate.h"

dataupdate::dataupdate(char* c) {
    tablename = ToString(c);
}
dataupdate::dataupdate(std::string s) {
    tablename = s;
}

dataupdate::dataupdate(const dataupdate& orig) {
}

dataupdate::~dataupdate() {
}

// Add an entry to the update list
void dataupdate::putupdate(int index, std::string, std::string){

}

// Add an entry to the update list
void dataupdate::putupdate(std::string s, std::string, std::string){

}


// Add an entry to the update list
void dataupdate::putupdate(char* ind, char* fld, std::string){

}


