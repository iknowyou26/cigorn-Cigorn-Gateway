/* 
 * File:   cypher.h
 * Author: john
 *
 * Created on December 18, 2010, 9:37 PM
 */

#ifndef CYPHER_H
#define	CYPHER_H
#include <string>
using namespace std;

class cypher {
public:
    cypher();
    cypher(const cypher& orig);
    virtual ~cypher();
    string encypher(string);
    string decypher(string);
    void StringToCbits(string& , char* , int);
    string CbitsToHex(char*, int);
    string HexToString(string);
    void HexToCbits(string,char*, int);
    int testcypher(void );
    void newkey(string);

    string cigornkey;

private:

};

#endif	/* CYPHER_H */

