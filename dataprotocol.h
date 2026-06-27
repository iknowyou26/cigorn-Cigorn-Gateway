/* 
 * File:   dataprotocol.h
 * Author: john
 *
 * Created on September 21, 2010, 10:47 PM
 */

#ifndef _DATAPROTOCOL_H
#define	_DATAPROTOCOL_H

#include <string>

#define MAX_PROTO_BUFF  5000
#define MAX_PROTOCOLS   16   // arbitrary

using namespace std;


class dataprotocol {
public:
    dataprotocol();
    dataprotocol(const dataprotocol& orig);
    virtual ~dataprotocol();

    int AddNewData(char *, int);   // add these bytes to the buffer
    int LookForMessages(void);     // remove an valid messages in the buffer
    int FindChar(char);            // find the first location of this char in our buffer
    bool ExtractMessage(void);
    void  CleanRaw(void);

    char SOH_ch;  // start of message
    char EOT_ch;  // end of message
    char delim;   // field delimnator to find the ID
    int  MaxLen;  // Maximum number of bytes in the message
    bool ascii;   // True if this is an ascii protocol, (no bytes >127)
    char rawdata[MAX_PROTO_BUFF];   // holds the raw data as it comes in.
    int  count;
    string header;  // the header bytes for this protocol. NUL if nothing
    string name;
    int timeout;    // end-of-message time out in mS.  -1 no time out.

private:

};

#endif	/* _DATAPROTOCOL_H */

