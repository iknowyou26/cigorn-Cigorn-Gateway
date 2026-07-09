/* 
 * File:   globalvar.h
 * Author: john
 *
 * Created on July 28, 2010, 11:33 PM
 */

#ifndef _GLOBALVAR_H
#define	_GLOBALVAR_H

#include <string>
#include "strings.h"
#include "health.h"
#include <queue>
#include <vector>
#include "ourstructures.h"
#include "database.h"
#include "database.h"
#include "datatable.h"
#include "datarow.h"
#include "emailer.h"
#include "logger.h"
#include "webserver.h"
#include "UserCLI.h"
#include "coutput.h"
#include "cypher.h"
#include "Security.h"

// Communication queue structures

#define iUNKNOWN_IO        0   // unknow interface type
#define iSOCK              1   // Socket interface
#define iCOM               2   // COM/TTY/RS232 interface
#define iCLI               4   // Command Line Interface
#define iHOST              8   // By this Host computer software
#define iINI              16   // by the ini fil;e parser


#define STAT_INTERFACE  ""

#define SYStxt      "SYS"    // A system device not used for gateway communications
#define ALLtxt      "ALL"    // Matches all interfaces

extern string Application;
extern string AppVersion;
extern bool maininitialized;

// The object to report health
extern health SysHealth;

// Our data tables. They are on the heap.
extern datatable *dtWD;
extern datatable *dtRT;     // Our data routes. Definition of what type of message from where gets routed to who.
//extern datatable *dtPR;     // Our protocols.  List of typical protocols we parse and route.
extern datatable *dtEDD;    // Eth Device designators
extern datatable *dtTDD;    // TTY Device designators
extern datatable *dtWNAT;   // Wireless NAT entries
extern datatable *dtPagers;        // Pagers table
extern datatable *dtSC;            // SiteConfiguration table.

extern webserver myWeb;     // www interface to cigorn
extern UserCLI  MyCLI;
extern string cmd_password_hashed;
extern cypher  myCrypto;    // basic crypto functions


// Global Statistics
extern long nmeacount;
extern long pravecount;
extern long wmxcount_in;
extern long xmlcount_in;
extern long Cigorncount_in;
extern long Cigorncount_out;
extern bool iniOK;
extern long routecount;
extern long FailedSockOut;
extern long FailedTTYOut;

// Global variables
extern std::streambuf* old_cout;   // the output buffer before we took it over.
extern std::ofstream  my_cout;

extern logger elog;           // our error log
extern logger mlog;           // our email log

extern coutput MyCout;        // redirection for the console output debug messages

enum mlevel{
    MSG_NONE,
    MSG_STATUS,
    MSG_DEBUG
};

extern mlevel messagelevel;      // message level 0=none, 1=status, 2=status, progress, and debug

extern Gateway Me;                 // the settings for this gateway
extern map<string, Gateway> GateWays;
using namespace std;

extern database myDB;
extern string dbUser;
extern string dbPass;
extern string dbName;
extern string dbHost;
extern string dbType;
extern int tm_seconds;             // how many seconds we have been running
extern queue<string> qCLIin;
extern bool ShutDownApplication;
extern bool RestartApplication;    // Set true to restart and reload tables
extern bool AppIsRunning;
extern time_t boot_time;

extern double statusemailinterval;    // number of hours between email notices.  -1 means never.
extern string webusername;
extern string webpassword;
extern int consoleport;

// Our object locks
extern pthread_mutex_t qlock;       // Lock the queus when we write or pop them.
extern pthread_mutex_t devlock;     // Lock for the device array accexss
extern pthread_mutex_t ttylock;     // Lock for the device array accexss
extern pthread_mutex_t socklock;    // Lock for the tcp/ip socket opening/manipulating
extern pthread_mutex_t addlistlock; // Lock for the tcp/ip socket opening/manipulating
extern pthread_mutex_t cmdqlock;    // Lock for the tcp/ip socket opening/manipulating
extern pthread_mutex_t dlyvlock;    // Lock for the delay vector structure that records delay in the router
extern int maxQcount;               // Maximum number of entries we allow in the queue
extern int maxQage;                 // Maximum age of messages in the queue, in seconds

#include "Router.h"
extern Router DataRouter;           // Create an instance of our data router

typedef std::map<int, WDupdate>  WDupdateList;

extern int TicksPerSecond;  // the default timer tickes-per-second

// The network management globals
extern int emailnotice;        // email notification level.  0=none, 1=minimal, 2=normal, 3=verbose.
extern emailer myEmail;        // create an email client to send email notices out

// The variables used for hot-standby and health monitoring of the remote gateway
extern int StandbyHealthTime;       // how often we check the health of the primary gateway if we are a hot-standby
extern Security SyncSecurity;              // handles data encryption for the inter-cigorn communication

// OS Signal counts
extern int sigHupCount;
extern int sigPipeCount;

#endif	/* _GLOBALVAR_H */

