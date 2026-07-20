/* 
 * File:   WirelessNAT.h
 * Author: john
 * Files related to translating IDs between TCP ports and IDs
 * Used to map Ethernet ports to Wireless Device IDs
 *
 * Created on May 8, 2011, 12:28 PM
 */

#ifndef WIRELESSNAT_H
#define	WIRELESSNAT_H
#include "platform/thread/PlatformMutex.h"
#include <queue>
#include <vector>
#include "BinaryEntry.h"

using namespace std;

// The maximum number of entries in the WNAT table
#define MaxWNATentries 1000
#define MaxHistory  20
// An entry in the Wireless NAT table

struct WNATEntry{
   string Designator;          // The device designator for this entry
   int PortCount;              // The number of ports in this WNAT block
   int lowerID;                // The Wireless Device ID for this base port.
   string DefaultDevDes;       // where to send it if the WNAT port is closed
   string Comment;             // general comment
};


class WirelessNAT {
public:
    WirelessNAT();
    WirelessNAT(const WirelessNAT& orig);
    virtual ~WirelessNAT();

    void AddWNAT(string, int, int, string, string);  // add an entry in the WNAT table
    int AddressTranslate(BinaryEntry&);
    int PortTranslate(BinaryEntry&, int&, int&);
    int GetID(string , int );
    int StoreHistory(string);
    bool GetEntry(int, WNATEntry&);
    void ClearAll(void);

    long ToLanCount;
    long FromLanCount;
    vector <WNATEntry> WNATentries;
    vector <WNATEntry>::iterator it;
    vector <string> history;
    vector <string>::iterator wit;
    cigorn::PlatformMutex wnatlock;
    cigorn::PlatformMutex historylock;

private:

};

#endif	/* WIRELESSNAT_H */

