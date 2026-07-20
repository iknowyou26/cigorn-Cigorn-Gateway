/* 
 * File:   Router.cpp
 * Author: Ryan Le
 * 
 * Created on 07/15/2026
 */

#include "Router.h"
#include <iostream>
#include <string>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "platform/thread/PlatformLockGuard.h"
#include <ios>
#include <limits>
#include <queue>
#include <vector>
#include <locale>

#include "Cigorn.h"     // Our application-specific constants
#include "CommThread.h"
#include <unordered_map>
#include "Translator.h"

using namespace std;

map<int, routeEntry>  RouteTable;               // details about all of our routes. 1-based index
map<unsigned long, routehistory> RecentRoutes;  // Used to detect duplicates

Router::Router() {
    msgcount = 0;      // count how many messages we process.
    duplicatecount = 0;
}

Router::Router(const Router& orig) {
}

Router::~Router() {
}

int Router::RecentRouteCount(void ) {

    return RecentRoutes.size();
}


// Add an entry to the router table
bool Router::AddRoute(
    int Format,
    string srcif,
    string dstif,
    int lowID,
    int upID)
{
    routeEntry NewEntry;
    int i = static_cast<int>(RouteTable.size()) + 1;

    NewEntry.format = Format;
    NewEntry.srcDevDes = srcif;
    NewEntry.dstDevDes = dstif;
    NewEntry.lowerID = lowID;
    NewEntry.upperID = upID;

    {
        cigorn::PlatformLockGuard lock(tablelock);
        RouteTable.insert(make_pair(i, NewEntry));
    }

    return true;
}

bool Router::ClearAll()
{
    cigorn::PlatformLockGuard lock(tablelock);
    RouteTable.clear();
    return true;
}

bool Router::AddRoute(routeEntry NewEntry)
{
    int i = static_cast<int>(RouteTable.size()) + 1;

    {
        cigorn::PlatformLockGuard lock(tablelock);
        RouteTable.insert(make_pair(i, NewEntry));
    }

    return true;
}

int Router::RouteCount(void){

    return RouteTable.size();
}

// Take the message Min that just came out of the inbound queue
// and route it; if it is a routable protocol.
bool Router::DoRouting(BinaryEntry Min)
{
    switch (Min.format) {
        case fmtASCII:
        case fmtPRAVE:
        case fmtWMX:
        case fmtPageASCII:
        case fmtWMXPOCSAG:
            return DataRouter.RouteMSG(Min);

        case fmtINVALID:
        case fmtNMEA:
        case fmtCigorn:
        case fmtXML:
        case fmtESRI_CSV1:
            return false;
    }

    return false;
}

