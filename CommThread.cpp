// **************************************************************
// commthread.cpp
// Communications Thread
// The background thread for real-time communications.
// Handles byte-level in/out of the RS232 serial ports, passing\
// the raw data to the variopus message processing routines, the 
// command-line-interface, and any other I/O interfaces.  
// **************************************************************

#include <iostream>
#include <string>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>

#include <ios>
#include <limits>
#include <sys/ioctl.h>

#include "serialhandler.h"
#include "Cigorn.h"     // Our application-specific constants
#include "CommandLine.h"
#include "TCPsocket.h"
#include "CommThread.h"
#include "Matrix192x64.h"
#include "StatusDisplay.h"
#include "rfport.h"
#include "DeviceList.h"
#include "Router.h"
#include "ourstructures.h"
#include "Translator.h"
#include "SiteManagement.h"
#include "tinyxml/tinystr.h"   //http://www.grinninglizard.com/tinyxml/
#include "tinyxml/tinyxml.h"
#include "xmlFunctions.h"
#include "microsleeper.h"
#include "SocketThread.h"
#include "piper.h"
#include "EventTimer.h"
#include "sync-roles.h"
#include "POCSAGEncoder.h"
// Local prototypes

using namespace std;
using namespace Communications;


int NumberOfComPorts = 0;       // The total number of COM ports we can access.

// Strucures to hold info and statistics for the TCP socket threads.  One per socket.
socketinfo threadinfo;

// Array of interfaces to our TCP/IP sockets
tcpnet tcpsockets[MAXSOCKETS];
tcpnet CigSocketIn;     // used to communicate to a standby or Primary gateway
tcpnet CigSocketOut;    // used to communicate to a standby or Primary gateway

// Local subroutines
void CheckForCLIinput(void);     // bring in charactors from the console into the CLI FIFOvoid CheckConsoleThread(void);
void processMSGinput(void);
void HandleRS232Comms(void);
int GetSerialIndex(string);
bool ConnectTTYdevice(std::string, std::string , int );
int GetNewTCPindex(string);
int InitializeProtocols(parselist&);
void SendSiteIdentifier(void);
void SendWDtableUpdates (void);
void TTYOut_PopFront(int index);

bool RedrawStatDisplay = false;

// Statistics for communications
int loopcount = 0;
bool CommThreadInitialized = false;
bool StopCommunications = false;
bool CommunicationsRunning = false;

// Used for managing cases where we have to reload tables
bool reloadEDDTable = false;
enum TableState{Loaded, ReloadRequest, WaitForSocketThread, ReloadTables};
TableState tableState = Loaded;

int SiteIdentifyInterval = DEFAULTSII;   // how often we send a site identification message in seconds.
int SiteDBinterval = DEFAULT_WDDBI;      // how often we send around WD table update messages
int RowsPerDBupdate = DEFAULT_WDROWS;    // the default number of rows we update per WD table update

// Create the rs232 classes to talk the the COM ports
rs232 COMport[MAX_TTY];               // Create an instance of the class rs232.

piper MyPiper;                         // used to pipe ASCII data port-to-port around the route table

// Define our gloabl queues for passing messages around
queue <BinaryEntry> qMSGin;    // Messages into the router
queue <BinaryEntry> qTTYout;   // Data to send out a tty port
queue <BinaryEntry> qETHout;   // Data to send out a TCP socket

// Use these containers to monitor throughput
vector <double> dlyPRAVE;       // The delay through the router for a PRAVE message
vector <double> dlyWMX;         // The delay through the router for a WMX message
vector <double> dlyCIGORN;      // The delay through the router for a Cigorn message

DeviceList OurDevices;         // create a class to mimic an array of device type information

SiteManagement SiteManager;    // The RF site management

WirelessNAT WNAT;              // Wireless device network address translator
PagerTable Pagers;
EventTimer MyEventTimer;
microsleeper  CommSleeper(COMMSLEEP, 2);   // does microsecond sleeping. Stay awake N loops if we are busy, or sleep X uS if not.

