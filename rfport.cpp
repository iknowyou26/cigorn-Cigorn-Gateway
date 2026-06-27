/* 
 * File:   rfport.cpp
 * Author: john
 * 
 * Created on August 26, 2010, 10:55 PM
 * One rfport structure is created for each RF port (radio channel) on
 * the gateway running this code. Created when the
 * The actual RF channel number is the index in the RFports map.
 */
// Communication queue structures

#define iUNKNOWN_IO        0   // unknow interface type
#define iSOCK              1   // Socket interface
#define iCOM               2   // COM/TTY/RS232 interface
#define iCLI               3   // Command Line Interface
#define iHOST              4   // By this Host computer

#define dNONE              0   // No device
#define dM7                1   // RV-M7, without GPS
#define dM7GX              2   // RV-M7-GX

#define MAX_NMEA        1024
#define NMEA_START      '$'

#include <iostream>

#include "rfport.h"
#include "WMX.h"
#include "DeviceList.h"
#include "ourstructures.h"
#include "Router.h"
//#include "SiteManagement.h"
#include "ascii.h"
#include "functions.h"
#include "CommThread.h"
#include "functions.h"

// Prototype the Local routines
int BuildBeacon(WNCrecordList&);

rfport::rfport() {
    connected = false;        // true once we are connected
    msgIn = 0;                // the number of messages in
    msgOut = 0;               // the number of messages we sent out
    devtype = dNONE ;         // The type of device connected to this RF port. (dXXXX defined above)
    DevDesIndx = -1;
    wncout = 0;
    wncin = 0;
}

rfport::rfport(int p) {
    connected = false;        // true once we are connected
    msgIn = 0;                // the number of messages in
    msgOut = 0;               // the number of messages we sent out
    devtype = dM7GX ;         // The type of device connected to this RF port. (dXXXX defined above)
    DevDesIndx = -1;
    wncout = 0;
    wncin = 0;
}

rfport::rfport(const rfport& orig) {
}

rfport::~rfport() {
}

void rfport::SendBeacon(void){
    BinaryEntry be;   // create a structure to hold the SystemInfo message
    WMX WmxMsg;

    //cout << "Send Sys Info " << devtype << endl;
    WNCrecordList  WNCrecords;          // a list of records in/to WNC messages
    if (BuildBeacon(WNCrecords) <= 0)   // Build up the list of records in the Wireless Network Control message
        return;   // nothing to send out in the beacon.

    switch (devtype){
        case dDataModem:
            // Raveon M7 data radio modem. The WNC System Info message must be in the WMX format for the M7 to understand it.
            // We successfully constructed the WNC message with the info for a SysInfo transmission
            WmxMsg.source = WMX_LOCALSOURCE;
            WmxMsg.destination = WMX_LOCALDEST;
            WmxMsg.control =  WMX_FR_WNC;           // Wireless Network Control message
            WmxMsg.seqnum = 0;                      // No need to put a sequence on this type of message
            WmxMsg.BuildWnc(WNCrecords);            // Build up the WMX message with the WNC data in it.
            // We now have the WMX message. Store it into the outboud Queue
            be.format = fmtWMX;                 // this is a WMX packet
            be.dstID = WMX_LOCALDEST;           // This destination is the local radio
            be.srcID = WMX_LOCALSOURCE;         //
            be.SrcDevDesIndex = DevDesIndx;     // Save the DevDes this message will be passed on to.
            be.DstDevDesIndex = DevDesIndx;     // Save the DevDes this message will be passed on to.
            be.bcount = WmxMsg.size;            // The number of bytes in the message
            be.PortIn = -1;                     // Originated internally
            // Now put the WMX message into the be structure so it can e loaded into the outbound queue.
            if ((WmxMsg.size) < be.MAXDATA)
                CopyCstr(be.data, WmxMsg.frame, WmxMsg.size);  // put it into the structure
            else
                return;  // error. can't do this for some reason.

            break;
         case dWMXmodem:
            // Raveon M7 data radio modem. The WNC System Info message must be in the WMX format for the M7 to understand it.
            // We successfully constructed the WNC message with the info for a SysInfo transmission
            WmxMsg.source = WMX_LOCALSOURCE;
            WmxMsg.destination = WMX_LOCALDEST;
            WmxMsg.control =  WMX_FR_WNC;           // Wireless Network Control message
            WmxMsg.seqnum = 0;                      // No need to put a sequence on this type of message
            WmxMsg.BuildWnc(WNCrecords);            // Build up the WMX message with the WNC data in it.
            // We now have the WMX message. Store it into the outboud Queue
            be.format = fmtWMX;                 // this is a WMX packet
            be.dstID = WMX_LOCALDEST;           // This destination is the local radio
            be.srcID = WMX_LOCALSOURCE;         //
            be.SrcDevDesIndex = DevDesIndx;     // Save the DevDes this message will be passed on to.
            be.DstDevDesIndex = DevDesIndx;     // Save the DevDes this message will be passed on to.
            be.bcount = WmxMsg.size;           // The number of bytes in the message
            be.PortIn = -1;                     // Originated internally
            // Now put the WMX message into the be structure so it can e loaded into the outbound queue.
            if ((WmxMsg.size) < be.MAXDATA)
                CopyCstr(be.data, WmxMsg.frame, WmxMsg.size);  // put it into the structure
            else
                return;  // error. can't do this for some reason.

            break;

    }

    // Put the WNC Site Info message into the outpbound queue
    if (be.bcount > 0){
        wncout++; // count the messsage out
        DataRouter.RouteBinTo(be);   // route the be using the router.
    }

}

