/* 
 * File:   SiteManagement.h
 * Author: john
 *
 * Created on September 30, 2010, 11:27 PM
 */


#include "ourstructures.h"
#include <string>
#include <queue>
#include <vector>
#include "WMX.h"
#include "Ticker.h"
#include "rfport.h"

using namespace std;


#ifndef _SITEMANAGEMENT_H
#define	_SITEMANAGEMENT_H

// The radio channel management default parameters

#define CM_SLOTTIME  0.1
#define CM_TXRATE    30.0
#define CM_MONCHAN   1
#define CM_GPSCHAN   2
#define CM_TDMATIME  30.0

typedef map<int, rfport> rfportlist;

// The WD states of communications
enum WDCOMSTATES
     {smReset,         // We just reset and don't know what the state of a WD is
      smNonExist,       // This WD does not exist on this site
      smRegRequest,     // The WD sent a registration request to us
      smRegDenied,      // The WD tried to register, but was denied
      smRegApproved,    // The WD is approved
      smDataTxRequest,  // The WD requested to transmit some data
      smDataTxGrant,    // We graned the WD some bandwidth to send data
      smDataSent,       // The WD sent some data to us
      smDataOut         // Gateway is sending a short message to the WD.
} ;

class SiteManagement {
public:
    SiteManagement();
    SiteManagement(const SiteManagement& orig);
    virtual ~SiteManagement();

    bool NewMessageIn(BinaryEntry&);
    bool AddRFport(int, int, int);
    bool ChannelExists(int);
    void ChannelManagement(void);        // do the Wireless channel management, TDMA allocation, trunking...
    bool NewTdmaParms(double, double);   // we changed some TDMA timing parameters
    string DeviceName(int);              // return the name of the device servicing this RF channel.
    string DeviceDesignator(int);        // return the name of the device designator for this interface.
    Ticker MyTicker;
    rfportlist RFports;                  // details about all of our RF ports

    // The radio channel management parameters
    double EpochTime;
    double slottime;
    double txrate;
    double monchan;
    double gpschan;
    int ControlChan;      // The channel this site is supposed to use for the
    int MaxControlChan;   // The maximum number of control channels in this system.

private:
    void ChannelManager(WMX&);
    void NextSlotIsZero(void);           // do things relaeted to slot zero.
    void SendSysInfo(void);

    int PreviousSlotProcessed;  // The number of the slot we already processed
    int LastSlotNum;            // the number of the last slot in the epoch
};

#endif	/* _SITEMANAGEMENT_H */

