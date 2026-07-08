/* 
 * File:  DeviceList.cpp
 * Author: john
 * 
 * Created on August 27, 2010, 11:04 PM
 */
#include "GlobalVar.h"
#include "Cigorn.h"
#include "functions.h"
#include "DeviceList.h"
#include "CommandLine.h"
#include "StatusDisplay.h"
#include "serialhandler.h"
#include "Matrix192x64.h"
#include "TCPsocket.h"
#include "CommThread.h"
#include "piper.h"
#include "SocketThread.h"
#include "network.h"

// Initializer for our strucure of device types.
DeviceList::DeviceList() {
    int i;
    for (i=0; i< MAXDEVDES; i++){
        interfaces[i] = -1;   // no bindings to any interfaces yet
        descriptions[i] = ""; // no devices defined yet
        channels[i] = -1;     // no channel number assigned to this device.
        devicetypes[i] = dNONE;
        bindings[i].clear();
        designator[i] = "";   // RFPxx, NAPxx,....
        BytesInThisMin[i] = 0;
        BytesOutThisMin[i] = 0;
    }
    // The text version of our various supported device types.
    typenames[0] = "dNONE";
    typenames[1] = "dDataModem";
    typenames[2] = "dCLI";
    typenames[3] = "dCigorn";
    typenames[4] = "dAVLPC";
    typenames[5] = "dClientPC";
    typenames[6] = "dINI";
    typenames[7] = "dStatDisplay";
    typenames[8] = "dWEBserver";
    typenames[9] = "dMailServer";
    typenames[10] = "dTerminal";
    typenames[11] = "dWMXmodem";
    typenames[12] = "dArcGIS";
    typenames[13] = "dTAP";
    ErrorsLoading = 0;
    LoadCount = 0;

}

DeviceList::DeviceList(const DeviceList& orig) {
}

DeviceList::~DeviceList() {
}

// Erase all device designators
void DeviceList::Clear(void){
    int i;
    for (i=0; i< MAXDEVDES; i++){
        interfaces[i] = -1;     // no bindings to any interfaces yet
        descriptions[i] = "";   // no devices defined yet
        channels[i] = -1;       // no channel number assigned to this device.
        devicetypes[i] = dNONE;
        bindings[i].clear();
        designator[i] = "";     // RFPxx, NAPxx,....
        BytesInThisMin[i] = 0;
        BytesOutThisMin[i] = 0;
    }

};

// Erase all Eth device designators
void DeviceList::ClearEth(void){
    int i;
    for (i=0; i< MAXDEVDES; i++){
        if (StringLeft(interfaces[i],3) == "eth"){
            interfaces[i] = -1;     // no bindings to any interfaces yet
            descriptions[i] = "";   // no devices defined yet
            channels[i] = -1;       // no channel number assigned to this device.
            devicetypes[i] = dNONE;
            bindings[i].clear();
            designator[i] = "";     // RFPxx, NAPxx,....
            BytesInThisMin[i] = 0;
            BytesOutThisMin[i] = 0;
        }
    }
};

// Erase all TTY device designators
void DeviceList::ClearTty(void){
    int i;
    for (i=0; i< MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
            interfaces[i] = -1;     // no bindings to any interfaces yet
            descriptions[i] = "";   // no devices defined yet
            channels[i] = -1;       // no channel number assigned to this device.
            devicetypes[i] = dNONE;
            bindings[i].clear();
            designator[i] = "";     // RFPxx, NAPxx,....
            BytesInThisMin[i] = 0;
            BytesOutThisMin[i] = 0;
        }
    }
};



// Return the device name give the device type.
std::string DeviceList::DeviceName(int i){
    if (i < 0) return "Invalid";
    if (i >= TOTALdTYPES ) return "Invalid";
    return typenames[i];

};

/**
 * Returns a human-readable health text for the given device designator
 * @param i Device designator
 * @return Short, human-readable health information about the given device designator
 */
string DeviceList::getHealth(int i){
    bool connected;
    bool inputPaused;
    bool outputPaused;
    bool recentInData;
    bool recentOutData;
    
    stringstream health;

    if ((i< 0) || (i>MAXDEVDES))
        return "";

    int binding = OurDevices.getBinding(i);

    if (IsTTY(interfaces[i]) && binding >= 0 && binding < MAX_TTY){
        connected = COMport[binding].DSRin();
        inputPaused = COMport[binding].isInputPaused();
        outputPaused = COMport[binding].isOutputPaused();
    }else if(binding >=0 && binding < MAXSOCKETS){
        connected = tcpsockets[binding].connected;
        inputPaused = tcpsockets[binding].isInputPaused();
        outputPaused = tcpsockets[binding].isOutputPaused();
    }else{
        connected = false;
    }
    
    recentInData = OurDevices.BytesInThisMin[i] > 0;
    recentOutData = OurDevices.BytesOutThisMin[i] > 0;
    
    if(!connected){
        health << Health_NOCONN;
    }else{
        health << Health_CONN;
    }
    
    if(inputPaused && outputPaused){
        health << Health_SEPARATOR << Health_INOUTPAUSED;
    }else{
        if(inputPaused){
            health << Health_SEPARATOR << Health_INPAUSED;
        }
        
        if(outputPaused){
            health << Health_SEPARATOR << Health_OUTPAUSED;
        }
        
        if(!recentInData && !recentOutData){
            health << Health_SEPARATOR << Health_INOUTQUIET;
        }else{
            if(!recentInData && !inputPaused){
                health << Health_SEPARATOR << Health_INQUIET;
            }

            if(!recentOutData && !outputPaused){
                health << Health_SEPARATOR << Health_OUTQUIET;
            }
        }
    }
    
    return health.str();
}