void *threadComm( void *ptr )
{
    int i = 0;              // temp index
    string s = "";          // temp string
    int Key = NUL;
    int CommLoopCounter = 0;
    int CommRunningTime = 0;
    int StatInterfaceindex = -1;
    bool busy = false;
 
    time_t now_time = time(NULL);
    time_t last_time = time(NULL);
    struct tm tm;
    strptime("2000-01-01-18.46.40.000000", "%Y-%m-%d-%H.%M.%S", &tm);
    time_t last_sii = mktime(&tm);
    time_t last_wdi = mktime(&tm);
    time_t last_gwsync = mktime(&tm);
    stringstream ss;

    int iret1;
    pthread_t SocketThread;
    bool SocketThreadCreated = false;

    StatusDisplay frontdisplay;        // create an object to talk to the status display on the StatPort rs232 port

    while (!ShutDownApplication){

        cout << "Comm thread Starting." << "\r\n";

        // Add the front-panel display to the device list
        try{
            i = OurDevices.AddttyDevice("dStatDisplay", STAT_INTERFACE, SYStxt);  // get the 0-based index of this device type.
            ConnectTTYdevice(i, STAT_INTERFACE, DISPLAYBAUD, "N81");
        }
        catch(string e){
           CoutM1(ss) << "Error threadComm1. Cannot connect status display to " << STAT_INTERFACE << " " << e << endl;
           elog.store(string("Error threadComm1. " + e));
        }

        StatInterfaceindex = GetSerialIndex(STAT_INTERFACE);  // get the serial index for the ttyS0 or whatever is the STAT_INTERFACE

        OurDevices.LoadEthDevDesTable(dtEDD);  // read in the eth device designator table
        CoutM1(ss) << "Loaded " << OurDevices.LoadCount << " Eth Device Designators. " << OurDevices.ErrorsLoading << " errors. " << endl;
        OurDevices.LoadTtyDevDesTable(dtTDD);  // read in the tty device designator table
        CoutM1(ss) << "Loaded " << OurDevices.LoadCount << " Tty Device Designators. " << OurDevices.ErrorsLoading << " errors. " << endl;

        BuildWNATtable();                      // read in the WNAT table
        BuildPagerTable();
        
        if(StatInterfaceindex < 0)
           elog.store("Error 112. Status display failure.");
        else
           frontdisplay.Initialize(&COMport[StatInterfaceindex]);                                    // Initial boot screen

        // Create a thread for this instance of the classs to do socket communications.
        // Fill in the sockets structure so the socket knows its port to use, and where to store statistics.
        RunSocketThread = true;
        threadinfo.valid = true;  // this structure is unused now. Maybe some day we'll need to pass info the the thread using it.
        if (SocketThreadCreated == false){
            iret1 = pthread_create(&SocketThread, NULL, threadTCPserver, (void*) &threadinfo);
            SocketThreadCreated = true;
        }

        CommThreadInitialized = true;    // done initializing the COmmThread.  Run the loop
        frontdisplay.Refresh(true);

        sleep(0.05);

        // Output the debug text if there is any
        MyCLI.OutputText(ss.str());   // send the text to the console output
        ss.str("");                   // clear the buffer

        while (maininitialized == false){
           sleep(0.05);  // wait for the main loop to get running and finish its initialization
        }

        // The main has initialized itself, got all threads running, and read teh .ini file in
        //readini(WIRELESSFILE); // read in the wireless-specific settings such as TDMA parameters

        // Create the sockets to communicate to standby/primary Gateways on

        // Output the debug text if there is any
        MyCLI.OutputText( ss.str() );   // send the text to the console output
        ss.str("");                     // clear the buffer

        // Loop here while running looking for commands in or messages to send out.
        while ((!ShutDownApplication) && (!StopCommunications)) {
            switch (tableState) {
                case Loaded:
                    if (reloadEDDTable) {
                        tableState = ReloadRequest;
                    }
                    break;
                case ReloadRequest:
                    tableState = WaitForSocketThread;
                    deconstructSockets = true;
                    break;
                case WaitForSocketThread:
                    if (!deconstructSockets) {
                        tableState = ReloadTables;
                    }
                    break;
                case ReloadTables:
                    if (OurDevices.getValidSocketCount() == 0) {
                        OurDevices.Clear();
                        OurDevices.LoadEthDevDesTable(dtEDD); // read in the eth device designator table
                        reloadEDDTable = false;
                        tableState = Loaded;
                        OurDevices.LoadTtyDevDesTable(dtTDD);
                    } else {
                        tableState = ReloadRequest;
                    }
                    break;
            }

               CommLoopCounter++;       // statistics of how fast this loop runs
               busy = false;            // set true if enyting gets busy. Causes this loop to execute faster
               CommunicationsRunning = true;

               // Check to see if there is a new command in the CLI FIFO.
               MyEventTimer.start();

               if (MyEventTimer.timeinterval() > .1){
                   CoutM2(ss) << "slow CLI" << endl;
               }

               // Do the socket communications to/from standby and primary gateways
               ClusterSockets();

               // Now run the 1-second routines.  These are the slower background tasks we do every second
               now_time = time(NULL);

               // Run this display update routine every second, or when a Key was pressed.
               MyEventTimer.start();
               if ((last_time) != (now_time)){
                    // Handle the STATUS display connected to the RS232 serial port
                    frontdisplay.Refresh(RedrawStatDisplay);
                    RedrawStatDisplay = false;            // we did it
               }
               if (MyEventTimer.timeinterval() > .1){
                   CoutM2(ss) << "slow CLI" << endl;
               }

               //Other 1-second interval routines.
               if (last_time != now_time) {
                    last_time = time(NULL);
                    // Handle the STATUS display connected to the RS232 serial port
                    ComputeTraffic();               // compute bytes/minute statistics
                    DataRouter.CleanMessageMap();   // do this every so often to make sure our history map does not get too big
                    CommRunningTime++;
                    if ((getCommLoopSpeed() < 2)&& (CommRunningTime > 4)){
                          // Something stuck
                          CoutM2(ss) << "Comm Loop slow. " << getCommLoopSpeed() << "Hz" << endl;
                      }
              }

              // Monitor the Qs and make sure they don't grow too big or have too old messages
              PurgeOldMessages();

              // Check the keyboard on the front panel of the unit for user interaction
              // Key = frontdisplay.CheckKeyboard();

              // Now check the various communication queues
              int msgInCount = 0;
              while(qMSGin.size() > 0 && msgInCount < 500){
                  MyEventTimer.start();
                  processMSGinput();  // main router called here.
                  if (MyEventTimer.timeinterval() > .1){
                      CoutM2(ss) << "slow PMI" << endl;
                  }
                  msgInCount++;
              }
              if(msgInCount == 500){
                  CoutM1(ss) << "Hit input queue processing limit in CommThread" << endl;
              }

              // Check the serial ports for data in/out
              MyEventTimer.start();
              HandleRS232Comms();
              if (MyEventTimer.timeinterval() > .1)
                   CoutM2(ss) << "slow PMIa" << endl;

              // See if it is time to send out an site identification message
              MyEventTimer.start();
              if ((difftime (now_time ,last_sii ) > SiteIdentifyInterval) && (SiteIdentifyInterval > 0)){
                   //make XML doc to identify us
                   SendSiteIdentifier();
                   last_sii = time(NULL);
              }
              if (MyEventTimer.timeinterval() > .1)
                   CoutM2(ss) << "slow SiteI" << endl;

              // SiteDBinterval
              // See if it is time to send out a table update for the WD device table (we must be a Chief to do this)
              MyEventTimer.start();
              if ((difftime (now_time ,last_wdi ) > SiteDBinterval) &&(Me.IsChief)){
                   //make XML doc to identify us
                   SendWDtableUpdates();
                   last_wdi = time(NULL);
              }
              if (MyEventTimer.timeinterval() > .1)
                   CoutM2(ss) << "slow SiteTI" << endl;


              // See if it is time to send out a table update for the WD device table (we must be a Chief to do this)
              MyEventTimer.start();
              if ((difftime (now_time ,last_gwsync ) > SiteDBinterval) && (Me.gaterole == Primary)){
                   //make XML doc to identify us
                   SyncToBackupGateway();   // update the backup gateway with any new settings
                   last_gwsync = time(NULL);
              }


              if (MyEventTimer.timeinterval() > .1)
                   CoutM2(ss) << "slow SiteTI" << endl;



              MyEventTimer.start();
              SiteManager.ChannelManagement();  // Do the wireless channel management, trunking, TDMA slot allocation...
              if (MyEventTimer.timeinterval() > .1)
                  CoutM2(ss) << "slow SiteCM" << endl;


              // Now sleep a little if we were not terribly busy
              CommSleeper.DozeOff(busy);

              // Output the debug text if there is any
              if (ss.str().size() > 0){
                  MyCLI.OutputText( ss.str() );   // send the text to the console output
                  ss.str("");                     // clear the buffer
              }

        }// while routing and communicating

        // Restart the sockets. Flag it to stop it.
        RunSocketThread = false;
        cout << "Wait for Socket thread to halt." << endl;
        i=0;
        // wait for a while if someone wants us to stop communications.
        if ((SocketThreadRunning) && (i<10) ){
            // wait here while socket thread stops.
            sleep(1);  // wait here for a bit
            i++;
        }
        cout << "Socket thread halted in " << i << " Seconds." << endl;

        CommunicationsRunning = false;
        i=0;
        // wait for a while if someone wants us to stop communications.
        if ((StopCommunications) && (i<10) ){
            // Someone told us to stop for a while and then we'll restart
            sleep(1);  // wait here for a bit
            i++;
            CommunicationsRunning = false;
        }



    }// while not shutting down

    //RunSocketThread = false;   // sockets must stop ASAP/

    frontdisplay.HomeScreen(REDRAWALL);
    ss << "Communications stopped.   thread..." << endl;
    MyCLI.Display(&ss);
    now_time = time(NULL);
    // Wait a little while the sockets close
    while ((now_time >= (time(NULL) - 2)) && (SocketThreadRunning)){

    };

    cout << "Waiting for socket thread to stop.." << endl;
    pthread_join( SocketThread, NULL);   // wait for the socket thread to stop running
    MyCLI.Display(&ss);
    cout << "Socket thread stopped.." << endl;
   //  exit(1);   // done with the thread.  Stop now.

}

