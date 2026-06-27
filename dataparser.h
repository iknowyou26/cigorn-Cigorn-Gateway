/* 
 * File:   dataparser.h
 * Author: john
 *
 * Created on August 17, 2010, 10:28 PM
 */

#ifndef _DATAPARSER_H
#define	_DATAPARSER_H

#include <map>
#include "dataprotocol.h"
#include "ourstructures.h"
#include "BinaryEntry.h"
#include "ascii.h"

#define MAX_PARSE_BUFF  10000

// A list that holds the definitions of our protocols
typedef std::map<int,dataprotocol> parselist;

enum TapState {
    WAIT_FOR_CONNECT,
    CONNECTED
}; 

class dataparser {
public:
    dataparser();
    dataparser(const dataparser& orig);
    virtual ~dataparser();
    int parse(char*, int, int, int);
    int RawRXcount();
    void ClearRXbuffer(void);
    int  rget;
    int  rput;
    int  ParsingPort;               // the Port number this parser is parsing
    bool NMEAparsing;               // set true when a parser sees a message that may be for it.
    bool WMXparsing;                // set true when a parser sees a message that may be for it.
    bool XMLparsing;                // set true when a parser sees a message that may be for it.
    int  DefaultSrcID;              // The default source ID to use when we don't have one in the protocol
    int  DefaultDstID;              // The default destination ID to use when we don't have one in the protocol
    char rawdata[MAX_PARSE_BUFF];   // holds the raw data as it comes in.
    parselist protocols;            // all of the protocols we need to look for.
    char EndOfLineChar;             // The char found when parsing for an enter key
    int   ExtractData(char *, int);
    bool look4command(string *);
    void  initialize(void);

private:


    bool  addnew(char*, int);
    void  cleanraw(void);
    void  cleanrawXML(void);
    void  realignRaw(void);
    int   look4NMEA();
    int   look4ASCII();
    int   look4WMX();
    int   look4XML();
    int   findchar(char);           // find a char in a NUL termianted string. Zero based.
    int   SetFormat(BinaryEntry&);
    int   sendASCII(int, int, char*, int);
    int   parseTAP();
    TapState tapState;
    
    int devindex;
    int devicetype;
};

#endif	/* _DATAPARSER_H */