// return the number of bytes we got in from this device
long DeviceList::getBytesIn(int i){

    if ((i< 0) || (i>MAXDEVDES))
        return 0;

    if (IsTTY(interfaces[i])){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
          return COMport[OurDevices.getBinding(i)].bytes_in;
    }

    if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
          return tcpsockets[OurDevices.getBinding(i)].bytes_in;
    }

    return 0;

}


// return the number of bytes we got in from all devices
long DeviceList::getMessagesOut(void){
    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
              count = count + COMport[OurDevices.getBinding(i)].msg_out;
        }

        if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
              count = count + tcpsockets[OurDevices.getBinding(i)].msg_out;
        }
    }
    return count;
}



// return the number of bytes we got in from all devices
long DeviceList::getMessagesIn(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
              count = count + COMport[OurDevices.getBinding(i)].msg_in;
        }

        if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
              count = count + tcpsockets[OurDevices.getBinding(i)].msg_in;
        }
    }

    return count;

}



// return the number of bytes we got in from all devices
long DeviceList::getBytesIn(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
              count = count + COMport[OurDevices.getBinding(i)].bytes_in;
        }

        if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
              count = count + tcpsockets[OurDevices.getBinding(i)].bytes_in;
        }
    }

    return count;

}


// return the number of bytes we got in from all devices
long DeviceList::getEthBytesIn(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){

        if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
              count = count + tcpsockets[OurDevices.getBinding(i)].bytes_in;
        }
    }

    return count;

}


// return the IP address in string form for the given device des index
string DeviceList::getDDIPaddress(int dd){
    string ipad;
    in_addr_t ip = 0;
    in_addr_t mask = 0;

    if ((dd > 0) && (dd < MAXDEVDES)){
        if (StringLeft(OurDevices.interfaces[dd],3) == "eth"){
            ipad = GetIP(OurDevices.interfaces[dd].c_str(), &ip, &mask);
            return ipad;
        }
    }
    return "";
}



// return the number of bytes we got in from all devices
long DeviceList::getTtyBytesIn(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
              count = count + COMport[OurDevices.getBinding(i)].bytes_in;
        }
    }

    return count;

}


// return the number of bytes we got in from this device
long DeviceList::getTtyBytesOut(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
              count = count + COMport[OurDevices.getBinding(i)].bytes_out;
        }
    }
    return count;

}


// return the number of bytes we got in from this device
long DeviceList::getEthBytesOut(void){

    int i=0;
    long count = 0;

    for (i=0; i < MAXDEVDES; i++){

        if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
              count = count + tcpsockets[OurDevices.getBinding(i)].bytes_out;
        }
    }

    return count;

}


// Find the first socket index of the socket assigned to device designator dd
int DeviceList::DevDesSocketIndex(int dd){

    int sockindex = -1;
    if ((dd < 0) || (dd > MAXDEVDES))
        return -1;


    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
        if (tcpsockets[sockindex].myDevDesIndex == dd){
            return sockindex;
        }
    }
    return -1;

}


// 
double DeviceList::HoursSinceTtyMsg(void){

    int i=0;
    long count = 0;
    bool found = false;
    double d = 0;
    double ret_val = -1;
    time_t tt;

    for (i=0; i < MAXDEVDES; i++){
        if (IsTTY(interfaces[i])){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
               if (found == false){
                   tt = COMport[OurDevices.getBinding(i)].time_last_msg;
                   ret_val = difftime(time(NULL), tt) / (60 * 60 * 24);
                   found = true;
               }else{
                   tt = COMport[OurDevices.getBinding(i)].time_last_msg;
                   d = difftime(time(NULL), tt) / (60 * 60 * 24);
                   if (ret_val > d)
                       ret_val = d;
               }
        }

    }

    if (found){
       return ret_val;
    }
    else
        return -1;

}

// Return the number of seconds since the last activity on any socket.
double DeviceList::HoursSinceEthMsg(void){

    int i=0;
    long count = 0;
    bool found = false;
    double d = 0;
    double ret_val = -1;
    time_t tt;

    for (i=0; i < MAXDEVDES; i++){
        if (StringLeft(interfaces[i],3) == "eth"){
           if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
               if (found == false){
                   tt = tcpsockets[OurDevices.getBinding(i)].time_last_in;
                   ret_val = difftime(time(NULL), tt) / (60 * 60 * 24);
                   found = true;
               }else{
                   tt = tcpsockets[OurDevices.getBinding(i)].time_last_in;
                   d = difftime(time(NULL), tt) / (60 * 60 * 24);
                   if (ret_val > d)
                       ret_val = d;
               }
        }

    }

    if (found){
       return ret_val;
    }
    else
        return -1;

}

// return the number of bytes we got in from this device
long DeviceList::getBytesOut(int i){

    if ((i< 0) || (i>MAXDEVDES))
        return 0;

    if (IsTTY(interfaces[i])){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
          return COMport[OurDevices.getBinding(i)].bytes_out;
    }

    if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
          return tcpsockets[OurDevices.getBinding(i)].bytes_out;
    }

    return 0;

}