// Compute the traffic through our devices.
// Must be called every second
void ComputeTraffic(void){
    int i;
    long n;
    int c;
    long beg;

    // remember bytes IN history
    for (i=0; i< MAXDEVDES; i++){
        if(OurDevices.devicetypes[i] != dNONE){
            OurDevices.TrafficIn[i].push_back(OurDevices.getBytesIn(i));
        }
        if (OurDevices.TrafficIn[i].size() > MaxTrafficQ)
            OurDevices.TrafficIn[i].erase(OurDevices.TrafficIn[i].begin());  // remove old entires
    }
    // remember bytes OUT history
    for (i=0; i< MAXDEVDES; i++){
        if(OurDevices.devicetypes[i] != dNONE){
            OurDevices.TrafficOut[i].push_back(OurDevices.getBytesOut(i));
        }
        if (OurDevices.TrafficOut[i].size() > MaxTrafficQ)
            //OurDevices.TrafficOut[i].pop_back();  // remove old entires
            OurDevices.TrafficOut[i].erase(OurDevices.TrafficOut[i].begin());  // remove old entires
    }

    vector<long>::iterator it;

    // now compute the stats based on the saved data stats
    for (i=0; i< MAXDEVDES; i++){
        if ((OurDevices.devicetypes[i] != dNONE) && (OurDevices.TrafficOut[i].size() > 1)){
          n = 0;  c = 0;
          beg = OurDevices.TrafficOut[i].front();  // get the first entry
          for(it = OurDevices.TrafficOut[i].begin(); it != OurDevices.TrafficOut[i].end(); it++){
              c = c + (*it - beg);
              beg = *it;
              n = n + 1;  // count the number we average
              if (n > 60)
                  break;
          }
          OurDevices.BytesOutThisMin[i] = c;
        }else{
          OurDevices.BytesOutThisMin[i] = 0;
        }
        
        if ((OurDevices.devicetypes[i] != dNONE) && (OurDevices.TrafficIn[i].size() > 1)){
          n = 0;  c = 0;
          beg = OurDevices.TrafficIn[i].front();  // get the first entry
          for(it = OurDevices.TrafficIn[i].begin(); it != OurDevices.TrafficIn[i].end(); it++){
              c = c + (*it - beg);  // add the number of bytes we got in the last second.
              beg = *it;
              n = n + 1;  // count the number we average
              if (n > 60)
                  break;
          }
          OurDevices.BytesInThisMin[i] = c;
        }else{
          OurDevices.BytesInThisMin[i] = 0;
        }
    }
    
};



// Use to add tty serial devices
bool ConnectTTYdevice(int devindex, string intf, int baudrate, string settings){
    int ttyindex;
    
    //cout << "connect ttydevice#" << devindex << "  " << intf << endl;
    
    if ((devindex >= 0) && (devindex < MAXDEVDES)){
        // now bind to a serial port object
        ttyindex = GetSerialIndex(intf);   // Find out the index to the object handling this serial port
        
        if(ttyindex < 0){
            // Port is not a ttySx. See if we can just hijack something else
            // TODO: All port assignments should be dynamic and not assume "ttySx"
            for(int i = MAX_TTY - 1; i >= 0; i--){
                if(COMport[i].myDevIndex == -1){
                    // Hijack this port
                    ttyindex = i;
                    COMport[i].devicename = intf;
                    break;
                }
            }
        }
        
        if ((ttyindex >= 0) && (ttyindex < MAX_TTY)){
            //cout << "DEV.." << intf << endl;
            pthread_mutex_lock(&devlock);
            OurDevices.setBinding(devindex, ttyindex);
            // initialize the serial port
            pthread_mutex_unlock(&devlock);

            pthread_mutex_lock(&ttylock);
            if (settings.size() >= 3){
                COMport[ttyindex].parity = toupper(settings[0]);
                COMport[ttyindex].databits = settings[1] - '0';
                COMport[ttyindex].stopbits = settings[2] - '0';
                
                if(settings.size() >= 4){
                    COMport[ttyindex].flowcontrol = toupper(settings[3]);
                }else{
                    COMport[ttyindex].flowcontrol = 'N';
                }
                
                if(settings.size() >= 5){
                    // Other various setting characters
                    string othersettings = settings.substr(4);
                    COMport[ttyindex].bcl = othersettings.find('B') != string::npos;
                    COMport[ttyindex].invertData = othersettings.find('I') != string::npos;
                }else{
                    COMport[ttyindex].bcl = false;
                    COMport[ttyindex].invertData = false;
                }
            }

            COMport[ttyindex].myDevIndex = devindex;       // remember who is bound to what
            COMport[ttyindex].myDevType = OurDevices.devicetypes[devindex];
            COMport[ttyindex].baudrate = baudrate;         // set the baud rate for this device
            COMport[ttyindex].OpenComPort();
            //InitializeProtocols(COMport[ttyindex].MyParser.protocols);  // load up the protocol list
            COMport[ttyindex].MyParser.ParsingPort = ttyindex;  // set it equal to the tty port number
            COMport[ttyindex].MyParser.DefaultDstID = DEFAULT_ID;
            COMport[ttyindex].MyParser.DefaultSrcID = DEFAULT_ID;
            pthread_mutex_unlock(&ttylock);
           return true;
        }
    }
    return false;
     
}