// ****************************************************************************
// The router function.  Loop through the entries in the route table
// and see if this message pertains to any entry. If so, route it to
// the destination interface per the entry in the route table.
// ****************************************************************************
bool Router::RouteMSG(BinaryEntry msg){
    int i;
    int ddIndex;
    string s;
    bool wildmatch = false;
    RouteInfo ri;
    stringstream out;

    bool routed = false;
    BinaryEntry XMsg;    // the protocol translated version of the message

    map<unsigned long, routehistory>::iterator it;

    // See if we recently routed this same message.
    // Just to be safe, keep the list reasonable
    while(RecentRoutes.size() >  MAXROUTEHISTORY ){
        // we should never get here
        it = RecentRoutes.begin();
        RecentRoutes.erase(it);
    }
  {
    cigorn::PlatformLockGuard lock(tablelock);
    if (msg.SrcDevDesIndex < 0 || msg.SrcDevDesIndex >= MAXDEVDES)
        return false; // invalid index

   string origInt = OurDevices.designator[msg.SrcDevDesIndex];   // get the name of the interface where this message originated from


    // DEBUG ............
    //    if ((msg.data[0]=='+') && (msg.data[1]=='+') && (msg.data[2]=='+') && (msg.bcount == 3)){
    //      coutm2 << "Removed +++ inbound on " << origInt << endl;
    //    return false;
    // }
  
    CoutM2(out) << "Message from ID:" << cBOLDITEM << msg.srcID << cNORMAL << " to ID:" << msg.dstID << " Format:"
            << ProtocolName(msg.format)  << " Source:" << origInt << endl;

    if (out.str().size() > 0){
         MyCLI.OutputText(out.str());   // send the text to the console output
         out.str("");                   // clear the buffer
    }
    
    

    // Loop through the route table to see if this message should be routed.
    for (i=1; i<= RouteTable.size(); i++){
        // CoutM2(out) << " Scan Routes" << i << endl;
        if ((RouteTable[i].format == msg.format) || (RouteTable[i].format == fmtALL)){
            // This route table entry applies to this format (gps position data)
            // Is the source interface one this entry designates as an OK source?
            wildmatch = WildCardMatch(RouteTable[i].srcDevDes, origInt);

            // CoutM2(out) << "  Chk:" << RouteTable[i].srcDevDes << " = " << origInt << "?" << endl; ;

            if ((RouteTable[i].srcDevDes == origInt) || (RouteTable[i].srcDevDes== ALLtxt) || wildmatch){
               // The interface is a match also. Is the ID within range for this entry in the route table?
               //cout << "RR Source match" << endl;

                if ((RouteTable[i].lowerID == -1) || ((msg.dstID >= RouteTable[i].lowerID) && (msg.dstID <= RouteTable[i].upperID)))
                { // This message is supposed to be routed. 
                    // See if there is any protocol translation that needs to take place.
                    ddIndex = OurDevices.IndexOf(RouteTable[i].dstDevDes);
                    if (ddIndex >= 0){
                        CoutM2(out) << "  Routing to:" << OurDevices.getDevDes(ddIndex) << " srcID=" << msg.srcID << cNORMAL << endl;
                        if (wildmatch)
                            CoutM2(out) << " matched:" << cBOLDITEM << RouteTable[i].srcDevDes << cNORMAL;

                        CoutM2(ss) << " dstID=" << msg.dstID << " protocol:" << ProtocolName(msg.format) << endl;

                        msg.DstDevDesIndex = ddIndex;        // set the destination devicedesignator index
                        
                        // Protocol translater before we route the message
                        ProtocolTranslate(msg, XMsg);        // Protocol translate it if need be.
                        // Keep the WD statistics. Update the stats in the dtWD table
                        if (IsNewMessage(OurDevices.getDevDes(XMsg.DstDevDesIndex), XMsg.data, XMsg.bcount, XMsg.format ) == false){
                            duplicatecount++;
                            //CoutM2(out) << "Duplicate discarded. srcID=" << XMsg.srcID << " dstID=" << XMsg.dstID << endl;
                        }else{
                            // OK to Route the message
                            if (RouteBinTo(XMsg) == false){
                                // could not find an outbound queue to put it in
                                CoutM2(out) << "Failed to add to outbound queue." << endl;
                            };
                            
                            // Store it in the history queue
                            ri.destID = XMsg.dstID;
                            ri.srcID = XMsg.srcID;
                            ri.srcDevDes = origInt;
                            ri.dstDevDes = OurDevices.getDevDes(XMsg.DstDevDesIndex);
                            ri.format = XMsg.format;
                            ri.bytecount = XMsg.bcount;
                            ri.timein = time(NULL);
                            LastRoutes.insert(LastRoutes.begin(), ri);
                            while (LastRoutes.size() > MAXROUTELIST){
                               LastRoutes.pop_back();
                            }
                            if ((XMsg.dstID != NULL_ID) && dtWD->RowPresent(XMsg.dstID)) {
                                // Update the WirelessDevice table with the statistics about this message
                                dtWD->AddToItem(XMsg.dstID, fld_countTo, 1);             // Add 1 to the countFm field for this WD with ID FROM
                                dtWD->AddToItem(XMsg.dstID, fld_countToD, 1);            // Increment the daily from field for this WD
                            }

                        }
                        //cout << "Route: " << i << " " << RouteTable[i].srcDevDes << " to " << OurDevices.getDevDes(msg.DstDevDesIndex) << " Dest ID:" << msg.dstID << "{" << RouteTable[i].lowerID << "-" << RouteTable[i].upperID << "}" <<endl;
                        routed = true;
                    }
                }
            }
        }
    }
 }

    // Output the debug text if there is any
    if (out.str().size() > 0){
         MyCLI.OutputText(out.str());   // send the text to the console output
         out.str("");                   // clear the buffer
    }

    return routed;

}

