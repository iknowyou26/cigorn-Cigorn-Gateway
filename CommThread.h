/* 
 * File:   commthread.h
 * Author: john
 *
 * Created on July 28, 2010, 11:21 PM
 */

#ifndef _COMMTHREAD_H
#define	_COMMTHREAD_H

#include <string.h>
#include "Cigorn.h"
#include "GlobalVar.h"
#include "CommandLine.h"
#include "serialhandler.h"
#include "TCPsocket.h"
#include "DeviceList.h"
#include "SiteManagement.h"
#include <list>       // for list
#include "WirelessNAT.h"
#include "PagerTable.h"

// The public variables
extern int NumberOfComPorts;        // The total number of COM ports we can access.
extern bool RedrawStatDisplay;
extern int SiteIdentifyInterval;   // how often we send a site identification message

typedef std::vector<BinaryEntry>  IQueue;

// Our message queues used to pass messages between threads and interfaces
extern queue <BinaryEntry> qTTYout;   // Data to send out a tty port
extern queue <BinaryEntry> qETHout;  // Data to send out a TCP socket
extern queue <BinaryEntry> qMSGin;      // WMX messages in
extern queue <BinaryEntry> qMSGout;     // WMX messages out

// Use these queues to monitor throughput
extern vector <double> dlyPRAVE;       // The delay through the router for a PRAVE message
extern vector <double> dlyWMX;         // The delay through the router for a WMX message
extern vector <double> dlyCIGORN;      // The delay through the router for a Cigorn message

// Command line processor and local console user interface
extern CommandLine cli;                // create the defaul CLI object to the interface to the user
extern CommandLine *pcli;

// Our variables
extern bool CommThreadInitialized;
extern bool StopCommunications;
extern bool CommunicationsRunning;
extern bool reloadEDDTable;

using namespace std;
using namespace Communications;

extern rs232 COMport[];       // Create an instance of the class rs232.

// TCP Socket information. Not used at this time..
struct socketinfo{
  bool valid;
} ;

struct socketstatus{
  int count_used;              // port number to use to connect on.
  int count_max;
  int count_connected;
  int count_servers;
} ;

extern socketinfo threadinfo;
extern tcpnet tcpsockets[];
extern tcpnet CigSocketIn;      // used to communicate to a standby or Primary gateway
extern tcpnet CigSocketOut;     // used to communicate to a standby or Primary gateway


extern DeviceList OurDevices;
extern SiteManagement SiteManager;    // The RF site management
extern WirelessNAT WNAT;              // Wireless device network address translator
extern PagerTable Pagers;

// Prototype the functions
void *threadComm( void * );
int InitializeComPorts(void);
void moutCLI(string);
socketstatus GetSocketStatus();
int DeviceBoundToTTY(std::string);
bool ConnectTTYdevice(int , std::string , int, std::string );
int AddConnection(string , string , string);
void PurgeOldMessages(void);
void startsocketthread(void);
int SocketCount(void);
int ConnectedSocketCount(void);
long SocketByteCount(void);
void ComputeTraffic(void);
void SyncToBackupGateway(void);
void SendCigornXML(string);
bool ResetSerial(string ResetDevDes);

#endif	/* _COMMTHREAD_H */

