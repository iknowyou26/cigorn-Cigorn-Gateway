/* 
 * File:   WirelessNAT.cpp
 * Author: john
 * 
 * Created on May 8, 2011, 12:28 PM
 */

#include "DeviceList.h"
#include "CommThread.h"
#include <string>
#include "WirelessNAT.h"

WirelessNAT::WirelessNAT() {
    ToLanCount = 0;
    FromLanCount = 0;
}

WirelessNAT::WirelessNAT(const WirelessNAT& orig) {
}

WirelessNAT::~WirelessNAT() {
}

// Translate the protocol if needed.
// Return the new address/ID if this message was translated. -1 if no translation
int WirelessNAT::AddressTranslate(BinaryEntry& be){
    string srcDevDes;
    int Port;
    int PortL;
    int PortH;
    int delta;
    int retval = -1;
    int socketindex = -1;
    string s;

    if (WNATentries.size() == 0)
        return -1;

    srcDevDes = OurDevices.getDevDes(be.SrcDevDesIndex);  // Get the source  device designator the message came in on

    if (srcDevDes.size() == 0)
        return -1;   // cannot do anything with an unknown Source Device Designator

    s = OurDevices.interfaces[be.SrcDevDesIndex];               // the Linux interface

    // See if this message came FROM on a Ethernet port that needs translation to a destination ID
    if (OurDevices.IsEth(be.SrcDevDesIndex)){
        // This message came from an ethernet interface. Lets see if the inbound port is within our translation range
        Port = be.PortIn;  // the ethernet port this message arrived on
        // Is it within our WNAT range? Check all WNAT entries.
        //cout << "Check WNAT->" << OurDevices.getDevDes(be.DevDesIndex) << " Port:" + intToString(i) << "-> ID:" + intToString(be.dstID) << endl;

        wnatlock.lock();     // Temporarily lock the list

        for (it = WNATentries.begin(); it != WNATentries.end(); it++ ){
            if ( it->Designator.size() > 0){
                if (srcDevDes == it->Designator){
                    PortL = OurDevices.getPortNum(be.SrcDevDesIndex);
                    PortH = (PortL+ it->PortCount - 1);
                    if ((Port >= PortL) && (Port<= PortH) && (it->PortCount > 0)){
                        // It is an address needing WNAT translation
                        delta = Port - PortL;
                        be.dstID = it->lowerID + delta;  // change the destination ID for this message
                        FromLanCount++;
                        //cout << "WNAT Des match" << it->Designator << " to " << be.dstID  << endl;
                        s = LocalTimeStamp() + " " + OurDevices.getDevDes(be.SrcDevDesIndex)
                            + " Port:" + intToString(Port) + "-> ID:" + intToString(be.dstID);
                        //cout << "WNAT " << s << endl;
                        StoreHistory(s);    // store the WNAT event in the history
                        retval = be.dstID;  // exit the sub
                    }
                }
            }
        }
        wnatlock.unlock();     // Unlock the WNAT list

        // Now see if the message came IN on the default WNAT port.
        // If so, we'll set the destination ID to the last ID of an output message as long as the
        //    default devdes is still connected.
        wnatlock.lock();     // Temporarily lock the list
        for (it = WNATentries.begin(); it != WNATentries.end(); it++ ){
            if ( it->DefaultDevDes.size() > 0){
                // Did it come in the default devicedesignator?
                if (srcDevDes == it->DefaultDevDes){
                    // Yes. Someone sent data into the default DevDes for this WNAT.
                    socketindex = OurDevices.DevDesSocketIndex(be.SrcDevDesIndex);
                    if (socketindex >= 0){
                        be.dstID = tcpsockets[socketindex].srcID; // send the data back to the ID that originated the connection
                    }
                    cout << "Reverse WNAT:" << OurDevices.getDevDes(be.SrcDevDesIndex) << " Port:" << intToString(Port) << "-> ID:" << intToString(be.dstID) << endl;
                    s = LocalTimeStamp() + "Reverse WNAT " + OurDevices.getDevDes(be.SrcDevDesIndex)
                            + " Port:" + intToString(Port) + "-> ID:" + intToString(be.dstID);
                        //cout << "WNAT " << s << endl;
                    StoreHistory(s);    // store the WNAT event in the history
                    retval = be.dstID;  // exit the sub
                }
            }
        }
        wnatlock.unlock();     // Unlock the WNAT list

    }

    // Come here if there was no WNAT to be performed due to comming in on an ethernet PORT
    
    return retval;
}