// Use to associate a device with an interface
// Return the devicedesignator index into the array of device designators.
int AddConnection(std::string dts, std::string intf, std::string devdes){

   // Get the index into our device structure
    string errs = "";

    // See if the device is supported by this verion of code
    int dt = OurDevices.DeviceTypeIndex(dts);
    if (dt < 0)
    {  // Error. This  device is not supported
       return -1;
    }

    // Find and open devicedesignator index to use
    int i;
    for (i=0; i< MAXDEVDES; i++){
        if(OurDevices.devicetypes[i] == dNONE)
            break;
    }

    if (OurDevices.devicetypes[i] == dNONE){
        // We found an unused device object.  Turn it into a device
        pthread_mutex_lock(&devlock);
        OurDevices.designator[i] = devdes; // The DeviceDesignator text for this interface
        OurDevices.devicetypes[i] = dt;    // The type of hardware connected. (index value)
        OurDevices.interfaces[i] = intf;   // the interface we are going to use.
        OurDevices.descriptions[i] = "";   // a text description for this device
        OurDevices.channels[i] = -1;       // we don't use a channel for  this type of device
        pthread_mutex_unlock(&devlock);
        return i;                          // we successfully added it.
    }

  return -1;
}

// ********************************************************************
// Handle taking messages out of the inbound message queue.
// ********************************************************************
void processMSGinput(void){
    BinaryEntry Min;
    double deltaT;
    int i;
    stringstream ss;

    Min = qMSGin.front();   // grab the oldest message

    // See if the ID of this message needs translation to a certain WD ID base on the port it came in on.
    WNAT.AddressTranslate(Min);

    // Did this message come from a WD or Base Radio. If so, it will have a source ID.
    if (Min.srcID != NULL_ID){
        // See if there is a pipe between this interface and some other. If so, pipe it, and negate this Min message
        // so it does not get routed.
        OurDevices.MyPiper.PipeBinary(Min);

        // see if it is in the list of WD. If not, add it.
        if ((dtWD->RowPresent(Min.srcID) == false) && (Min.srcID != NULL_ID) && (dtWD->AutoAddRows == true)){
            if ((Min.bcount > 0) && ((Min.format == fmtWMX) || (Min.format == fmtPRAVE))){
                // Only add certain formats to avoid growing table unnecessarily
                dtWD->AddNewRow(Min.srcID);  // add the row
                CoutM1(ss) << "Added new WD. ID:" << Min.srcID << endl;
            }
        }

        // From a WD. A radio control, site management, trunking, slot assignments...
        if (SiteManager.NewMessageIn(Min) == false){
            // This was NOT a radio control message.  Must be a WD so continue processing it.
            DataRouter.DoRouting(Min);            // Route the message as defined in the routes table.

            // Keep the WD statistics. Update the stats in the dtWD table
            if ((Min.srcID != NULL_ID) && dtWD->RowPresent(Min.srcID)) {
                // Update the WirelessDevice table with the statistics about this message
                dtWD->AddToItem(Min.srcID, fld_countFm, 1);             // Add 1 to the countFm field for this WD with ID FROM
                dtWD->AddToItem(Min.srcID, fld_countFmD, 1);            // Increment the daily from field for this WD
                dtWD->StoreTime(Min.srcID ,fld_tmLastMsg , LocalTimeStamp(YYYY_MM_DD, HH_MM_SS));  // The Zulu time now in YYYY MM DD format
            }
        }

    }
    else{
        // This message did not come from a wireless device. Maybe another gateway? Terminal?
        switch (Min.format){
            case fmtPRAVE:
               break;
            case fmtWMX:
                break;
            case fmtNMEA:
                break;
            case fmtCigorn:
                // inter-site XML from another Cigorn site
                InterSiteMessageIn(Min);
                break;
            case fmtASCII:
                // ASCII. Maybe from a terminal
                break;
            case fmtXML:
                break;
            case fmtESRI_CSV1:
                break;
         }

        // Before we route it...
        // Run and subs specific to the type of device that originated this message
        if ((Min.SrcDevDesIndex >= 0) && (Min.SrcDevDesIndex < MAXDEVDES)){
            switch (OurDevices.devicetypes[Min.SrcDevDesIndex]){
                case dNONE:
                    break;
                case dDataModem:
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
                case dWMXmodem:
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
                case dAVLPC:
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
                case dCigorn :
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
                case dClientPC :
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
                case dMAILserver:
                    break;
                case dWEBserver:
                    break;
                case dTerminal:
                    OurDevices.MyPiper.DataFromTerminal(Min);   // see if this data should be piped somewhere
                    break;
                case dTAP:
                    OurDevices.MyPiper.PipeBinary(Min);
                    break;
            }
        }
        // this message was from something other than a WD
        DataRouter.DoRouting(Min);  // Route the message as defined in the routes table.

    }

    // Keep system statistics on messages in
    CountMessage(Min.format);


    if ((Min.timein > 0) && (Min.timein < 9e20))
        deltaT = TimeNow() - Min.timein;
    else
        deltaT = -1;

    // Save some statistics on the speed this message was processed in.
    pthread_mutex_lock(&dlyvlock);    // lock the delay vector while we manipulate it
    switch (Min.format){
        case fmtPRAVE:
            if (deltaT > 0) dlyPRAVE.insert(dlyPRAVE.begin(),deltaT);  // keep track of the message delay through the gateway
            break;
        case fmtWMX:
            if (deltaT > 0) dlyWMX.insert(dlyWMX.begin(),deltaT);  // keep track of the message delay through the gateway
            break;
        case fmtNMEA:
            break;
        case fmtCigorn:
            // inter-site XML from another Cigorn site
            if (deltaT > 0) dlyCIGORN.insert(dlyCIGORN.begin(),deltaT);  // keep track of the message delay through the gateway
            break;
        case fmtXML:
            break;
        case fmtESRI_CSV1:
            break;
     }
    pthread_mutex_unlock(&dlyvlock);    // unlock the vector after we are done with it.

    // Limit the size of the array we keep the statistics for throughput
    pthread_mutex_lock(&dlyvlock);    // lock the vector while we manipulate it
    if (dlyPRAVE.size() > MAXDLYQSIZE)
        dlyPRAVE.erase(dlyPRAVE.end()-1);
    if (dlyWMX.size() > MAXDLYQSIZE)
        dlyWMX.erase(dlyWMX.end()-1);
    if (dlyCIGORN.size() > MAXDLYQSIZE)
        dlyCIGORN.erase(dlyCIGORN.end()-1);
    pthread_mutex_unlock(&dlyvlock);    // lock the vector while we manipulate it


    // Remove the object from the queue.  Threadlock it while this is done.
    pthread_mutex_lock(&qlock);
    qMSGin.pop();                        // take one NMEA sentence off the top
    pthread_mutex_unlock(&qlock);

    // Output the debug text if there is any
    MyCLI.OutputText( ss.str() );   // send the text to the console output
    ss.str("");                     // clear the buffer

};


