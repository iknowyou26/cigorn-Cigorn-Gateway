/* 
 * File:   Router.h
 * Author: Ryan Le
 *
 * Created on 07/15/2026mber 10, 2010, 9:19 PM
 */

#ifndef _ROUTER_H
#define	_ROUTER_H


#include "ourstructures.h"
#include "BinaryEntry.h"
#include "platform/thread/PlatformLockGuard.h"
#include <string>
#include <queue>
#include <vector>
#include <sstream>

using namespace std;

// The message formats we support
#define    fmtINVALID   0     // Used to indicate a format is invalid or unsupported
#define    fmtALL       1     // deisgnator for all formats "*" in the command
#define    fmtWMX       2     // Raveon's WMX packet format
#define    fmtPRAVE     3     // Raveon's PRAVE and any other GPS position packet
#define    fmtNMEA      4     // Generic standard NMEA (not proprietary formats)
#define    fmtXML       5     // general XML documents
#define    fmtCigorn    6     // XML documents from/to Cigorn Gateways
#define    fmtASCII     7     // raw ascii charactors
#define    fmtESRI_CSV1 8
#define    fmtPageASCII 9     // Page requests, ASCII encoded
#define    fmtWMXPOCSAG 10

// These are the command-line text entries to indicate the various message formats
// MUST BE UPPER CASE!
#define fmtINVtxt       "INVALID"
#define fmtALLtxt       "ALL"
#define fmtWMXtxt       "WMX"
#define fmtPRAVEtxt     "PRAVE"   // Raveon's PRAVE and any other GPS position packet
#define fmtNMEAtxt      "NMEA"
#define fmtXMLtxt       "XML"
#define fmtCigorntxt    "CIGORN"
#define fmtASCIItxt     "ASCII"
#define fmtESRI_CSV1txt "ESRI_CSV1"

// Protocol information
#define NMEA_START      '$'
#define NMEA_MAXLEN      200      // we will only process a max of 999 bytes

#define NULL_ID         -1
#define DEFAULT_ID       0        // use this if the ID is unknown or we want to use a default ID

#define OLDESENTRY  10            // Don't keep any entries in the routing history table older than this many seconds.
#define MAXROUTEHISTORY 1000      // Just to be safe, limit the number of route history entries to this.
#define MAXROUTELIST  25          // How many routes we keep in the list of previous routes.
#define DUPTIMEOUT  2             // If we see the same data withing XX seconds, it may be a duplicate

// An entry in the router table
struct routeEntry{
   int format;                // the format code for this protocol format
   string srcDevDes;          // The device designator for the source of a message to route
   string dstDevDes;          // The device designator for the destination to route the message to
   int lowerID;
   int upperID;
};

// An entry in the router historical queue
struct RouteInfo{
   int format;                // the format code for this protocol format
   string srcDevDes;          // The device designator for the source of a message to route
   string dstDevDes;          // The device designator for the destination to route the message to
   int srcID;
   int destID;
   int bytecount;
   time_t timein;        // The time it is added to the route history structure
};

struct routehistory{
    string devdes;        // the device designator for the thing on the interface we are routing to.
    unsigned long hash;
    time_t timein;        // The time it is added to the route history structure
};


// typedef map<int, routeEntry> Routertable;

//extern Routertable RouteTable;             // details about all of our routes

class Router {
public:
    Router();
    Router(const Router& orig);
    virtual ~Router();
  
    bool RouteMSG(BinaryEntry);
    bool AddRoute(int ,string , string , int , int );
    bool  AddRoute(routeEntry);
    int RouteCount(void);
    long msgcount;
    long duplicatecount;
    bool RouteBinTo(string, char*,  int );
    bool  RouteBinTo(int , char* , int);
    bool  RouteBinTo(BinaryEntry);
    bool IsNewMessage(string , char *, int, int);
    string ProtocolName(int);
    string RouteTableToText(void);
    string RouteHistoryToString(int);
    int ToMsgFormat(string);
    bool DoRouting(BinaryEntry);
    routeEntry RouteTableEntry(int);
    int getDestinations(string sourceDeviceDesignator, vector<int>& deviceList);
    int RecentRouteCount(void );
    void CleanMessageMap(void);
    bool  ClearAll(void);
    int pauseStreams(string devDesFilter, bool pauseInput, bool pauseOutput, double unpauseTime);
    int unpauseStreams(string devDesFilter, bool unpauseInput, bool unpauseOutput);
    
   cigorn::PlatformLockGuard lock(tablelock);

    stringstream ss;

    vector<RouteInfo> LastRoutes;  // Messages that are queued up to be sent out this TTY port
    vector<RouteInfo>::iterator lrit;

private:
 
};

#endif	/* _ROUTER_H */

