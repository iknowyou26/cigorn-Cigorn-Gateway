/* 
 * File:   Cigorn.h
 * Author: john
 *
 * Created on July 27, 2010, 9:13 PM
 */

#ifndef _CIGORN_H
#define	_CIGORN_H

// The inlcudes we will include in all program files
#include <time.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>   
#include <stdio.h>
#include <pthread.h>
#include <queue>
#include <exception>
#include "limits.h"

#include "GlobalVar.h"
#include "io_utilities.h"
#include "functions.h"
#include "settings.h"
#include "ascii.h"
#include "ourstructures.h"
#include "language.h"
#include "Access.h"
#include "datarow.h"
#include "TableDefs.h"
#include "mainsubs1.h"
#include "htmlformatter.h"
#include "BinaryEntry.h"
#include "logger.h"
#include "coutput.h"
#include "VT100.h"
#include "safestring.h"

#define APP_TITLE  "Cigorn Gateway"

#define REV_MAJOR  5
#define REV_MINOR  0
#define REV_BUILD  1

#define INIFILE       "cigorn.ini"
#define WIRELESSFILE  "Wireless.ini"
#define OUTPUTFILE    "Activity.txt"     // Log the activity of the program operation
#define ERRORLOGFILE  "ErrorLog"         // log file for errors to be recorded in
#define LOGEXTENSION  "log"              // log file extension for errors to be recorded in
#define LOGDIRNAME    "logs"             // directory where log files are stored.
#define EMAILLOGFILE  "EmailLog.log"

// Constants related to site management
#define DEFAULTSII    20     // default interval between site identification packets being sent out
#define DEFAULT_WDDBI 15     // default time of how often an intersite message to update the WD database
#define DEFAULT_WDROWS 5     // The number of rows we will have in the WD table update intersite message
#define DEFAULT_DBPUSH 60    //
#define MIN_DBPUSH     5     // Interval between updates to the back end database

// Maximum constants related to the system architecture
#define MAXSITENUM    999   // Maximum number of gateway sites
#define MAXNAPNUM     999   // Maximum number of Network Access Points on this
#define MAXRFPNUM     999           // Maximum RF Port number
#define MAXWDID      0x7fffffff     // The biggest ID that a WD can have

// Global constants related the communications with devices
#define MAX_TTY          64      // maximum number of serial ports on this machine
#define MAXSOCKETS       3500    // the maximum number of TCP sockets we will ever have open
#define MAXDEVDES        100     // The maximum bumber of devicedesignators we will ever create
#define MAX_ETH          10      // Maximum number of ether net interfaces we can deal with.
#define CIGSERVPORT      5443    // The Server TCP socket we will communicate to a standby/primary gateway
//#define CIGCLIPORT       5444    // The Client TCP socket we will communicate to a standby/primary gateway

// User interface options
#define MAX_CLI_LENGTH      1024
#define STAT_DISPLAY_PORT   1

// Command-line options
#define NO_COUT_REDIR     "/ncr"

#define CoutM0(a)   if(messagelevel >= MSG_NONE ) a
#define CoutM1(a)   if(messagelevel > MSG_NONE ) a
#define CoutM2(a)   if(messagelevel >= MSG_DEBUG ) a

// The Command-line listing column widths
#define w_ID        7
#define w_system    4
#define w_enabled   3
#define w_count     8
#define w_timestamp 23
#define w_devdes    12
#define w_device    12
#define w_comment   40
#define w_ipadd     20
#define w_port      8
#define w_interface 8
#define w_rssi      7

// The microsleeper time periods that the various routines sleep
#define COMMSLEEP  1000      // number of microseconds to sleep each loop through the communications loop
#define SOCKSLEEP  500       // number of microseconds to sleep each loop through the TCP socket loop
#define MAINSLEEP  1000      // number of microseconds to sleep each loop through the main loop

#define MAXDLYQSIZE 32        //number of entires in the q that measures delays through the router
#define MAX_WDC_PURGE  1000   // Maximum loops through the Q purge routine. Use as watchdog limit

// Constants related to the ethernet/TCP configuration
#define DEFAULTWEBPORT 25002            // port number for our web server
#define CLIENT_PORT_BASE  10000         // TCP socket clients use port number 10,000 on up
#define SERVER_PORT_RES    5000         // The range of server IP addresses to use.  BASE to BASE+SERVER_PORT_RES

// Miscelaneous constants
#define SecondsInAyear  31556900

#endif	/* _RAVEONNET_H */

