/* 
 * File:   WMX.cpp
 * Author: John Sonnenberg
 * 
 * Created on September 13, 2010, 6:36 PM
 *
 * C++ Class to parse a WMX frame into all of its components.
 * Version 1.0 of this class.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <iostream>
#include "functions.h"
#include "WMX.h"
#include "ascii.h"

// Constructor for the WMX frame parser
// Called when there is a buffer with a WMX frame in it (d) with bcount bytes.
WMX::WMX(char* d, int& c) {
    source = -1;               // not defined yet
    destination = -1;
    control = WMX_FR_UNDEF;    // undefined
    payloadsize = -1;          // none yet
    group = -1;
    seqnum = 0;
    rssi = 0;
    cksum = 0;
    thecks = 0;
    size = 0;                 // number total number of bytes in the WMX frame.
    pMsg = d;                 // point to the first byte to try to parse out.
    good_checksum = true;     // start by assuming OK.
    valid = parse(d, c);      // go parse this message
}

// Constructor for the WMX frame structure
// Called to create a WMX frame
WMX::WMX() {
    source = -1;               // not defined yet
    destination = -1;
    control = WMX_FR_UNDEF;    // undefined
    payloadsize = -1;          // none yet
    group = -1;
    seqnum = 0;
    rssi = 0;
    pMsg = NULL;               // point to the first byte in the WMX frame
    cksum = 0;
    thecks = 0;
    size = 0;                 // number of bytes in the message, after removed the encoding/stuffing.
    good_checksum = true;      // start by assuming OK.
 }

WMX::WMX(const WMX& orig) {
}

WMX::~WMX() {
}

// build up a WMX frame with WNC records in it. Return true if OK.
bool WMX::BuildWnc(WNCrecordList& rl){
    char *c;
    string s;
    c = frame;
    int i;
    size = 0;
    valid = false;
    payloadsize = 0;

    *c++ = WMX_SOH;   // First byte is always this.
    *c++ = control;

    // Now put in the destination ID for this packet. Most significan digit first.
    s = IntToHex(destination);      // convert the destination address to a hex string of 1-8 digits
    i = 0;
    while (i < s.size())
       *c++ = s[i++];               // copy the string to the c buffer

    *c++ = FS1;                     // here comes the Source ID
    s = IntToHex(source);           // convert the destination address to a hex string of 1-8 digits
    i = 0;
    while (i < s.size())
       *c++ = s[i++];               // copy the string to the c buffer

    *c++ = FS3;                     // here comes the Source ID
    seqnum++;
    if (seqnum > WMX_MAX_SEQUENCE)
        seqnum = 0;                 // Restart the sequence

    s = intToString(seqnum);           // convert the destination address to a hex string of 1-8 digits
    i = 0;
    while (i < s.size())
       *c++ = s[i++];               // copy the string to the c buffer


    *c++ = WMX_SOT;                 // here comes the text

    // Now load the payload bytes into the WMX message
    vector<WNCrecord>::iterator it;
    for (it = rl.begin(); it != rl.end(); it++){
        *c++ = WNC_SEPARATOR;
        payloadsize++;
        *c++ = it->type;          // The record type
        payloadsize++;
        i = 0;
        while (i < it->data.size())
           *c++ = it->data[i++];       // copy the record string to the wmc data buffer
           payloadsize++;
     }

    *c++ = WMX_DLE;                    // end of the text
    *c++ = WMX_ETX;                    // end of the text
    *c++ = WMX_EOT;                    // end of the text
   
    *c = NUL;         // always NUL terminat

  
    size = c - frame; // save the length of the WMX frame we made
    valid = true;
    return valid;

}

// build up a WMX frame. Return the number of bytes in the WMX frame if OK,
// -1 if fail
int WMX::BuildWMX(char* data, int bcout, int src, int dst, int frametype){

    string s;
    char* c = frame;               // the first byte of the frame we are building up
    unsigned char b;
    //c = data;
    int i;

    if (bcout > WMX_MAX_PAYLOAD)
        return -1;   // too much data

    // initialize the WMX structure
    size = 0;
    valid = false;
    destination = dst;
    source = src;
    control = frametype;

    *c++ = WMX_SOH;   // First byte is always this.
    *c++ = control;

    // The destination ID for this WMX packet. Most significan digit first.
    s = IntToHex(destination);      // convert the destination address to a hex string of 1-8 digits
    i = 0;
    while ((i < s.size()) && (i < WMX_MAX_IDBYTES)){
       *c++ = s[i++];               // copy the string to the c buffer
    }

    // The source ID for this WMX packet. Most significan digit first.
    *c++ = FS1;                     // here comes the Source ID
    s = IntToHex(source);           // convert the destination address to a hex string of 1-8 digits
    i = 0;
    while ((i < s.size())&& (i < WMX_MAX_IDBYTES)){
       *c++ = s[i++];               // copy the string to the c buffer
    }

    // The sequence number.
    if (seqnum > 0){
        *c++ = FS3;                     // here comes the Source ID
        seqnum++;
        if (seqnum > WMX_MAX_SEQUENCE)
            seqnum = 1;                 // Restart the sequence

        s = intToString(seqnum);           // convert the destination address to a hex string of 1-8 digits
        i = 0;
        while ((i < s.size()) && (i < WMX_MAX_SEQLEN)){
           *c++ = s[i++];               // copy the string to the c buffer
        }
    }

    *c++ = WMX_SOT;                 // here comes the text
 
    // Now load the payload bytes into the WMX message
    vector<WNCrecord>::iterator it;
    i = 0;  // count payload bytes
    while (bcout > 0){
        b = *data++;                     // get the byte we want to put into the message
        if (((unsigned char)b == WMX_BINARY_CODE) || ((unsigned char)b == WMX_ENCODE_0x03)
             || ((unsigned char)b == WMX_ENCODE_0x04)|| ((unsigned char)b == WMX_ENCODE_0x10)|| ((unsigned char)b == WMX_ENCODE_0x0D)){
            // This byte needs to be binary encoded
            *c++ = WMX_BINARY_CODE;                   // the binary encode flag
            *c++ = ~b;                                // The binary byte is the inverse of the actual
        }else{
            *c++ = b;    // no binary encoding. Just store the byte
        }
        bcout--;
     }

    *c++ = WMX_DLE;                    // end of the text
 
    *c++ = WMX_ETX;                    // end of the text
 
    *c++ = WMX_EOT;                    // end of the text
 
    *c = NUL;                          // always NUL terminate
 
    valid = true;
    size = c - frame;         // save the length of the WMX frame we made
    return size;

}

string WMX::Display(){
    stringstream ss;
    int i, y;

   ss << "WMX Packet:  Source=" << source << "  Destination=" << destination << endl;
   ss << "Sequence=" << seqnum << " Bytes of data = " << dsize << endl;
   ss << "Data:" ;
    
    for (i=0; i<dsize; i++){
        y = data[i];
        ss << IntToHex(y, 2) << " ";
    }
            
    return ss.str();
        
}


// return true if the WMX message parsed OK
bool WMX::parse(char* thedata, int& count){
    char* p;
    char* q;  // the pointer after p
    char* pEnd;
    char* pDLE;  // points to the DLE/EOT char in the message
    char ch;
    size = 0;

    int i;

    p = thedata;             // start pointing to the beginning of the message
    pEnd = p + count;        // pointer to the end of the WMX message

    if (*p != WMX_SOH)
        return false;    // must always start with SOH

    p++;
    control = *p;        // get the control byte

    if (p>=pEnd)
        return false;    // we hit the end of the message way to early. Error in protocol formatting.

    p++;                 // point to the first byte of the destination address (in HEX)
    destination = 0;
    i = 0;
    while (IsHex(*p)){
        destination = destination * 16 + HexVal(*p);    // Convert ASCII HEX to integer value
        i++; // count digits in the address field
        p++;
    }
    if (i>8)
        return false;  // can't have mooe than 8 digits in the address field
    
    if (*p == FS1){
        // Here comes the Source address
        p++;
        if (p>=pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
        source = 0;
        i = 0;
        while (IsHex(*p)){
            source = source * 16 + HexVal(*p);   // Convert ASCII HEX to integer value
            i++; // count digits in the address field
            p++;
        }
        if (i>8)
            return false;  // can't have mooe than 8 digits in the address field

    }
    if (*p == FS2){
        // Here comes the Group Number
        p++;
        if (p>=pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
        group = 0;
        i = 0;
        while (IsDecimal(*p)){
            group = group * 10 + DecVal(*p);   // Convert ASCII to integer value
            i++; // count digits in the field
            p++;
        }
        if (i>4)
            return false;  // can't have more than 4 digits in the group

    }
     if (*p == FS3){
        p++;
        if (p>=pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
        // Here comes the Sequence Number
        seqnum = 0;
        i = 0;
        while (IsDecimal(*p)){
            seqnum = seqnum * 10 + DecVal(*p);   // Convert ASCII to integer value
            i++; // count digits in the field
            p++;
        }
        if (i>4)
            return false;  // can't have more than 4 digits

    }
    if (*p == FS4){
        p++;
        if (p>=pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
        // Here comes the RSSI
        rssi = 0;
        i = 0;
        while (IsDecimal(*p)){
            rssi = rssi * 10 - DecVal(*p);   // Convert decimal to integer value (negative)
            i++; // count digits in the field
            p++;
        }
        if (i > 4)
            return false;  // can't have more than 3 digits in the rssi
    }

    // Now find the SOT byte.  Always look for it in case there are new protocol fields that we are not aware of
    i = 0;
    while ((*p != WMX_SOT) && (i < WMX_SOT_MAXPOS)){
        p++;
        i++;
        if (p >= pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
    }

    if (*p != WMX_SOT)
        return false;      // protcol error. fail to parse.

    // Start looking at the payload portion of the frame.
    p++;     // point to the first byte of the payload data
    if (p>=pEnd)
        return false;    // we hit the end of the message way to early. Error in protocol formatting.
    q = p;
    q++;          // q points to the next charactor after p
    dsize = 0;   // count the number of data bytes in the payload portion of the frame (including stuffed bytes)
    i = 0;
    data[0] = NUL;
    while ((*q != WMX_ETX ) && (i < WMX_MAX_PAYLOAD) && (p < pEnd)){
        ch = *p;
        if ((unsigned char)ch == WMX_BINARY_CODE){
            // We know there was an extra byte stuffed into the message
            //cout << "Bin" << IntToHex((int)ch, 2) << endl;
            p++;
            q++;  // go get it
            if (p >= pEnd)
                return false;    // we hit the end of the message way to early. Error in protocol formatting.
            data[i] = ~*p;       // the actual data byte was the inverse of this byte sent.
        }else{
           data[i] = *p;
        }
        i++;   // count this byte
        data[i] = NUL;   // null terminate just because
        p++;
        q++;
        if (p>=pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
    }
    dsize = i;

    // All data bytes are parsed.
    if ((*p != WMX_DLE) || (*q != WMX_ETX))
        return false;      // protcol error. fail to parse.

    pDLE = p;   // remember where the DLE is so we can later calculate checksum
    p++; p++;   // step over the DLE ETX sequence to the checksum
    cksum = 0;  // The checksum as reported in the WMX frame

    if (p>=pEnd)
        return false;    // we hit the end of the message way to early. Error in protocol formatting.
    i = 0;
    while (IsHex(*p)){
        cksum = cksum * 16 + HexVal(*p);   // Convert ASCII to integer value
        i++; // count digits in the field
        p++;
        if (p > pEnd)
            return false;    // we hit the end of the message way to early. Error in protocol formatting.
    }
    if (i>8)
        return false;  // can't have more than 8 digits

    if (*p != WMX_EOT)
        return false;      // protcol error. We should be at the end of the message now.  Fail to parse.
 
    if (cksum > 0){
        // There was a checksum sent, so go check it
        thecks = 0;
        for (p = pMsg; p < pDLE; p++){
           thecks = thecks + *p; // add up the checksum of all of the bytes.
        }

        if (thecks != cksum)
            good_checksum = false;
    }
    size = p - thedata;

    return true;

}