// return the binding value for this DeviceDesignator index
bool DeviceList::setBinding(int DevDesIndex, int InterfaceIndex){

    bindings[DevDesIndex].insert(bindings[DevDesIndex].begin(), InterfaceIndex );;
    return true;

}


// return the binding value for this DeviceDesignator index
int DeviceList::getBindCount(int DevDesIndex){

    if ((DevDesIndex < 0) || (DevDesIndex > MAXDEVDES))
        return -1;

    return bindings[DevDesIndex].size();

}


// return the lowest binding value for this DeviceDesignator index
int DeviceList::getBinding(int i){
    int j;
    int RetVal = -1;

    if (bindings[i].size() == 0)
        return 0;  // Very bad error

    if (bindings[i].size() == 1)
       return bindings[i].at(0);
    else{
        RetVal = bindings[i].at(0);
        for (j=0; j<bindings[i].size(); j++){
            if (bindings[i].at(j) < RetVal)
                RetVal = bindings[i].at(j);
        }
    }
    return RetVal;

}

// return the TCP port number used for this devicedesignator. Either Ethernet port to TTYS serial port number
int DeviceList::getPortNum(int i){

    if ((i< 0) || (i>MAXDEVDES))
        return 0;

    if (IsTTY(interfaces[i])){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
          return COMport[OurDevices.getBinding(i)].bytes_out;
    }

    if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
       if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS))
          return tcpsockets[OurDevices.getBinding(i)].portnum;
    }

    return 0;

}

// return true if this devDesignator is an ethernet interface
bool DeviceList::IsEth(int i){

    if ((i< 0) || (i>MAXDEVDES))
        return false;

    if (StringLeft(OurDevices.interfaces[i],3) == "eth")
        return true;

    return false;

}
bool DeviceList::LoadEthDevDesTable(EthDeviceTableAdapter* adapter)
{
    if (adapter == NULL)
        return false;

    if (!adapter->Load())
        return false;

    int  DevIndex = -1;
    int  rfp = -1;
    bool success = false;
    int didit = true;
    string dtype = "";
    string devdes = "";
    string intf = "";
    string ipadd = "";
    string protocol = "";
    string parm1 = "";
    bool tcp_keepalive = false;
    int ClientTimeout = 0;
    int pnum, i, dtypeindex;
    stringstream ss;
    string emessage = "";
    int pcount, port;

    ErrorsLoading = 0;
    LoadCount = 0;

    for (i = 0; i < adapter->RowCount(); i++)
    {
        success = false;
        didit = true;

        devdes = adapter->GetString(i, 0);
        if (devdes.size() > 0)
        {
            intf = adapter->GetString(i, 2);
            dtype = adapter->GetString(i, 1);
            dtypeindex = DeviceTypeIndex(dtype);

            if ((dtypeindex == dDataModem) || (dtypeindex == dWMXmodem))
            {
                rfp = adapter->GetInt(i, 3);
                DevIndex = AddRadioChannel(devdes, rfp, dtype, intf, pnum, protocol);
                if (DevIndex == -1)
                {
                    emessage = "Error: RF Channel number " + intToString(rfp) + " already assigned to" + devdes;
                    elog.store(emessage);
                    CoutM2(ss) << emessage << endl;
                }
            }
            else
            {
                DevIndex = AddConnection(dtype, intf, devdes);
            }

            if (DevIndex >= 0)
            {
                pnum = adapter->GetInt(i, 3);
		ipadd = adapter->GetString(i, 4);
		protocol = adapter->GetString(i, 5);
		parm1 = adapter->GetString(i, 6);
		OurDevices.descriptions[DevIndex] = adapter->GetString(i, 8);
		pcount = adapter->GetInt(i, 9);

                if (pcount < 1)
                    pcount = 1;

                port = pnum;

                if (parm1.size() > 0)
                {
                    ClientTimeout = StringToInt(parm1);
                    tcp_keepalive = (StringToInt(parm1) < 0);
                }
                else
                {
                    ClientTimeout = 0;
                    tcp_keepalive = false;
                }

                while ((pcount > 0) && didit)
                {
                    if (InterfaceExists(intf))
                    {
                        didit = ConnectDeviceSocket(DevIndex, intf, port, protocol, ipadd, ClientTimeout, tcp_keepalive);

                        if (didit != true)
                        {
                            CoutM2(ss) << "Error 392. Failed to create socket for:" << devdes
                                       << " port " << port << " on " << intf << " EC=" << didit << endl;
                            elog.store(string("Error 392. Failed to create socket for:" + devdes + " Port:" + intToString(port)));
                            ErrorsLoading++;
                        }
                        else
                        {
                            CoutM2(ss) << "New eth DeviceDesignator socket:" << devdes << " " << intf << " port:" << port << endl;
                            LoadCount++;
                        }
                    }
                    else
                    {
                        CoutM2(ss) << "Error 393. Invalid eth interface for device designator:" << devdes << " on " << intf << endl;
                        elog.store(string("Error 393. Invalid eth interface for device designator:" + devdes));
                        ErrorsLoading++;
                    }

                    pcount--;
                    port++;
                }

                success = true;
            }
        }

        if (!success)
        {
            ss << "Failed to add New DeviceDesignator " << devdes << ". " << emessage << endl;
            elog.store(string("Failed to add devicedesignator:" + devdes));
        }
    }

    if (ss.str().size() > 0)
    {
        MyCLI.OutputText(ss.str());
        ss.str("");
    }

    return success;
}
// return true if this devDesignator is an ethernet interface
bool DeviceList::IsTTY(int i){

    if ((i< 0) || (i>MAXDEVDES))
        return false;

    return IsTTY(interfaces[i]);
}