// Verifies that the message sent to this destination interface is not the same as one just sent
bool Router::IsNewMessage(string intf, char *mdata, int n, int mformat){

    double deltaT;
    bool isNew = true;        // assume it is a new message (not duplicate)
    bool CheckForDup = true;
    unsigned long l;
    int MaxAge = OLDESENTRY;
    int x;
    string header="";
    int MaxCheckLen = n;
    int count = 0;

    switch (mformat){
            case fmtINVALID:
                CheckForDup = false;
                break;
            case fmtPRAVE:
                // Raveon PRAVE message came in. Only check first 6 fields for uniqueness (excludes RSSI)
                x = PositionOfField(mdata, 7, n);
                if ((x>0) && (x < n))
                    MaxCheckLen = x;
                CheckForDup = true;
                break;
            case fmtWMX:
                // Raveon WMX message came in
                CheckForDup = true;
                break;
            case fmtNMEA:
                // Generic NMEA message, no IDS.
                CheckForDup = false;
                break;
            case fmtCigorn:
                // XML from a Cigorn site
                CheckForDup = true;
                break;
            case fmtXML:
                // XML document
                CheckForDup = true;
                break;
            case fmtASCII:
                // OK to route ASCII data
                CheckForDup = false;   // can't check for duplicates
                break;
            case fmtPageASCII:
            case fmtWMXPOCSAG:
                CheckForDup = false;
                break;
        }

    routehistory  thisroute;
    map<unsigned long, routehistory>::iterator it;

    thisroute.hash = ddhash(mdata, MaxCheckLen, intf);
    thisroute.devdes = intf;
    thisroute.timein = time(NULL);  // get the current time

    // Build up a debug string to show part of the message
    for (x=0; x < n; x++){
        header = header + mdata[x];
        if (x > MaxCheckLen)
            break;
    }
    
    //CoutM2(ss) << "Message:" << header << endl;
    //CoutM2(ss) << "[" << n << "] to " << intf << "  UID=" << thisroute.hash << endl;

    // If the container is getting full, trim it more agressively
    if (RecentRoutes.size() > MAXROUTEHISTORY)
        MaxAge = MaxAge /2;

    // check all of the recent routes for this one
    for (it = RecentRoutes.begin(); it != RecentRoutes.end(); it++){
        deltaT = difftime (thisroute.timein, it->second.timein);  // get the age in seconds
        //cout << "TST:" << it->second.devdes << "=" << thisroute.devdes << "   " << it->second.hash << "=" << thisroute.hash << "   " << deltaT << endl;
        if ((it->second.hash == thisroute.hash) && (deltaT <= DUPTIMEOUT)){
            count++;
            // Dupliciate
            isNew = false;
            //cout << "DUPLICATE:" << it->second.devdes << " " << it->second.hash << "  " << deltaT << "  Fmt:" << mformat << endl;
            break;
        }
    }

    if (isNew == true){
        // The route is not in the recent route queue. Add it.
        //cout << "New msg in RecentRoutes" << endl;
        RecentRoutes.insert(make_pair(thisroute.hash, thisroute));
        RecentRoutes.at(thisroute.hash).timein = thisroute.timein;
        RecentRoutes.at(thisroute.hash).devdes = thisroute.devdes;
    }

    // For safety do this.
    while (RecentRoutes.size()> MAXROUTEHISTORY){
        RecentRoutes.erase(RecentRoutes.begin());
    }

    if (CheckForDup == false)
        isNew = true;   // we can't check for cuplicates, so always return true

    return isNew;

}

// Remove old messages from the map to keep its size maageable
void Router::CleanMessageMap(void){

    double deltaT;
    bool isHere = false;
    unsigned long l;
    int MaxAge = OLDESENTRY;
    int x;
    time_t timenow = time(NULL);

    map<unsigned long, routehistory>::iterator it;

    vector <unsigned long> DeleteList;
    vector <unsigned long>::iterator  dli;


    // If the container is getting full, trim it more agressively
    if (RecentRoutes.size() > MAXROUTEHISTORY)
        MaxAge = MaxAge /2;

    // check all of the recent routes for this one
    for (it = RecentRoutes.begin(); it != RecentRoutes.end(); it++){
        deltaT = difftime (timenow, it->second.timein);
        // Now prune any old entries in the routing history structure
        if (deltaT > MaxAge )
            DeleteList.insert(DeleteList.begin(), it->first);     // remember old entries.
    }

    // Are there are some things we need to delete?
    for (x=0; x< DeleteList.size(); x++){
        // Yes. Delete them
        l = DeleteList.at(x);
        // cout << "deleteting " << l << " " << (time(NULL) - RecentRoutes.at(l).timein) << endl;
        RecentRoutes.erase(l);
    }

    // For safety do this.
    while (RecentRoutes.size()> MAXROUTEHISTORY){
        RecentRoutes.erase(RecentRoutes.begin());
    }

    return ;

}



