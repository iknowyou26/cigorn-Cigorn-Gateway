/* 
 * File:   DeviceType.h
 * Author: john
 *
 * Created on August 27, 2010, 11:04 PM
 */

#ifndef _DEVICETYPE_H
#define	_DEVICETYPE_H

#include <string>
#include <sstream>
#include <queue>
#include <vector>
#include "piper.h"
#include "TCPsocket.h"
#include "datatable.h"

// dTypes are the device types. These are devices connected to a port on the gateway.
#define dNONE              0   // No device
#define dDataModem         1   // RV-M7, without GPS
#define dCLI               2   // Command-line interface to human
#define dCigorn            3   // Another Cigorn gateway
#define dAVLPC             4   // A computer used only for AVL
#define dClientPC          5   // A computer for communicating to the RaveonNet network.
#define dINI               6   // .ini file parser
#define dStatDisplay       7   // The gateway's front panel status display
#define dMAILserver        8   // this is talking to an SMTP mail server
#define dWEBserver         9   // Our web server interface
#define dTerminal          10  // and ascii terminal
#define dWMXmodem          11  // Data Modem with WMX protocol
#define dArcGIS            12  // ESRI ArcGIS server (for now, we only talk CSV to these. there are many formats)
#define dTAP               13  // TAP paging input
#define TOTALdTYPES        14  // Always 1 bigger than the last dType

#ifndef MAXDEVDES
  #define MAXDEVDES  100   // The maximum bumber of devices we will communicate with
#endif
using namespace std;


#define  MaxTrafficQ  60

#define Health_CONN      "Connected"
#define Health_NOCONN  "Disconnected"
#define Health_INQUIET   "No input data"
#define Health_OUTQUIET   "No output data"
#define Health_INOUTQUIET "No data"
#define Health_INPAUSED "Input paused"
#define Health_OUTPAUSED "Output paused"
#define Health_INOUTPAUSED "Paused"
#define Health_SEPARATOR ", "

class DeviceList{

public:
    DeviceList();
    DeviceList(const DeviceList& orig);
    virtual ~DeviceList();
    std::string DeviceName(int);
    int  IndexOf(std::string);
    void Clear(void);
    void ClearEth(void);
    void ClearTty(void);
    bool rfportexists(int );
    bool InterfaceIsFree(std::string, int );
    bool InterfaceIsFree(std::string );
    bool IsEth(int);
    bool IsTTY(int);
    bool IsTTY(std::string);
    int  DeviceTypeIndex(std::string );
    bool IsDesignator(string);
    int AddRadioChannel(string, int , string  , string , int, string);  // use to add ethernet devices
    int AddttyDevice(string  , string, string);                         // use to add RS232 devices
    int ConnectDeviceSocket(int, string , int , string, string, int, bool );
    int getChannelCount(void);
    int getActiveChannelCount(void);
    int getSocketCount(void);
    int getConnectedSocketCount(void);
    int getValidSocketCount(void);
    int DevDesSocketIndex(int );
    int GetNewTCPindex(void);
    int getBaudRate(int);
    long getBytesIn(int);
    long getMessagesIn(void);
    long getMessagesOut(void);
    long getBytesIn(void);
    long getBytesOut(int);
    long getBytesOut(void);
    long getTtyBytesOut(void);
    long getEthBytesOut(void);
    long getTtyBytesIn(void);
    long getEthBytesIn(void);
    string getDDIPaddress(int);
    string getInterface(int);
    int getDevTypeIndex(int);
    string getHealth(int);
    string getBoundDesignator(int, string );
    int getBinding(int);
    int getBindCount(int);
    bool setBinding(int DevDesIndex, int InterfaceIndex);
    string getDevDes(int);
    string getSourceIPaddress(int);
    int getPortNum(int);          // get the PORT number this device designator used
    bool  LoadEthDevDesTable(datatable* );
    bool  LoadTtyDevDesTable(datatable* );
    double HoursSinceTtyMsg(void);
    double HoursSinceEthMsg(void);
    void ResetStatistics(void);

    int index;                    // the defined index for this device. Never change this once defined.
   
    // The arrays that actually hold the device information
    int devicetypes[MAXDEVDES];           // index of the type of hardware device as defined above in dXXXX
    std::string designator[MAXDEVDES];    // The device designator for this RFPx, NAPxx, ...
    std::string descriptions[MAXDEVDES];  // text description of this Device Designator
    std::string interfaces[MAXDEVDES];    // the linux interface we bind to (tty0, tty1, eth0 ...)
    int channels[MAXDEVDES];              // The channel number assigned to this device. For radios, it is the channel number we use for it
    vector<int> bindings[MAXDEVDES];      // The TCP socket or other driver we are bound to
    long BytesInThisMin[MAXDEVDES];
    long BytesOutThisMin[MAXDEVDES];

    // Some statistics
    vector<long>  TrafficIn[MAXDEVDES];   // Number of bytes in the last 60 seconds, each second.
    vector<long>  TrafficOut[MAXDEVDES];  // Number of bytes in the last 60 seconds, each second.

    piper MyPiper;
    int ErrorsLoading;
    int LoadCount;

private:


    // Table of text strings to describe the device types
    std::string typenames[TOTALdTYPES];

};

#endif	/* _DEVICETYPE_H */

