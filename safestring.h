/* 
 * File:   safestring.h
 * Author: john
 *
 * Created on September 10, 2012, 7:42 PM
 */

#ifndef SAFESTRING_H
#define	SAFESTRING_H

#include <string>

using namespace std;

class safestring {
public:
    safestring();
    safestring(const safestring& orig);
    virtual ~safestring();

    void clear(void);
    void store(string);

    string thestring;
    bool ready;

private:

};

#endif	/* SAFESTRING_H */