// get information about the configuration of the TCP sockets
socketstatus GetSocketStatus(){
    int i;  // gp variables
    int x;
    socketstatus ss;
    ss.count_servers = 0;
    ss.count_connected = 0;
    ss.count_max = 0;
    ss.count_used = 0;

    x=0;
    for (i=0; i<MAXSOCKETS; i++){
       if( tcpsockets[i].sockfd >= 0)
           x++;  // default port numbers
       if( tcpsockets[i].protocol == pServer)
           ss.count_servers++;
     }
    ss.count_used = x;

    x=0;
    for (i=0; i<MAXSOCKETS; i++){
       if( tcpsockets[i].connected == true)
           x++;  // default port numbers
     }
    ss.count_connected = x;
    ss.count_max = MAXSOCKETS;

    return ss;

}

// return the index of the Device that is bound to a specific serial port
int DeviceBoundToTTY(string ttyname){
    // first look for devices that are missing bindings to tty serial ports.
    // COMport[MAX_TTY]
    int devindex;

    for (devindex = 0; devindex < MAXDEVDES; devindex++){

            if (OurDevices.interfaces[devindex] == ttyname) {
                return devindex;

            }// if device==ttyS
     }// for (divindex

    return -1;

}


// Find out what serial ports are on this machine, and load the COMport[MAX_TTY] structure with
// their Linux names
int InitializeComPorts(void){
    int i = 0;
    for (i=0; i<MAX_TTY; i++){
        pthread_mutex_lock(&ttylock);

        // Try to open all the serial ports to see if they exist on the machine
        COMport[i].index = i;
        COMport[i].baudrate = 9600;    // a default baud rate to try to use
        COMport[i].devicename = "ttyS" + intToString(i);
        COMport[i].myDevIndex = -1;
        COMport[i].myDevType = dNONE;
        pthread_mutex_unlock(&ttylock);
    }
    return 0;
}