bool DeviceList::IsTTY(std::string interface){
    return (StringLeft(interface,3) == "tty" || StringLeft(interface, 6) == "serial");
}

// Get the devicedesignator for the Device with this binding
std::string DeviceList::getBoundDesignator(int i, string intface){
    if (i < 0) return "Invalid";
    if (i >= MAXDEVDES ) return "Invalid";
    int x;

    for (x=0; x<MAXDEVDES; x++){
        if ((getBinding(x) == i) && (interfaces[x] == intface))
            return designator[x];
    }
    
    return "";

};

// Get the device designator for the Device with this binding
std::string DeviceList::getDevDes(int i){
    if (i < 0)
        return "";
    if (i >= MAXDEVDES)
        return "";
    return designator[i];
 };

 // Get the device type index for the Device with this binding
int DeviceList::getDevTypeIndex(int i){
    if (i < 0)
        return 0;
    if (i >= MAXDEVDES)
        return 0;
    return devicetypes[i];
 };

 // Get the device designator for the Device with this binding
std::string DeviceList::getInterface(int i){
    if (i < 0)
        return "";
    if (i >= MAXDEVDES)
        return "";
    if (interfaces[i].size() > 0)
        return interfaces[i];
    else
        return "";
 };


// Get the IP address for the Device Designator i interface
std::string DeviceList::getSourceIPaddress(int i){
    if (i < 0) return "Invalid";
    if (i >=MAXDEVDES ) return "Invalid";
    int x;
    int sockindex;

    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
      if (i == tcpsockets[sockindex].myDevDesIndex){
          // This is the socket we are looking for
          return tcpsockets[sockindex].ConnectedToIP;

      }
   }

   return "";

};


// Get the device designator for the Device with this binding
int  DeviceList::IndexOf(string intface){
    int x;

    intface = trim(intface);
    
    if (intface.size() == 0)
        return -1;

    for (x=0; x<MAXDEVDES; x++){
        if (designator[x] == intface)
            return x;
    }

    return -1;

};



int DeviceList::DeviceTypeIndex(std::string s){
    // Find the index for this device.  -1 for not found.
    int i;
    s = StringToUpper(trim(s));    // get rid of spaces around text
    for (i=0; i < TOTALdTYPES ; i++){
        if (s == StringToUpper(typenames[i]))
            return i;
    }
    return -1;  // not found
};

bool DeviceList::rfportexists(int p){

    int i = 0;

    while (i < MAXDEVDES){
        if (channels[i] == p)
            return true;
        i++;
    };

    return false;
}

// Get the number of sockets that are created.
int DeviceList::getSocketCount(void){
    int i=0;
    int sockindex;
    int count = 0;

    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
      if (tcpsockets[sockindex].myDevDesIndex >= 0){
          count++;
      }
   }
  return count;

}


// Get the number of sockets that are created.
int DeviceList::getConnectedSocketCount(void){
    int i=0;
    int sockindex;
    int count = 0;

    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
      if ((tcpsockets[sockindex].myDevDesIndex >= 0) && (tcpsockets[sockindex].connected)){
          count++;
      }
   }
  return count;

}

// Get the number of sockets that are created.
int DeviceList::getValidSocketCount(void){
    int i=0;
    int sockindex;
    int count = 0;

    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
      if (tcpsockets[sockindex].interface.size() > 0){
          count++;
      }
   }
  return count;

}

int DeviceList::getChannelCount(void){

    int i = 0;
    int c = 0;
    while (i < MAXDEVDES){
        if (channels[i] >= 0)
            c++;
        i++;
    };

    return c;
}


// Return the number of radio channels that have incomming messages on them
int DeviceList::getActiveChannelCount(void){

    int i = 0;
    int c = 0;
    while (i < MAXDEVDES){
        if (OurDevices.channels[i] >= 0){
            if (IsEth(i)){
              if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS)){
                  if(tcpsockets[OurDevices.getBinding(i)].msg_in > 0){
                     c++;
                  }
              }
            }
            if (IsTTY(i)){ 
              if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
                  if(COMport[OurDevices.getBinding(i)].msg_in){
                      c++;
                  }                
            }
        }
        i++;
    }
   return c;
}


// Return the number of radio channels that have incomming messages on them
void DeviceList::ResetStatistics(void){

    int i = 0;
    int c = 0;
    while (i < MAXDEVDES){
        if (OurDevices.channels[i] >= 0){
            if (IsEth(i)){
              if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAXSOCKETS)){
                  tcpsockets[OurDevices.getBinding(i)].msg_in = 0;
                  tcpsockets[OurDevices.getBinding(i)].msg_out = 0;
                  tcpsockets[OurDevices.getBinding(i)].bytes_in = 0;
                  tcpsockets[OurDevices.getBinding(i)].bytes_out = 0;
                  tcpsockets[OurDevices.getBinding(i)].connects = 0;
                  tcpsockets[OurDevices.getBinding(i)].init_tries = 0;
                  }
              }
            if (IsTTY(i)){
              if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                  COMport[OurDevices.getBinding(i)].bytes_in = 0;
                  COMport[OurDevices.getBinding(i)].bytes_out = 0;
                  COMport[OurDevices.getBinding(i)].msg_in = 0;
                  COMport[OurDevices.getBinding(i)].msg_out = 0;
                  }
            }
        }
        i++;
    }

}


