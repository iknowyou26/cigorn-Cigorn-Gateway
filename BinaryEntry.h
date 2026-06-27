/* 
 * File:   BinaryEntry.h
 * Author: john
 *
 * Created on December 25, 2010, 6:35 AM
 */

#ifndef BINARYENTRY_H
#define	BINARYENTRY_H

#define MAX_DATA        9999

class BinaryEntry {
public:
    BinaryEntry();
    BinaryEntry(const BinaryEntry& orig);
    virtual ~BinaryEntry();

    // The structure of an entry in the q for binary data.
    int SrcDevDesIndex;     // The device designator index for the interface this message came in on.
    int DstDevDesIndex;     // The destination device designator index.
    int PortIn;             // The port number this message came in on.
    char data[MAX_DATA];    // the data in the message
    int bcount;             // the number of bytes in this message
    int format;             // the format the parser thinks this message is in
    int srcID;              // the ID of the source for this message
    int dstID;              // the ID where this message is destined to be sent to
    double timein;	    // Time when this was created.  */
    static const int MAXDATA = MAX_DATA;         // the maximum numebr of bytes we can handle

private:

};

#endif	/* BINARYENTRY_H */