void HandleRS232Comms(void){
    bool success = false;
    int i = 0;
    WMX wmx;

    // See if there is data comming in the serial port
    for (i=0; i<MAX_TTY; i++){
        if (COMport[i].ForceReset || (COMport[i].ShouldConnect && COMport[i].handle <= 0)){
            COMport[i].ForceReset = false;
            COMport[i].ReOpen();
        }
            
        // Try to open all the serial ports to see if they exist on the machine
        if (COMport[i].handle >= 0 ){
            // This port is open. See if data is comming in.
            if (COMport[i].GetChars() > 0){
                // Some data came in
            }
            
            if(COMport[i].flowcontrol == 'H' && COMport[i].isInputPaused()){
                // If we are input-paused and hardware flow control is on,
                // tell the source not to send anything
                if(COMport[i].isInputPaused()){
                    COMport[i].setIncomingFlowStatus(true);
                    continue;
                }
            }else if(COMport[i].flowcontrol == 'H'){
                // Hardware flow control is on. See if anything we are routing
                // to is getting backed up. If so, we'll try to tell the source
                // of messages (this TTY) to slow it down
                bool stopInboundMessages = false;
                vector<int> destinationDevices;
                DataRouter.getDestinations(OurDevices.designator[COMport[i].myDevIndex], destinationDevices);
                // TODO: This is an insane level of iteration for a simple lookup. We need precompiled routes!
                for(int j = 0; j < destinationDevices.size(); j++){
                    int destinationDeviceIndex = destinationDevices[j];
                    if (OurDevices.IsTTY(destinationDeviceIndex)){
                        // We have to manually find the queue by digging through all the TTYs
                        for(int k = 0; k < MAX_TTY; k++){
                            if(COMport[k].devicename == OurDevices.interfaces[destinationDeviceIndex]){
                                if(COMport[k].MsgQout.size() > 3 * maxQcount / 4){
                                    // We've hit the 3/4 point on the queue. We shouldn't accept any more messages
                                    stopInboundMessages = true;
                                }
                                break;
                            }
                        }
                    }else{
                        // TODO: For now, we only check the outbound queue of TTYs.
                    }
                    if(stopInboundMessages){
                        // No need to look further
                        break;
                    }
                }
                // Set RTS line accordingly
                COMport[i].setIncomingFlowStatus(!stopInboundMessages);
            }
        }
    }

     // See if there is data from the outbound TTY data queue to send out to the serial ports on this site
     int routed = 0; 
     while (qTTYout.size() > 0 && routed < 200){
         // Arbitrary limit on how many we move per call
         routed++;
         
         pthread_mutex_lock(&qlock);
         BinaryEntry TopEntry;
         TopEntry = qTTYout.front();           // get the q object with the data we need to send out
         qTTYout.pop();                        // take one object off the top
         pthread_mutex_unlock(&qlock);
         success = false;   // set true once we find a suitable socket to send this data to.
         for (i = 0; i < MAX_TTY; i++){
              // See if there is data to send out the serial port
              pthread_mutex_lock(&COMport[i].qlock);
              if ((TopEntry.DstDevDesIndex == COMport[i].myDevIndex)
                   && (TopEntry.bcount < MAXBUFSIZE) && (COMport[i].myDevIndex >= 0)){
                   // This is the socket we need to send out the data to...
                   if(COMport[i].MsgQout.size() >= maxQcount){
                       cout << "Queue overflow at " << COMport[i].devicename;
                       pthread_mutex_unlock(&COMport[i].qlock);
                       break;
                   }
                  
                   if (Me.IsActive == false){
                        // We are off-line. Dont send data
                   }else{
                        // Send the data out if we are online(active)
                        COMport[i].MsgQout.push_back(TopEntry);    // put the message in the outbound message q for this TTY port
                        success = true;
                   }

              }
              pthread_mutex_unlock(&COMport[i].qlock);         // pthread_mutex_unlock(&addlistlock)
              if (success)
                  break;
           }
 
         if (success == false)
             FailedTTYOut++;    // count the homeless messages
     }

    
     // Check the serial ports, and if there is room in their data buffers and a message to send, send it
     for (i = 0; i < MAX_TTY; i++){
         pthread_mutex_lock(&COMport[i].qlock);
            if (COMport[i].MsgQout.size() > 0){
                if(COMport[i].bcl){
                   // We've got a message we'd like to send. Before we do so,
                   // we need to check to see if we should delay because someone
                   // is on the air.
                   // After 
                   bool lockout = false;
                   switch(COMport[i].bclState){
                       case WaitForCD:
                           if(COMport[i].CDin()){
                               // Start the busy channel timer
                               COMport[i].busyChannelStartTime = TimeNow();
                               COMport[i].bclState = CDHigh;
                               lockout = true;
                           }else{
                               // Stop the busy channel timer
                               COMport[i].busyChannelStartTime = 0;
                           }
                           break;
                       case CDHigh:
                           if(!COMport[i].CDin() || TimeNow() - COMport[i].busyChannelStartTime > 5){
                               // We've been in full lockout for 4 seconds. Go to cool down
                               COMport[i].bclState = WaitForCD;
                           }else{
                               // CD is high and it hasn't been long enough to override
                               // Skip sending data for now
                               lockout = true;
                           }
                           break;
                   }
                   if(lockout){
                       pthread_mutex_unlock(&COMport[i].qlock);   // pthread_mutex_unlock(&addlistlock)
                       continue;
                   }
              }
              
              if(COMport[i].flowcontrol == 'H' && !COMport[i].CTSin()){
                  // Flow control is preventing sending
                  pthread_mutex_unlock(&COMport[i].qlock);   // pthread_mutex_unlock(&addlistlock)
                  continue;
              }
                
              if(COMport[i].isOutputPaused()){
                  // Pause condition is preventing sending
                  pthread_mutex_unlock(&COMport[i].qlock);   // pthread_mutex_unlock(&addlistlock)
                  continue;
              }
              
              if(COMport[i].queuedBytes() > 0){
                  // Since Cigorn runs _far_ faster than a serial port can,
                  // we only allow the driver to queue a single message.
                  // Without this, it's possible for the OS to queue hundreds
                  // of messages without our knowledge and we lose the ability
                  // to do more than basic flow control
                  pthread_mutex_unlock(&COMport[i].qlock);   // pthread_mutex_unlock(&addlistlock)
                  continue;
              }
              
             // There is a message to send
             BinaryEntry TopEntry;
             TopEntry = COMport[i].MsgQout.front();         // get the top entry in this port's Q
             if(TopEntry.format == fmtPageASCII && COMport[i].myDevType == dWMXmodem){
                 // This optimization allows multiple pages to share
                 // a pre-amble. Since this connection is the source of the
                 // flow-control bottleneck, we have to do this here
                 
                 // Remove from the queue.
                 TTYOut_PopFront(i);
                 
                 // Create the first page
                 PagerTableEntry firstPager = Pagers.GetPager(TopEntry.dstID);

                 if(firstPager.capCode != PagerTable::RESULT_NO_PAGER){
                    // Limit page requests to 1 every half second
                    COMport[i].timeNextPageAllowed = TimeNow() + 0.5;
                     
                    // Create an actual page
                    WMX wmx;
                    string message;
                    TopEntry.data[TopEntry.bcount] = '\0'; // Null terminate for string assignment
                    message.assign(TopEntry.data);
                    syslog(LOG_INFO, "page initmsg %d %s", firstPager.capCode, TopEntry.data);
                    // Hijack the be buffer for big savings!
                    int byteCount = POCSAGEncoder::encode(firstPager, message, TopEntry.data, BinaryEntry::MAXDATA);

                    // Now, continue to remove page messages as long as they match the parameters of the first page
                    while(COMport[i].MsgQout.size() > 0){
                        BinaryEntry nextEntry = COMport[i].MsgQout.front();
                        PagerTableEntry pager = Pagers.GetPager(nextEntry.dstID);
                        if(nextEntry.format == fmtPageASCII && pager.capCode != PagerTable::RESULT_NO_PAGER
                                && pager.otaProtocol == firstPager.otaProtocol){
                            // Everything looks good. Let's try to stitch this pager in
                            nextEntry.data[nextEntry.bcount] = '\0'; // Null terminate for string assignment
                            message.assign(nextEntry.data);
                            bool messageAdded = POCSAGEncoder::append(pager, message, TopEntry.data, &byteCount, 480);//208
                            if(messageAdded){
                                syslog(LOG_INFO, "page stitchmsg %d %s", pager.capCode, nextEntry.data);
                                // This page was successfully stitched on. Remove it
                                TTYOut_PopFront(i);
                            }else{
                                // We must be getting too long. Leave the message in the queue and stop trying
                                break;
                            }
                        }else{
                            // Encountered a message that can't be added. Have to stop
                            break;
                        }
                    }
                    
                    if(COMport[i].invertData){
                        POCSAGEncoder::invert(TopEntry.data, byteCount);
                    }
                    
                    // Stuff all the pages we got into WMX
                    BinaryEntry pageEntry;
                    wmx.BuildWMX(TopEntry.data, byteCount, TopEntry.srcID, TopEntry.dstID, WMX_FR_RAW);

                    if ((wmx.size < BinaryEntry::MAXDATA) && (wmx.size > 0) ) {
                        memcpy ( pageEntry.data, wmx.frame, wmx.size );
                        pageEntry.format = fmtWMXPOCSAG;  // set the format flag
                        pageEntry.bcount = wmx.size;

                        pageEntry.srcID = wmx.source;
                        pageEntry.dstID = wmx.destination;
                        pageEntry.timein = TimeNow();

                        pageEntry.SrcDevDesIndex = TopEntry.SrcDevDesIndex;
                        pageEntry.DstDevDesIndex = TopEntry.DstDevDesIndex;
                        pageEntry.PortIn = TopEntry.PortIn;
                        
                        COMport[i].MsgQout.push_front(pageEntry);
                    }
                    
                    // In front of the page itself, we need to set the modem to the right baud
                    WMX baudWMX;
                    bool sendBaudChange = false;
                    if(firstPager.otaProtocol == "POCSAG512"){
                        baudWMX.BuildWMX("ATR2 12", 7, 0, 0, WMX_FR_COMMAND);
                        sendBaudChange = true;
                    }else if(firstPager.otaProtocol == "POCSAG1200"){
                        baudWMX.BuildWMX("ATR2 1", 6, 0, 0, WMX_FR_COMMAND);
                        sendBaudChange = true;
                    }else if(firstPager.otaProtocol == "POCSAG2400"){
                        baudWMX.BuildWMX("ATR2 2", 6, 0, 0, WMX_FR_COMMAND);
                        sendBaudChange = true;
                    }
                    if(sendBaudChange){
                        BinaryEntry baudChangeEntry;

                        if ((baudWMX.size < baudChangeEntry.MAXDATA) && (baudWMX.size > 0) ) {
                            memcpy ( baudChangeEntry.data, baudWMX.frame, baudWMX.size );
                            baudChangeEntry.format = fmtWMX;  // set the format flag
                            baudChangeEntry.bcount = baudWMX.size;

                            baudChangeEntry.srcID = baudWMX.source;
                            baudChangeEntry.dstID = baudWMX.destination;
                            baudChangeEntry.timein = TimeNow();

                            baudChangeEntry.SrcDevDesIndex = TopEntry.SrcDevDesIndex;
                            baudChangeEntry.PortIn = TopEntry.PortIn;

                            //COMport[i].MsgQout.push_front(baudChangeEntry);
                        }
                    }
                }
             }else if ((TopEntry.bcount < MAXBUFSIZE)&& (TopEntry.bcount > 0)){
                // There is some data to send
                if (Me.IsActive == true){
                    COMport[i].SendBytes(TopEntry.data, TopEntry.bcount);  // store the topentry in the data buffer
                    if(TopEntry.format == fmtWMXPOCSAG){
                        syslog(LOG_INFO, "page transmit");
                    }
                }
                // Remove the message from the queue
                TTYOut_PopFront(i);
             }else{
               cout << "Invalid data byte count";
               TTYOut_PopFront(i);
             }
          }
          pthread_mutex_unlock(&COMport[i].qlock);   // pthread_mutex_unlock(&addlistlock)
       }
    
};