// Return true if this interface and port are not currently assigned to something
bool DeviceList::InterfaceIsFree(std::string interface, int port){
    int i = 0;

   

    return true;
}


// Return true if this interface and port are not currently assigned to something
bool DeviceList::InterfaceIsFree(std::string interface){
    int i = 0;

    while (i < MAXDEVDES){
        if (interfaces[i] == interface)
            return false;
        i++;
    };

    return true;
}


// Return true if this parameter is already used as a device designator
bool DeviceList::IsDesignator(std::string s){
    int i = 0;

    if (s.size() == 0)
        return false;

    s = trim(s);

    if (s.size() == 0)
        return false;

    while (i < MAXDEVDES){
        if (designator[i] == s)
            return true;
        i++;
    };

    return false;
}

// Use to add these devices:   tty
int DeviceList::AddttyDevice(string dts, string intf, string devdes){

   // Get the index into our device structure
    string errs = "";

    // See if the device is supported by this verion of code
    int dt = DeviceTypeIndex(dts);
    if (dt < 0)
    {
       return -1;    // this type of device is not supported
    }

    // This routine can only add devices that use the tty interface
    if (IsTTY(intf))
        return -1;

    // This interface/port is not currently assigned to anything. Try to bind it to radio channel
    // Find and open index to use
    int i;
    for (i=0; i< MAXDEVDES; i++){
        if(devicetypes[i] == dNONE)
            break;
    }

    if (devicetypes[i] == dNONE){
        // We found an unused device object.  Turn it into a device
        pthread_mutex_lock(&devlock);
        devicetypes[i] = dt;
        interfaces[i] = intf;   // the interface we are going to use.
        descriptions[i] = "";   // a text description for this device
        channels[i] = -1;       // we don't use a channel for  this type of device
        designator[i] = devdes; // The device designator
        pthread_mutex_unlock(&devlock);
        return i;            // we successfully added it.
    }

  return -1;
}

// {channel:int} {device type:string}  {interface:string]  {port:int} {protocol:string}
// Use to bind a device that use TCP/IP sockets to a TCP object.
// Return the index to the TCP object we are going to use.  -1 fail.
int DeviceList::ConnectDeviceSocket(int devindex,  std::string intf, int portno, 
        std::string protocol, std::string remoteserver, int ctimeout, bool keepalive){
    int tcpindex;
    int pr = -1;
    string devdes = "";
    in_addr_t ip = 0;
    in_addr_t mask = 0;
    string s;

    if ((devindex < 0) && (devindex >= MAXDEVDES)){
        return -6;  // invalid device des index
    }


    // Determine the type of protocol we want to use, and set the designator integer
    if (StringToUpper(protocol) == "CLIENT"){
        pr = pClient;
        remoteserver = trim(remoteserver);
        if (remoteserver.size() < 7){
            return -1; // failed. No server IP
        }
    }

    if (StringToUpper(protocol) == "SERVER"){
        pr = pServer;
        remoteserver = "";    // no remote server when we are the server.
    }

    if (StringToUpper(protocol) == "UDPTX"){
        pr = pDGRAMTX;
        remoteserver = trim(remoteserver);
    }
    if (StringToUpper(protocol) == "UDPRX"){
        pr = pDGRAMRX;
        remoteserver = trim(remoteserver);
    }

    if (pr == -1)
        return -2;  // no valid protocol

    // See if thie interface exists
    intf = trim(intf);

    if (intf.size() < 1)
        return -3;

    if ((devindex >= 0) && (devindex < MAXDEVDES)){
        tcpindex = GetNewTCPindex();  // get the next unused TCP socket object
        if ((tcpindex >= 0) && (tcpindex < MAXSOCKETS)){
            // Store the binding info in the device structure
            pthread_mutex_lock(&devlock);
            OurDevices.setBinding(devindex, tcpindex);
            devdes = OurDevices.designator[devindex];  // device designator string
            pthread_mutex_unlock(&devlock);
            tcpsockets[tcpindex].myDevDesIndex = devindex;
            tcpsockets[tcpindex].myDevType = OurDevices.devicetypes[devindex];  // the hardware type we talk to
            tcpsockets[tcpindex].index = tcpindex;     // tell each object what its index is
            tcpsockets[tcpindex].description = OurDevices.getDevDes(devindex);
            tcpsockets[tcpindex].MyParser.DefaultSrcID = DEFAULT_ID; // Get the default port number
            tcpsockets[tcpindex].MyParser.DefaultDstID = WNAT.GetID(OurDevices.getDevDes(devindex), portno); // Get the default port number
            tcpsockets[tcpindex].clienttimeout = ctimeout;
            tcpsockets[tcpindex].enable_keepalive = keepalive;
            // Setup the ports we use
            switch (pr){
                case pServer:
                    tcpsockets[tcpindex].portnum = portno;  // Use the assigned port ot listen on
                    tcpsockets[tcpindex].hostport = 0;      // No host port yet. Not till we are connected
                    break;
                case pClient:
                    tcpsockets[tcpindex].portnum = CLIENT_PORT_BASE + tcpsockets[tcpindex].index;   // The default port we will communicate on
                    tcpsockets[tcpindex].hostport = portno;         // Port number on the remote host
                    break;
                case pDGRAMTX:
                    tcpsockets[tcpindex].portnum = CLIENT_PORT_BASE + tcpsockets[tcpindex].index;   // The default port we will communicate on
                    tcpsockets[tcpindex].hostport = portno;         // Port number on the remote host
                    break;
                case pDGRAMRX:
                    tcpsockets[tcpindex].portnum = portno;   // The default port we will communicate on
                    tcpsockets[tcpindex].hostport = 0;      // No host port yet. Not till we get a packet
                    break;
                }
            tcpsockets[tcpindex].init_tries = 0;       // have not yet tried to initialize it.
            tcpsockets[tcpindex].interface = trim(intf);     // the default ethernet interface.
            s = GetIP(tcpsockets[tcpindex].interface.c_str(), &ip, &mask);
            //cout << "eth0dfsdf =" << s << endl;

            tcpsockets[tcpindex].myIP4 = s;
            tcpsockets[tcpindex].hostaddress = remoteserver;
            tcpsockets[tcpindex].protocol = pr;

            //tcpsockets[tcpindex].description = OurDevices.getBoundDesignator(tcpindex, tcpsockets[tcpindex].interface);
            // Now go try to initialize this socket
            tcpsockets[tcpindex].initialize_socket();

           // cout << "Connecting " << devdes << " to:" << tcpsockets[tcpindex].interface << ":" << portno << endl;
                          // the protocl to use.
        return true;
        }
    }
    return -4;
}