// Build up a system info message to be sent out to the base station
// Format it in WMX format
// Data is in Decimal ASCII chars, Most significant digit first. 
// Note decimal numbers may be longer or shorter.  Use Record Separator to parse the records, and
// WNC record type designators to determine the type of record being passed.
// General message format is:
// Bytes 1   Message type
// Bytes 3   Epoch time (TDMATIME in M7) in seconds
// Bytes  1 ---- Field Separator charactor ----
// Bytes 4   Slot Time in mS (width of the TDMA slots in this network)
// Bytes  1 ---- Field Separator charactor ----
// Bytes 2   Control Channel number
// Bytes  1 ---- Field Separator charactor ----
// Bytes 2  First Site Control Slot (SCS) slot number
// Bytes  1 ---- Field Separator charactor ----
// Bytes 1  Site Control Slot (SCS) quantity
// Bytes 2  Site Control Slot (SCS) modulous (number of slots between SCS)


int BuildBeacon(WNCrecordList& WNCrecords){
    int p = 0;
    double d = 0;
    string s = "";
    
    WNCrecord wncr;                // a single record

    // The TDMA epoch in seconds. (3 bytes of ASCII decimal digits)
    p = SiteManager.EpochTime;     // Convert double to integer
    s = intToString(p, 3);         // get 4 digit value of the slottime in mS
    wncr.type = WNC_EPOCH;
    wncr.data = s;
    WNCrecords.push_back(wncr);

    // The TDMA slot time width, in mS.  (4 bytes of ASCII decimal digits)
    d = SiteManager.slottime * 1000;     // Convert double to integer
    p = d;
    s = intToString(p,4);             // get three digit hex value of the slottime
    wncr.type = WNC_SLOTTIME;
    wncr.data = s;
    WNCrecords.push_back(wncr);

    // The control channel number.   (2 bytes of ASCII decimal digits)
    s = intToString(SiteManager.ControlChan, 2);                     // get three digit hex value of the slottime
    wncr.type = WNC_CONTROLCH;
    wncr.data = s;
    WNCrecords.push_back(wncr);


    // Send a Transmission request to kick of the transmission of the System Info message.
    wncr.type = WNC_TXREQUEST;
    wncr.data = WNC_TXT_BEACON;
    WNCrecords.push_back(wncr);

   return WNCrecords.size();
}