int GetSerialIndex(string interfacename){
    int i;

    for (i=0; i < MAX_TTY; i++ ){
        if (COMport[i].devicename == interfacename)
            return i;
    }
    return -1;
}

void SyncToBackupGateway(void){

}

// send periodic updates of the WD table to all other gateways so they have a list of who is allowed on the network.
void SendWDtableUpdates (void){

    string s;
    static int idA = 0;  // the IDs used to keep track of what WDs have been included in the site identifier.
    static int idB = 99;
    static int lastrow = 0;
    int x = 0;
    int devindex;
    BinaryEntry be;

    idA = lastrow;
    idB = idA;
    
    // Find the beginning ID
    for (dtWD->dit = dtWD->rows.begin(); dtWD->dit != dtWD->rows.end(); dtWD->dit++){
       if (dtWD->GetIntItem(dtWD->dit->first, fld_ID) >= idA){
           x++;  // count this entry
           idB =dtWD->GetIntItem(dtWD->dit->first, fld_ID);
           lastrow = idB;                 // remember the last row we sent the update for.
       }
       if (x >= RowsPerDBupdate)
           break;  // we have enough
    }

    if (x < RowsPerDBupdate){
       // there were not enough rows to make a complete update. rememeber we hit the end of the ID list
       lastrow = 0;         // start over at ID 0 next time around.
       idB = MAXWDID;
    }


   try{
      s = MakeWDtableUpdateXML(idA, idB);   // make our table update XML
    }
    catch( ...){
        s = "";
    }


    if (s.size() > 0){
        for (devindex = 0; devindex < MAXDEVDES; devindex++){
            if (OurDevices.devicetypes[devindex] == dCigorn) {
                // This device is another cigorn gateway.  Send it the updates.
                if (s.size() < be.MAXDATA){
                    to_cstring (be.data, s, be.MAXDATA);     // convert the XML to cstring
                    be.bcount = s.size();
                    be.SrcDevDesIndex = -1;                  // the device this data is intended to be sent to
                    be.DstDevDesIndex = -1;
                    be.dstID = 0;
                    be.format = fmtXML;
                    be.srcID = 0;
                    be.timein = TimeNow();
                    be.PortIn = -1;                       // The port this message originates from
                    DataRouter.RouteBinTo(be);            // send the xml doc to the device(s) defined in the router table.
                    Cigorncount_out++;
                }
            }// if device==ttyS
         }// for (divindex
    } // s.size>0

}

// Send a site identifier to any device designator communicating to other gateways
void SendSiteIdentifier(void){

    string s;
    stringstream ss;

    int devindex;
    BinaryEntry be;

    for (devindex = 0; devindex < MAXDEVDES; devindex++){
        if (OurDevices.devicetypes[devindex] == dCigorn) {
            // This device is another cigorn gateway.  Send it our information
            s = MakeSiteIdentifierXML();         // make our identification
            if (s.size() < be.MAXDATA){
                to_cstring (be.data, s, be.MAXDATA);  // convert the XML to cstring
                be.bcount = s.size();
                be.SrcDevDesIndex = -1;
                be.DstDevDesIndex = devindex;
                be.dstID = 0;
                be.format = fmtXML;
                be.srcID = 0;
                be.timein = TimeNow();
                DataRouter.RouteBinTo(be);              // send the xml doc to the device(s) defined in the router table.
                Cigorncount_out++;
                CoutM2(ss) << "Sent Site Identifier #" << Cigorncount_out << " via " << OurDevices.getDevDes(devindex) << endl;
                MyCLI.Display(&ss);
            }
        }// if device==ttyS
     }// for (divindex

     MyCLI.Display(&ss);

}



// Send the XML to all Cigorn gateways on the system
void SendCigornXML(string xmltext){

    string s;
    stringstream ss;

    int devindex;
    BinaryEntry be;

    for (devindex = 0; devindex < MAXDEVDES; devindex++){
        if (OurDevices.devicetypes[devindex] == dCigorn) {
            // This device is another cigorn gateway.  Send it our information
            s = xmltext;         //
            if (s.size() < be.MAXDATA){
                to_cstring (be.data, s, be.MAXDATA);  // convert the XML to cstring
                be.bcount = s.size();
                be.SrcDevDesIndex = -1;
                be.DstDevDesIndex = devindex;
                be.dstID = 0;
                be.format = fmtXML;
                be.srcID = 0;
                be.timein = TimeNow();
                DataRouter.RouteBinTo(be);              // send the xml doc to the device(s) defined in the router table.
                Cigorncount_out++;
                CoutM2(ss) << "Sent Cigorn Intersite #" << Cigorncount_out << " via " << OurDevices.getDevDes(devindex) << endl;
                MyCLI.Display(&ss);     // send the text to the console output
            }
        }// if device==ttyS
     }// for (divindex

    MyCLI.Display(&ss);   // send the text to the console output
 
}