//  command line:  RFPORT {channel:int} {device type:string}  {interface:string]  {port:int}
//  ch=channel  dt=device type code  intf=interface on this machine  port=port number
int DeviceList::AddRadioChannel(std::string devdes, int ch, std::string dts, std::string intf, int portno, std::string proto){

    // Get the index into our device structure
    int deviceIndex = -1; // parm ch is the RF channel number.
    string errs = "";
    stringstream ss;

    if (rfportexists(ch)){
        // Warning. This  device is already assigned to something.
        ss << "Warning 923: Channel " << ch << " already assigned. " << endl;
        elog.store(string("Warning 923: Channel already assigned:" + intToString(ch)));
        MyCLI.OutputText(ss.str());
        ss.str("");
        //return -1;
    }

    // See if the device is supported by this verion of code
    int dt = DeviceTypeIndex(dts);
    if (dt < 0)
    {  // Error. This  device is not supported
       ss << "Error 924: Device not supported as a radio channel:" << dts << endl;
       elog.store(string("Error 924: Device not supported as a radio channel:" + dts));
        MyCLI.OutputText(ss.str());
        ss.str("");
       return -1;    // this type of device is not supported
    }

    if (InterfaceIsFree(intf, portno) == false){
       ss << "Error 925: Interface " << intf << " cannot be assigned to " << devdes << dts << endl;
       elog.store(string("Error 925: Interface" + intf + " cannot be assigned to " + devdes));
        MyCLI.OutputText(ss.str());
        ss.str("");
       return -1;    // this interface/port is already used so we cannot re-use it
    }

    // This device is not currently assigned to anything. Try to bind it to an interface
    // Find and open index to use in the array
    int i;
    for (i=0; i< MAXDEVDES; i++){
        if(devicetypes[i] == dNONE)   // == dNONE if this device entry is unassigned
            break;
    }

    deviceIndex = i;
    if (devicetypes[deviceIndex] == dNONE){
        // Unused entry, so lets use this one
        pthread_mutex_lock(&devlock);
        designator[deviceIndex] = devdes; // The device designator text
        devicetypes[deviceIndex] = dt;    // Remember the type of device this entry is talking to
        interfaces[deviceIndex] = intf;   // the interface we are bound to.
        descriptions[deviceIndex] = "";   // a text description for this device
        channels[deviceIndex] = ch;       // the radio channel
        pthread_mutex_unlock(&devlock);
        SiteManager.AddRFport(ch, deviceIndex, dt);  // add the rf port to our list of managed RF ports
        return deviceIndex ;              // we successfully added it.
    }

    if (ss.str().size() > 0){
        MyCLI.OutputText(ss.str());
        ss.str("");
    }
  return -1;
}


int DeviceList::getBaudRate(int i){

    switch(i){
        case dWMXmodem:
            return 38400;
        case dDataModem:
            return 38400;
        case dStatDisplay:
            return DISPLAYBAUD;

    }
    return -1;

}

int DeviceList::GetNewTCPindex(void){
    int i;

    for (i=0; i < MAXSOCKETS; i++ ){
        // cout << tcpsockets[i].interface  << endl;
        if (tcpsockets[i].myDevDesIndex < 0)
            return i;  // this one not used for any device yet.
    }
    return -1;
}