// retrieve an entry in the route table.
routeEntry Router::RouteTableEntry(int i){
    std::stringstream ss;
    string s;
    routeEntry re;

cigorn::PlatformLockGuard lock(tablelock);           // route table lock

    // Loop through the route table to see if this message should be routed.
    if (i <= RouteTable.size()){
        re.lowerID = RouteTable[i].lowerID;
        re.upperID = RouteTable[i].upperID;
        re.srcDevDes = RouteTable[i].srcDevDes;
        re.dstDevDes = RouteTable[i].dstDevDes;
        re.format =  RouteTable[i].format;
    }else{
        re.srcDevDes= "";
        re.dstDevDes = "";
        re.lowerID = -1;
        re.upperID = -1;
        re.format = -1;
    }

    return re;
}

/**
 * Returns a list of device indices which are destinations of messages to the 
 * given device designator.
 * @param destinationDeviceDesignator The source for all routes returned
 * @param routeList A vector in which to place the found device indices
 * @return Number of routes found
 */
int Router::getDestinations(string sourceDeviceDesignator, vector<int>& deviceList){
    int foundCount = 0;
    
    for (int i=1; i<= RouteTable.size(); i++){
        if(RouteTable[i].srcDevDes == sourceDeviceDesignator){
            deviceList.push_back(OurDevices.IndexOf(RouteTable[i].dstDevDes));
            foundCount++;
        }
    }
    
    return foundCount;
}

string RouteHistoryToString(int count)
{
    return "";
}

// Create a string with all of the rout table entries
string Router::RouteTableToText(void)
{
    map<unsigned long, routehistory>::iterator it;
    string s = "";
    int i = 0;

    {
        cigorn::PlatformLockGuard lock(tablelock);

        for (i = 1; i <= RouteTable.size(); i++) {
            s = s + "Route #" + intToString(i) + " ";
            s = s + RouteTable[i].srcDevDes + " to " +
                RouteTable[i].dstDevDes;
            s = s + " Format=" +
                ProtocolName(RouteTable[i].format);

            if (RouteTable[i].lowerID >= 0) {
                s = s + " [" +
                    intToString(RouteTable[i].lowerID) + " " +
                    intToString(RouteTable[i].upperID) + "]";
            } else {
                s = s + " [ALL]";
            }

            s = s + (char)'\r' + (char)NL;
        }
    } // closes lock scope

    return s;
}

string Router::ProtocolName(int r){

    switch (r){
        case fmtINVALID:
            return fmtINVtxt;
        case fmtALL:
            return fmtALLtxt;
        case fmtWMX:
            return fmtWMXtxt;
        case fmtPRAVE:
            return fmtPRAVEtxt;
        case fmtNMEA:
            return fmtNMEAtxt;
        case fmtXML:
            return fmtXMLtxt;
        case fmtCigorn:
            return fmtCigorntxt;
        case fmtASCII:
            return fmtASCIItxt;
    }
    return "?(" + intToString(r) + ")";
}

int Router::ToMsgFormat(string s){

    s = StringToUpper(s);
    if (fmtALLtxt == s)
        return fmtALL;
    if (fmtWMXtxt == s)
        return fmtWMX;
    if (fmtPRAVEtxt == s)
        return fmtPRAVE;
    if (fmtESRI_CSV1txt == s){
        return fmtESRI_CSV1;
    }

    // not found.
    return fmtINVALID;

}

