/* 
 * File:   SiteManagement.cpp
 * Author: john
 * 
 * Created on September 30, 2010, 11:27 PM
 */
#include "Router.h"
#include <iostream>
#include <string>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "platform/Platform.h"
#include <ios>
#include <limits>
#include <queue>
#include <vector>

#include "Cigorn.h"              // Our application-specific constants
#include "SiteManagement.h"
#include "WMX.h"
#include "rfport.h"
#include "CommThread.h"

using namespace std;


SiteManagement::SiteManagement() {
    // The radio channel control paramters
    EpochTime = CM_TDMATIME;
    slottime = CM_SLOTTIME;
    txrate = CM_TXRATE;
    monchan = CM_MONCHAN;
    gpschan = CM_GPSCHAN;
    ControlChan = -1;
    MaxControlChan = 1;
    LastSlotNum = EpochTime/slottime - 1;
    PreviousSlotProcessed = -1;             // have not processed any yet
}

SiteManagement::SiteManagement(const SiteManagement& orig) {
}

SiteManagement::~SiteManagement() {
}

// ***************************************************************************
// Wireless Channel Management logic
//  called each pass of the CommThread, typically a couple times/millisecond.
// ***************************************************************************
void SiteManagement::ChannelManagement(void){
    
    int CurrentSlotNum = MyTicker.SlotNum(EpochTime, slottime);    // get the slot number we are currently in
    int NextSlotNum;

    if (PreviousSlotProcessed == CurrentSlotNum)
        return;  // we already did everything we could for this slot

    // Remember we did all of the logic for this slot.
    PreviousSlotProcessed = CurrentSlotNum;


    // The folowing code is executed once at the beginning of each time slot.
    NextSlotNum = CurrentSlotNum + 1;
    if (NextSlotNum >= LastSlotNum)
        NextSlotNum = 0;

    if (NextSlotNum == 0){
        // The next slot is slot 0.
        NextSlotIsZero();
    }

    if (ControlChan < 0)
        return;  // this is not a managed system. 

    // Based upon the next slot, what messages need to be sent out?? 
    if (NextSlotNum < MaxControlChan){
        // These slots are only to be used for control information to the WDs.
        if ((NextSlotNum == (ControlChan - 1)) && (ControlChan > 0) && ChannelExists(ControlChan)){
            // The next slot is the one this site is supposed to transmit a beacon message out
            RFports[ControlChan].SendBeacon();      // send the system info message out this RFport.
        }
    }



}

string SiteManagement::DeviceName(int i){
    string s = "";

    if (ChannelExists(i)){
       s = OurDevices.DeviceName(RFports[i].devtype);
    }
    return s;
}

string SiteManagement::DeviceDesignator(int i){
    string s = "";

    if (ChannelExists(i)){
       s = OurDevices.designator[RFports[i].DevDesIndx];
    }
    return s;
}

// Time to send out a SYSINFO message out of our base station control channel.
void SiteManagement::SendSysInfo(void){ 
    
    if (ChannelExists(ControlChan)){
        // The control channel is configured on this gateway.  Go send the SysInfo out it.

        
    }
    
}

void SiteManagement::NextSlotIsZero(void){

}

// A message came in from a WD. See if it is something we should process
// Return TRUE if we handled this message, and it should not be routed any furthur.
//
bool SiteManagement::NewMessageIn(BinaryEntry& be) {
    int rssi = 0;

    //cout << "Msgr:" << be.srcID <<  " " << be.format << endl;
    if (be.format == fmtWMX){
        // This is a WMX packet.  Go see if we need to do something about it.
        WMX wmx(be.data, be.bcount);     // create a WMX message structure and parse the data
        if (wmx.valid){
            // We were able to parse this WMX message.  What type of WMX message is this?
            switch (wmx.control){
                case WMX_FR_TXD:
                    break;
                case WMX_FR_RXD:
                    // Got a data message in from some wireless device.
                    rssi = dtWD->StoreUpdate(be.srcID, fld_rssi, wmx.rssi );  // update rssi info the the WD table
                    break;
                case WMX_FR_COMMAND:
                    break;
                case WMX_FR_RESPONSE:
                    break;
                case WMX_FR_SEND_CMD:
                    break;
                case WMX_FR_WNC:
                    // Radio site management
                    ChannelManager(wmx);
                    return true;
                    break;
                case WMX_FR_UNDEF:
                    break;
            }
        }
    }

    return false;   // we dont' understand it, so ignore it and return.

}

// We received a site management message in from a WD.
// Go see what to do about it. Send an answer back if needed.
void SiteManagement::ChannelManager(WMX& w){


}

// Set new TDMA timing parameters for our site
bool SiteManagement::NewTdmaParms(double ep, double st){
    if ((ep >= 1) && (st <= ep)){
        EpochTime= ep;
        slottime = st;
        LastSlotNum = EpochTime/slottime - 1;
        return true;
    }
    return false;
}

//Return true of this channel exists
bool SiteManagement::ChannelExists(int i){

    rfportlist::iterator it;
    for (it = RFports.begin(); it != RFports.end(); it++){
        if (it->first == i)
            return true;  // we found this channel.
    }

    return false; // not found.

}


// Add an RF port to the array of rfports we manage.
bool SiteManagement::AddRFport(int ch, int devindex, int devtype){

  RFports[ch].devtype = devtype;
  RFports[ch].connected = false;  // unknown status
  RFports[ch].msgIn = 0;
  RFports[ch].msgOut = 0;
  RFports[ch].DevDesIndx = devindex;

  return true;

}