// *******************************************************************************
// Parse the Ethernet device designator table and add devicedesignators as needed
// *******************************************************************************
bool  DeviceList::LoadEthDevDesTable(datatable* dt){
 
    int  DevIndex = -1;
    int  rfp = -1;
    bool success = false;
    int didit = true;
    string dtype = "";
    string devdes = "";
    string intf = "";
    string ipadd = "";
    string protocol = "";
    string parm1 = "";
    bool tcp_keepalive = false;
    int ClientTimeout = 0;
    int pnum, i,dtypeindex;
    stringstream ss;
    string s = "";
    string emessage="";
    int pcount, port;
    ErrorsLoading = 0;
    LoadCount = 0;

    i = 0;
    while (i < dt->rows.size()){
         success = false;
         didit = true;     
         devdes = dt->GetItem(i, dt->IndexCol ); // get device designator text
         if (devdes.size() > 0 ){
            intf  = dt->GetItem(i, fld_interface);      // get the linux interface to use for this device
            CoutM2(ss)  << "Adding DeviceDesignator:" <<  devdes << " " << intf << endl;
            //if (StringLeft(intf, 3) == "eth"){
                dtype = dt->GetItem(i, fld_device);     // get the type of device we are connected to
                dtypeindex = DeviceTypeIndex(dtype);    // the numeric index for this type of device

                // Create an index into the devicedesignator table for this new device
                if ((dtypeindex == dDataModem) || (dtypeindex == dWMXmodem)){
                    // This is a radio interface. Add it
                    rfp = dt->GetIntItem(i, fld_channel);   // get the ethernet port number
                    DevIndex = AddRadioChannel(devdes, rfp, dtype, intf, pnum, protocol);
                    if (DevIndex == -1){
                        emessage = "Error: RF Channel number " + intToString(rfp) + " already assigned to" + devdes ;
                        elog.store(emessage);
                        CoutM2(ss)  << emessage << endl;
                    }
                }else{
                    // This is not a radio channel.  Add it.
                    DevIndex = AddConnection(dtype, intf, devdes) ;
                }

                // Now add a socket for this devicedesignator
                if (DevIndex >= 0){
                   // We added it to the device list, now connect it to a ethernet port yet
                   // Connect this device to an ethernet port
                   pnum  = dt->GetIntItem(i, fld_port);          // get the ethernet port number
                   pcount = dt->GetIntItem(i, fld_pcount);       // The number of ports to use. Usually 1.
                   protocol  = dt->GetItem(i, fld_protocol);     // get the protocol to use for this interface (server, client,...)
                   ipadd  = dt->GetItem(i, fld_ipadd);           // get the IP address to connect to if we are a client
                   parm1 = dt->GetItem(i, fld_parm1);            // get parameter1
                   OurDevices.descriptions[DevIndex] = dt->GetItem(i, fld_comment);
                   CoutM2(ss) << "Protocol:" <<  protocol << " "
                              << " port " << pnum << " on " << intf <<  " Pcount=" << pcount
                              << " IP: " << ipadd << endl;
                   if (pcount < 1)
                       pcount = 1;
                   port = pnum;   //  begin by using the first port defined.
                   if (parm1.size() > 0){
                       ClientTimeout = StringToInt(parm1);
                       if (StringToInt(parm1) < 0)
                            tcp_keepalive = true;   // user want this socket to send keep-alive packets out.  
                       else
                            tcp_keepalive = false;
                   }else{
                       ClientTimeout = 0;
                       tcp_keepalive = false;
                   }
                   while ((pcount > 0)  && didit){
                       // Create a socket for each port on this DevDes uses.
                       if (InterfaceExists(intf)){
                           didit = ConnectDeviceSocket(DevIndex, intf, port, protocol, ipadd, ClientTimeout, tcp_keepalive);
                           // cout << "DID:" << didit << endl;
                           if (didit != true){
                               CoutM2(ss) << "Error 392. Failed to create socket for:" <<  devdes << " "
                                          << " port " << port << " on " << intf <<  " EC=" << didit << endl;
                               elog.store(string("Error 392. Failed to create socket for:" + devdes + " Port:" +  intToString(port)));
                                ErrorsLoading++;
                           }else{
                               // successful creation of ethernet socket for this device designator
                               CoutM2(ss) << "New eth DeviceDesignator socket:" <<  devdes << " " << intf << " port:" << port << endl;
                               LoadCount++;
                           }
                       }else{
                           CoutM2(ss) << "Error 393. Invalid eth interface for device designator:" <<  devdes << " on " << intf <<  endl;
                           elog.store(string("Error 393. Invalid eth interface for device designator:" + devdes));
                           ErrorsLoading++;
                       }                           
                       pcount--;
                       port++;
                   }
                   success = true;
                }else{
                    CoutM2(ss) << "Error. Could not create socket for " << devdes << " on " << intf << endl;
                    emessage = emessage + " Could not create socket";
                    ErrorsLoading++;
                }

             //}
         }
         i++;
         if (success){

         }else{
           ss << "Failed to add New DeviceDesignator " << devdes << ". " << emessage << endl;
           elog.store(string("Failed to add devicedesignator:" + devdes));
         }
    }

    // Output the status messages to the command-line
    if (ss.str().size() > 0 ){
         MyCLI.OutputText(ss.str());
         ss.str("");
    }
    return  success;
}