// Route the text data to the proper outbound queue  (string, char*,  int )
bool  Router::RouteBinTo(string dest, char* txt, int n){

    int i;
    BinaryEntry NewEntry;

    CoutM2(ss) << "Routing to " << dest << "  " << txt << endl;
    
    if (n >= MAX_DATA)
        n = MAX_DATA;   // too much data to route.

    i = OurDevices.IndexOf(dest);  // get the index of the device type with name

    if (i >=0 ){
      // This device is where the data should be routed to
      NewEntry.DstDevDesIndex = i;        // remember the desitnation device for this entry
      CopyCstr(NewEntry.data, txt, n);    // copy the data to the new queue object. NUL terminate
      NewEntry.bcount = n;                // remember how many buytes we have
      NewEntry.timein = TimeNow();        // remember when we created this entry
      NewEntry.PortIn = -1;
      //cout << NewEntry.data << endl;
      return RouteBinTo(NewEntry);
    }
    return false;

}

// Route the binary data to the proper outbound queue  (int, char*,  int )
bool  Router::RouteBinTo(int intf, char* txt, int n){

  BinaryEntry NewEntry;

  NewEntry.DstDevDesIndex = intf;     // Remember the destination device for this entry
  NewEntry.timein = TimeNow();
  CopyCstr(NewEntry.data, txt, n);    // Copy the data to the new queue object. NUL terminate
  NewEntry.bcount = n;                // Remember how many bytes we have
  return RouteBinTo(NewEntry);

}

// Route the binary data to the proper outbound queue
// The destination interface for for the message must be set before calling this.
bool  Router::RouteBinTo(BinaryEntry NewEntry){
  if (OurDevices.IsTTY(NewEntry.DstDevDesIndex)){
      {
    cigorn::PlatformLockGuard lock(tablelock);

    // protected code
}
      routecount++;                   // count the number of messages we route
     // cout << "Routing to " << dest << " " << NewEntry.data << endl;
      return true;
  }else{
      {
    cigorn::PlatformLockGuard lock(tablelock);

    // protected code
}
      routecount++;                   // count the number of messages we route
      return true;
  }
}

/**
 * Pause streams matching the filter
 * @param devDesFilter Device designator filter with wildcards enabled
 * @param pauseInput True if the input stream should be paused
 * @param pauseOutput True if the output stream should be paused
 * @param unpauseTime Time to pause for, in milliseconds
 * @return Number of streams paused
 */
int Router::pauseStreams(string devDesFilter, bool pauseInput, bool pauseOutput, double unpauseTime){
    int pauseCount = 0;
    
    // Look for any serial ports that should be paused and pause them
    for(int i = 0; i < MAX_TTY; i++){
        int devIndex = COMport[i].myDevIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                pauseCount++;
                if(pauseInput){
                    COMport[i].pauseInputUntil(unpauseTime);
                }
                if(pauseOutput){
                    COMport[i].pauseOutputUntil(unpauseTime);
                }
            }
        }
    }
    
    // Look for any network connections that should be paused and pause them
    for(int i = 0; i < MAXSOCKETS; i++){
        int devIndex = tcpsockets[i].myDevDesIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                pauseCount++;
                if(pauseInput){
                    tcpsockets[i].pauseInputUntil(unpauseTime);
                }
                if(pauseOutput){
                    tcpsockets[i].pauseOutputUntil(unpauseTime);
                }
            }
        }
    }
    
    return pauseCount;
}

/**
 * Unpause streams matching the filter
 * @param devDesFilter Device designator filter with wildcards enabled
 * @param unpauseInput True if the input stream should be unpaused
 * @param unpauseOutput True if the output stream should be unpaused
 * @return Number of streams unpaused
 */
int Router::unpauseStreams(string devDesFilter, bool unpauseInput, bool unpauseOutput){
    int unpauseCount = 0;
    
    // Look for any serial ports that should be paused and pause them
    for(int i = 0; i < MAX_TTY; i++){
        int devIndex = COMport[i].myDevIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                unpauseCount++;
                if(unpauseInput){
                    COMport[i].unpauseInput();
                }
                if(unpauseOutput){
                    COMport[i].unpauseOutput();
                }
            }
        }
    }
    
    // Look for any serial ports that should be paused and pause them
    for(int i = 0; i < MAXSOCKETS; i++){
        int devIndex = tcpsockets[i].myDevDesIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                unpauseCount++;
                if(unpauseInput){
                    tcpsockets[i].unpauseInput();
                }
                if(unpauseOutput){
                    tcpsockets[i].unpauseOutput();
                }
            }
        }
    }
    
    return unpauseCount;
}