// Return the new IP port to send this to if the message is destined TO an etherent port with WNAT
int WirelessNAT::PortTranslate(BinaryEntry& be, int& Port, int& DefaultPort){
    string DestDevDes;
    int ddPort;
    int IDL;
    int IDH;
    int delta;
    string DestIntf;
    string s = "";
    int i;
    int retval = -1;

    // Default to now WNAT
    Port = -1;
    DefaultPort = -1;

    if (WNATentries.size() == 0)
        return -1;

    DestDevDes = OurDevices.getDevDes(be.DstDevDesIndex);  // Get the source device designator the message came in on

    // We can only translate ethernet ports
    if (OurDevices.IsEth(be.DstDevDesIndex) == false)
        return -1;

    if (DestDevDes.size() == 0)
        return -1;   // cannot do anything with an unknown Device Designator

    // limit the history storage to a manageable number
    while (history.size() > MaxHistory){
        history.erase(history.end());
    }

   DestIntf = OurDevices.interfaces[be.DstDevDesIndex];               // the destination interface
   ddPort = OurDevices.getPortNum(be.DstDevDesIndex);                 // The device designator's TCP base port number

   // Must have a valid base port number to translate
   if (ddPort <= 0)
       return -1;

    wnatlock.lock();     // Temporarily lock the list

    // Is the source ID within our WNAT range and the destination a device that supports WNAT?
    for (it = WNATentries.begin(); it < WNATentries.end(); it++ ){
        if(DestDevDes == it->Designator){
            // This is destined for an interface used with WNAT
            IDL = it->lowerID;
            IDH = (it->lowerID + it->PortCount - 1);
            if ((be.srcID >= IDL) && (be.srcID <= IDH) && (it->PortCount > 0)){
                // It is an ID  needing translation to a port number
                delta = be.srcID - IDL;
                Port = ddPort + delta;  // get the port number to send this message to
                // Now figure out where to sent this if the WNAT port is closed  (fld_wnat_defaultdes)
                DefaultPort = -1;
                // Next line threw exception
                if (it->DefaultDevDes.size() > 0){
                   DefaultPort = OurDevices.getPortNum(OurDevices.IndexOf(it->DefaultDevDes));
                }
                // See if this port is connected/listening
                ToLanCount++;
                s = LocalTimeStamp() + DestDevDes + " ID:" + intToString(be.srcID) + "-> Port:" + intToString(Port) ;
                //cout << "WNAT src:" << OurDevices.getDevDes(be.SrcDevDesIndex) << " dst:" << DestDevDes  << " port:" << Port << endl;
                StoreHistory(s);    // store the WNAT event in the history
                retval = Port;      // exit the sub. Tell them where to route it.
                break;              // exit for...
            }
        }
    }
    wnatlock.unlock();     // Temporarily lock the list
   // No WNAT for this entry
    return retval;
}

// Return the default ID for this Device Designator / Port pair.
// Return -1 if WNAT is not on this pair
int WirelessNAT::GetID(string DevDes, int Port){
    int PortL, PortH, delta, retval;
    int dd;

    retval = -1;

    if (DevDes.size() == 0)
        return -1;

    if (Port <= 0)
        return -1;   // cannot do anything with an unknown Device Designator

    dd = OurDevices.IndexOf(DevDes);
    if (dd<0)
        return -1;

    // Is the source ID within our WNAT range and the destination a device that supports WNAT?
    wnatlock.lock();     // Temporarily lock the list
    for (it = WNATentries.begin(); it != WNATentries.end(); it++ ){

        if ( it->Designator.size() > 0){

            if (DevDes == it->Designator){

                PortL = OurDevices.getPortNum(dd);                 // The device designator's TCP base port number
                PortH = (PortL + it->PortCount - 1);
                if ((Port>=PortL) && (Port<= PortH) && (it->PortCount > 0)){
                    // It is an address needing translation
                    delta = Port - PortL;
                    retval = it->lowerID + delta;  // change the destination ID for this message
                    break;
                }
            }
        }
    }
    wnatlock.unlock();     // Temporarily lock the list

    return retval;
}

// Return the default ID for this Device Designator / Port pair.
// Return -1 if WNAT is not on this pair
bool WirelessNAT::GetEntry(int I, WNATEntry& wne){

    wne.Comment = "";
    wne.DefaultDevDes = "";
    wne.Designator = "";
    wne.PortCount = 0;
    wne.lowerID = 0;

    if ((I > WNATentries.size()) || (I < 0)){
        return false;
    }

    // Is the source ID within our WNAT range and the destination a device that supports WNAT?
    wnatlock.lock();     // Temporarily lock the list
    wne = WNATentries[I];
    wnatlock.unlock();     // Temporarily lock the list
    return true;
}



void WirelessNAT::AddWNAT(string DevDes, int count, int BaseID, string DefaultDes, string comment) {

    WNATEntry NewWNAT;

    // Verify that parameters are valid
    if ((count < 1) || (DevDes.size()== 0) || (BaseID < 1))
        return;

    if (DefaultDes.size() == 0)
        DefaultDes = "";

    if (comment.size() == 0)
        comment = "";

    // Create a new entry in the WNAT table for this block of IDs
    wnatlock.lock();     // Temporarily lock the list

    if  (WNATentries.size() < MaxWNATentries){
        NewWNAT.PortCount = count;
        NewWNAT.lowerID = BaseID;
        NewWNAT.Designator = trim(DevDes);
        NewWNAT.Comment = comment;
        NewWNAT.DefaultDevDes =  DefaultDes;
        WNATentries.push_back(NewWNAT);
    }
    wnatlock.unlock();     // Temporarily lock the list

}

void WirelessNAT::ClearAll(void) {

   
    // Create a new entry in the WNAT table for this block of IDs
    wnatlock.lock();     // Temporarily lock the list
    WNATentries.clear();
    wnatlock.unlock();     // Temporarily lock the list

}


int WirelessNAT::StoreHistory(string s) {

    historylock.lock();     // Temporarily lock the list
    history.insert(history.begin(), s);  // store the WNAT event in the history

    // Trim the hsitor to a managable size
    while (history.size() > MaxHistory){
        history.erase(history.end());
    }
    historylock.unlock();     // Temporarily lock the list
}