// ***********************************************************************
// Parse the device designator table and add devicedesigntators as needed
// ***********************************************************************
//bool LoadTtyDevDesTable(TtyDeviceTableAdapter* adapter);
bool DeviceList::LoadTtyDevDesTable(TtyDeviceTableAdapter* adapter)
{
    if (adapter == NULL)
        return false;

    if (!adapter->Load())
        return false;

    stringstream ssout;
    int DevIndex;
    int rfp = -1;
    bool success = false;
    string dtype;
    string devdes;
    string intf;
    string settings;
    string protocol;
    int pnum = 0;
    int baudrate;
    int dtypeindex;

    ErrorsLoading = 0;
    LoadCount = 0;

    for (int i = 0; i < adapter->RowCount(); i++)
    {
        success = false;

        devdes = adapter->GetString(i, 0);

        if (IsDesignator(devdes) == false)
        {
            if (devdes.size() > 0)
            {
                intf = adapter->GetString(i, 2);

                if (IsTTY(intf))
                {
                    dtype = adapter->GetString(i, 1);
                    dtypeindex = DeviceTypeIndex(dtype);
                    rfp = adapter->GetInt(i, 3);

                    if ((dtypeindex == dDataModem) || (dtypeindex == dWMXmodem))
                        DevIndex = AddRadioChannel(devdes, rfp, dtype, intf, pnum, protocol);
                    else
                        DevIndex = AddConnection(dtype, intf, devdes);

                    if (DevIndex >= 0)
                    {
                        settings = adapter->GetString(i, 4);
                        baudrate = adapter->GetInt(i, 5);
                        OurDevices.descriptions[DevIndex] = adapter->GetString(i, 6);

                        if (ConnectTTYdevice(DevIndex, intf, baudrate, settings))
                        {
                            CoutM2(ssout) << "New TTY DeviceDesignator:" << devdes << " "
                                          << dtype << " on " << intf
                                          << " baud:" << baudrate << " "
                                          << settings << endl;
                            success = true;
                            LoadCount++;
                        }
                    }
                    else
                    {
                        CoutM2(ssout) << "Error 922: Cannot add new DeviceDesignator" << devdes << endl;
                        elog.store(string("Error 922: Cannot add new DeviceDesignator:" + devdes));
                    }
                }

                if (!success)
                {
                    CoutM2(ssout) << "Failed to add New DeviceDesignator:" << devdes << endl;
                    elog.store(string("Error. Failed to add DeviceDesignator:" + devdes));
                    ErrorsLoading++;
                }
            }
        }
        else
        {
            CoutM2(ssout) << "Failed to add New DeviceDesignator:" << devdes << "(Duplicate)" << endl;
            elog.store(string("Error. Failed to add DeviceDesignator:" + devdes + "(Duplicate)"));
        }
    }

    if (ssout.str().size() > 0)
    {
        MyCLI.OutputText(ssout.str());
        ssout.str("");
    }

    return success;
}
bool  DeviceList::LoadTtyDevDesTable(datatable* dt){
    stringstream ssout;

    int  DevIndex;
    int  rfp = -1;
    bool success = false;
    string dtype;
    string devdes;
    string intf ;
    string ipadd;
    string settings;
    string protocol;
    int pnum, i, baudrate, dtypeindex;
    string s;
    ErrorsLoading = 0;
    LoadCount = 0;

     i = 0;
     while (i < dt->rows.size()){
         success = false;
         devdes = dt->GetItem(i, dt->IndexCol );          // get device designator text

         // Test to see if this devdes is already in use.
         if (IsDesignator(devdes) == false){
             if ( devdes.size() > 0 ){
                intf  = dt->GetItem(i, fld_t_interface);      // get the linux interface to use for this device
                if (IsTTY(intf)){
                    dtype = dt->GetItem(i, fld_t_device);     // get the type of device we are connected to
                    dtypeindex = DeviceTypeIndex(dtype);      // the numeric index for this type of device
                    rfp = dt->GetIntItem(i, fld_t_channel);   // get the ethernet port number


                    // Create an index into the devicedesignator table for this new device
                    if ((dtypeindex == dDataModem) || (dtypeindex == dWMXmodem)){
                        // This is a radio interface. Add it
                        DevIndex = AddRadioChannel(devdes, rfp, dtype, intf, pnum, protocol);
                    }else{
                        // This is not a radio channel.  Add it.
                        DevIndex = AddConnection(dtype, intf, devdes) ;
                    }

                    if (DevIndex >= 0){
                       // We added it to the device list, not connect it to a i/o port yet
                           // Connect this device to a serial port
                           settings = dt->GetItem(i, fld_t_settings);
                           baudrate = dt->GetIntItem(i, fld_t_baudrate);
                           OurDevices.descriptions[DevIndex] = dt->GetItem(i, fld_t_comment);
                           if (ConnectTTYdevice(DevIndex, intf,  baudrate, settings)){
                             CoutM2(ssout) << "New New DeviceDesignator:" << devdes << " " << dtype << " on " <<  intf << " baud:" << baudrate << " " << settings << endl;
                             success = true;
                             LoadCount++;
                           }
                    }else{
                        CoutM2(ssout) << "Error 922: Cannot add new DeviceDesignator" << devdes << endl;
                        elog.store(string("Error 922: Cannot add new DeviceDesignator:" + devdes));
                    }
                 }
                 if (!success){
                    CoutM2(ssout) << "Failed to add New DeviceDesignator:" << devdes << endl;
                    elog.store(string("Error.  Failed to add DeviceDesignator:" + devdes));
                    ErrorsLoading++;
                 }
             }
         }else{
           CoutM2(ssout) << "Failed to add New DeviceDesignator:" << devdes << "(Duplicate)" << endl;
           elog.store(string("Error.  Failed to add DeviceDesignator:" + devdes + "(Duplicate)"));
         }
         i++;
   }
    // Output the status messages to the command-line
    if (ssout.str().size() > 0 ){
         MyCLI.OutputText(ssout.str());
         ssout.str("");
    }
    return  success;

}

//bool LoadTtyDevDesTable(TtyDeviceTableAdapter* adapter);
