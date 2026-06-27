/* 
 * File:   WMX.h
 * Author: john
 *
 * Created on September 13, 2010, 6:36 PM
 */

#ifndef _WMX_H
#define	_WMX_H
#include <vector>

using namespace std;

// **************************************************

// WMX declarations

// **************************************************

#define WMX_SOH   0x01      // Start of header
#define WMX_SOT   0x02      // Start of text
#define WMX_ETX   0x03      // End of text
#define WMX_EOT   0x04      // End of transmission
#define WMX_DLE   0x10      // DLE

// Frame types
#define WMX_FR_TXD      0x40      // Frame type 0 = transmit data
#define WMX_FR_RXD      0x42      // Frame type 2 = Received data
#define WMX_FR_COMMAND  0x44      // Frame type 4 = command processor
#define WMX_FR_RESPONSE 0x45      // Frame type 5 = response from command processor
#define WMX_FR_WNC      0x48      // Wireless Network Control
#define WMX_FR_SEND_CMD 0x54      //Send a command, ACK requested
#define WMX_FR_RAW      0x46      // Raw bits on the air
#define WMX_FR_UNDEF   -1

#define FS1 0x21 // Field separator 1 (!)
#define FS2 0x22 // Field separator 2 (")
#define FS3 0x23 // Field separator 3 (#)
#define FS4 0x24 // Field separator 4 ($)


// WMX control code positions.
#define WMX_SOT_MAXPOS 30 // the SOT is never any further than this into the frame. Make large for future protocol modifications.
// WMX binary encoding
#define WMX_BINARY_CODE 0xFF // byte used to flag binary encoding
#define WMX_ENCODE_0x03 0x03 // one of the byte we encode
#define WMX_ENCODE_0x04 0x04 // one of the byte we encode
#define WMX_ENCODE_0x0D 0x0D // one of the bytes we encode
#define WMX_ENCODE_0x10 0x10 // one of the bytes we encode

// Wireless Network Control record types
#define WNC_SEPARATOR   '|'        // Record Separator
#define WNC_EPOCH           'e'    // Epoch record
#define WNC_SLOTTIME        's'    // Slottime record
#define WNC_CONTROLCH       'd'    // Control Channel
#define WNC_SCS             'a'    // The first system control slot
#define WNC_SCSQTY          'b'    // The number of slots used each SCS interval
#define WNC_SCSINT          'c'    // The interval in slots between the start of System Control Slots
#define WNC_TXREQUEST       't'    // Requesting a transmission of sime type.
#define WNC_TXT_BEACON        '1'  // System Info Transmission. Usually sent in the CELL ID slot.
#define WNC_TXT_AUTH          '2'  // Device Authorization grant.
#define WNC_TXT_DENY          '3'  // Device Authorization denied.


// Protocol limits 
#define WXM_MAX_BYTES   5000   // Much bigger than the offical limit. Gives us some margin.
#define WMX_MAX_SEQUENCE 255    // The largest sequence number we will use.
#define WMX_LOCALDEST     0    // The destination ID for WMX to the local radio
#define WMX_LOCALSOURCE   0    // The source ID for WMX from the local gateway
#define WMX_MAX_PAYLOAD  1000  //
#define WMX_MAX_IDBYTES   9
#define WMX_MAX_SEQLEN    3

struct _wncrecord {
    char type;
    std::string data;
};

typedef _wncrecord WNCrecord;
typedef vector<WNCrecord>  WNCrecordList;

class WMX {
public:
    WMX();
    WMX(char*, int&);
    WMX(const WMX& orig);
    virtual ~WMX();
    bool BuildWnc(WNCrecordList&);     // build up a WMX frame. Return true if OK.
    int BuildWMX(char* , int , int , int , int);
    string Display();
    bool parse(char* , int& );

    bool valid;
    int source;
    int destination;
    int group;
    int seqnum;
    int rssi;
    int control;
    static const int MAXDATA = WXM_MAX_BYTES;         // the maximum numebr of bytes we can handle
    char* pPayload;                          // pointer to the payload's first byte
    char data[WXM_MAX_BYTES];                // data bytes represented by the payload (does not include stuffed bytes)
    int  dsize;                             // number of actual data bytes in the payload not including stuffed bytes
    char frame[WXM_MAX_BYTES];               // The complete WMX frame we built up
    int size;                                // The length of the complete WMX frame
    bool  good_checksum;

private:
    char* pMsg;          // pointer to the first byte in the WMX frame
    char* pData;         // point to the first byte of payload data
    int payloadsize;     // The bumber of bytes in the payload section of the WMX frame
    int   cksum;         // frame check sum as stored in the WMX message
    int   thecks;        // the checksum we calculated from the message
    char*  pFS1;
    char*  pFS2;
    char*  pFS3;
    char*  pFS4;
};

#endif	/* _WMX_H */

