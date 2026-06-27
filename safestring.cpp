/* 
 * File:   safestring.cpp
 * Author: john
 * 
 * Created on September 10, 2012, 7:42 PM
 */

#include "safestring.h"

safestring::safestring() {
    thestring = "";
    ready = false;
}

safestring::safestring(const safestring& orig) {
}

safestring::~safestring() {
}

void safestring::store(string s){
     thestring = s;
     ready = true;
}

void safestring::clear(void) {
    thestring = "";
    ready = false;
}