// Periodically purge old messages in the outbound Qs that are stuck there because
// the connection is down or some other communication problem.
void PurgeOldMessages(void){
    int wdc = 0;   // watch dog counter
    double deltaT=0;
    static int idx = 0;
    static int j = 0;
    static int counter = 0;
    int loop = 0;
    static BinaryEntry be;

    stringstream ss;

    counter++;
    while (loop < 20){
        // check 20 sockets at a time
        loop++;
        // do one socket per call of this sub
        if (idx >= MAXSOCKETS)
            idx = 0;

        //limit the number of entries in the Q;
        while ((tcpsockets[idx].MsgQout.size() >= (maxQcount - 1)) && (wdc < MAX_WDC_PURGE) && (tcpsockets[idx].MsgQout.size() > 0)){
            // Time to get rid of an old message
            pthread_mutex_lock(&tcpsockets[idx].qlock);   // lock the q
            CoutM1(ss) << "Q overflow in " << tcpsockets[idx].description << endl;
            try {
                tcpsockets[idx].MsgQout.pop(); // remove the message. It is too old
             }
                catch (exception& e) {
             }
            pthread_mutex_unlock(&tcpsockets[idx].qlock);   // lock the q
            wdc++;
        }



        // Make sure the entries are not getting too old
        if (tcpsockets[idx].MsgQout.size() > 0){
            // Now lock it and check again.  Faster than always locking it. 
            pthread_mutex_lock(&tcpsockets[idx].qlock);   // lock the q
    
            if (tcpsockets[idx].MsgQout.size() > 0){
                be = tcpsockets[idx].MsgQout.front();         // get the oldest entry
                if ((be.timein > 0) && (be.timein < 9e20)){
                    deltaT = TimeNow() - be.timein;
                }else{
                    deltaT = -1;
                }
                if (deltaT > maxQage){
                     CoutM2(ss) << "Old message in Q " << tcpsockets[idx].description << " (" << deltaT << ")  deleting it." << endl;
                    try {
                         tcpsockets[idx].MsgQout.pop();   // remove the message. It is too old
                     }
                        catch (exception& e) {
                     }
                }
            }
            pthread_mutex_unlock(&tcpsockets[idx].qlock);   // unlock the q
        }
        idx++;  // next socket next time
     }

      if (j >= MAX_TTY)
         j = 0;

      // Now look at the serial port (tty) queues and see if any messages are way to old
      //limit the number of entries in the Q
      int oldSize = COMport[j].MsgQout.size();
      while ((COMport[j].MsgQout.size() > maxQcount) && (wdc < MAX_WDC_PURGE)){
         pthread_mutex_lock(&COMport[j].qlock);
         TTYOut_PopFront(j);
         pthread_mutex_unlock(&COMport[j].qlock);     // pthread_mutex_unlock(&addlistlock)
         wdc++;
      }

      // Look at the first entry and see how old it is.
      pthread_mutex_lock(&COMport[j].qlock);
      if (COMport[j].MsgQout.size() > 0){
         // Make sure the entries are not getting too old
         BinaryEntry be;
         be = COMport[j].MsgQout.front();  // get the oldest entry
         if ((be.timein > 0) && (be.timein < 9e20))
            deltaT = TimeNow() - be.timein;
         else
            deltaT = -1;
         if (deltaT > maxQage){
             // Remove the message. It is too old
             TTYOut_PopFront(j);
        }
      }
      int newSize = COMport[j].MsgQout.size();
      pthread_mutex_unlock(&COMport[j].qlock);         // pthread_mutex_unlock(&addlistlock)
      if(newSize != oldSize){
         CoutM1(ss)<<"Purged messages from " << COMport[j].devicename 
                 << ". Queue reduced from " << oldSize
                 << " to " << newSize << endl;
      }
      j++;

      MyCLI.OutputText(ss.str());   // send the text to the console output
   
}

void startsocketthread(void) {
   int iret1;

   // remember the port numebr this socket is using.

   // Create a thread for this instance of the classs to do socket communications.
   pthread_t SocketThread;

   // Fill in the sockets structure so the socket knows its port to use, and where to store statistics.
   threadinfo.valid = true;  // this structure is unused now. Maybe some day we'll need to pass info the the thread using it.
   iret1 = pthread_create(&SocketThread, NULL, threadTCPserver, (void*) &threadinfo);

}

// Return the number of sockets we are using
int SocketCount(void){
    int c = 0;
    int i;

    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].myDevDesIndex >= 0)
              c++;        //
    }
    return c;
}

// Return the number of sockets we are using and are connected
int ConnectedSocketCount(void){
    int c = 0;
    int i;

    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].connected == true)
              c++;        //
        }
    return c;
}

// Return the number of sockets we are using and are connected
long SocketByteCount(void){
    long c = 0;
    int i;

    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].myDevDesIndex >= 0)
              c = c + tcpsockets[i].bytes_out;        //
        }
    return c;
}

/**
 * Removes the front entry from the output queue for a TTY device
 * @param index Index of the TTY device
 */
void TTYOut_PopFront(int index){
    try
    {
        if(COMport[index].MsgQout.size() > 0){
            COMport[index].MsgQout.pop_front();                  // remove the top entry. Has thrown an exception at times.
        }
    }
    catch(exception& e)
    {
        elog.store(string("Error MsgQout.pop exception"));
    }
}

/**
 * Resets all serial ports associated with a device designator.
 * @param ResetDevDes Wildcard-enabled device designator to reset
 * @return True if any serial connections were reset
 */
bool ResetSerial(string ResetDevDes){
    int i;
    bool retval = false;
    
    ResetDevDes = StringToUpper(ResetDevDes);
    for (i=0; i < MAX_TTY; i++){
        int deviceIndex = COMport[i].myDevIndex;
        if(deviceIndex >= 0 && deviceIndex < MAXDEVDES){
            if (WildCardMatch(ResetDevDes, StringToUpper(OurDevices.designator[deviceIndex]))){
               COMport[i].ForceReset = true;
               retval = true;
            }
        }
    }
    return 0;
}

/**
 * Gets the current measured comm loop speed, in Hz
 * @return Comm loop's speed, in Hz
 */
int getCommLoopSpeed(){
    return CommSleeper.getLoopSpeed();
}
