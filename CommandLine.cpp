/* 
 * File:   CommandLine.cpp
 * Author: john
 * 
 * Created on August 2, 2010, 4:56 PM
 * Handles in input/output to the command-line user interface
 */
#include "platform/thread/PlatformLockGuard.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "platform/Platform.h"
#include <map>
#include <queue>
#include <vector>
#include <exception>
#ifndef _WIN32
#include <sys/resource.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#endif

#include "Cigorn.h"     // Our application-specific constants
#include "GlobalVar.h"
#include "serialhandler.h"
#include "CommandLine.h"
#include "ascii.h"
#include "rfport.h"
#include "CommThread.h"
#include "Console.h"
#include "CommandLine.h"
#include "datarow.h"
#include "datatable.h"
#include "database.h"
#include "Router.h"
#include "VT100.h"
#include "SocketThread.h"
#include "cypher.h"
#include "sync-roles.h"

// Prototype our local functions
bool okToExecute(rncommand , int );

queue<string> qCLIout;  // Console output FIFO.
queue<string> qCLIin;   // Command line input FIFO


// The parsed command parameters
const int MAXPARMS = 10;

string sx[MAXPARMS];  // string parameters
int    si[MAXPARMS];  // integer versions
string SX[MAXPARMS];  // Uppercase string parms

#include "procinfo.h"

// Construct the CLI object
CommandLine::CommandLine() {

    ResultStr = "";
    mycommand = "";

}

CommandLine::CommandLine(const CommandLine& orig) {
}

CommandLine::~CommandLine() {
}


int CommandLine::CommandQcount(void){
    int x = qCLIin.size();
    return x;
}

void CommandLine::IntoCommandQ(string s){

{
    cigorn::PlatformLockGuard lock(cmdqlock);
    qCLIin.push(s);
}

};
// ******************************************************************************
// This routine processes a command line string.
// device must be a type that this command is allowed to execute.
// The result is returned in ResultStr
// ******************************************************************************
bool CommandLine::processCommand(string cmd, int device){

    stringstream ssout;

    int i = 1;  //number of commands and parameters
    int c = 1;
    bool found = false;
    bool retval = false;  // return false if we do not properly execute the command
    ResultStr = "";
    
    // First parse the parameters
    if (cmd.length() > 0 ){
        while (i < MAXPARMS){
            sx[i] = GetSubString(cmd, i);  // get each individual paramter on the line
            SX[i] = StringToUpper(sx[i]);  // UPPERCASE version of the paramters.
            si[i] = StringToInt(sx[i]);    // convert to interger if possible.
            if (sx[i].length() <= 0)
                break;  // no more paramters
            i++;
        }
    }
    else{
        // erase the data. No command on this line.
        for (i = 0; i < MAXPARMS; i++){
            sx[i] = "";
            SX[i] = "";
            si[i] = 0;
        }
        i= 0;  // no commands
        return true;  // all is OK. No command to process. 
    }

   //ssout << cmd << " i=" <<  i << " " << sx[3] << " " << sx[4] << " " << sx[5] << endl;
   // ssout << cmd << endl;
   // is there a command?
   if (i > 0){
       // There appears to be a valid command entered. Try to find it
       while (c<= commandMap.size()){
           // search for the command
           if (StringToUpper(sx[1]) == commandMap[c].command){

                // Found the command!
                found = true;
                // see if it is OK to execute this command via this device
                if (okToExecute(commandMap[c], device) == false)
                    return false;   // This device is not allowed to execute this command.
                if (StringToUpper(sx[2]) == "?"){
                    // User wants some help
                    ssout << commandMap[c].help << endl;
                    ResultStr = ssout.str();
                    
                    retval = true;
                }
                else{
                    try{
                        ResultStr="";
                        // Execute the function related to this command.
                        if (commandMap[c].function == "cmdConfig" ){
                            retval = cmdConfig();
                        }else if (commandMap[c].function == "cmdAutoadd" ){
                            retval = cmdAutoadd();
                        }else if (commandMap[c].function == "cmdChparm" ){
                            retval = cmdChparm();
                        }else if (commandMap[c].function == "cmdSet" ){
                            retval = cmdSet();
                        }else if (commandMap[c].function == "cmdDBase" ){
                            retval = cmdDBase();
                        }else if (commandMap[c].function == "cmdEmail" ){
                            retval = cmdEmail();
                        }else if (commandMap[c].function == "cmdErrLog" ){
                            retval = cmdErrLog();
                        }else if (commandMap[c].function == "cmdExit" ){
                            retval = cmdExit();
                        }else if (commandMap[c].function == "cmdGateway" ){
                            retval = cmdGateway();
                        }else if (commandMap[c].function == "cmdDevices" ){
                            retval = cmdDevices();
                        }else if (commandMap[c].function == "cmdShowHelp" ){
                            retval = cmdShowHelp();
                        }else if (commandMap[c].function == "DoNothing" ){
                            retval = DoNothing();
                        }else if (commandMap[c].function == "cmdSerial" ){
                            retval = cmdSerial();
                        }else if (commandMap[c].function == "cmdShow" ){
                            retval = cmdShow();
                        }else if (commandMap[c].function == "cmdStatistics" ){
                            retval = cmdStatistics();
                        }else if (commandMap[c].function == "cmdEcho" ){
                            retval = cmdEcho();
                        }else if (commandMap[c].function == "cmdSockets" ){
                            retval = cmdSockets();
                        }else if (commandMap[c].function == "cmdVer" ){
                            retval = cmdVer();
                        }else if (commandMap[c].function == "cmdRoute" ){
                            retval = cmdRoute();  //"
                        }else if (commandMap[c].function == "cmdReboot" ){
                            retval = cmdReboot();  //cmdReboot"
                        }else if (commandMap[c].function == "cmdReload" ){
                            retval = cmdReload();  
                        }else if (commandMap[c].function == "cmdRadio" ){
                            retval = cmdRadio();  
                        }else if (commandMap[c].function == "cmdMessage" ){
                            retval = cmdMessage();  
                        }else if (commandMap[c].function == "cmdExitCLI" ){
                            retval = cmdExitCLI();  //
                        }else if (commandMap[c].function == "cmdReset" ){
                            retval = cmdReset();  //
                        }else if (commandMap[c].function == "cmdPause" ){
                            retval = cmdPause();  //
                        }else if (commandMap[c].function == "cmdUnpause" ){
                            retval = cmdUnpause();  //
                        }else if (commandMap[c].function == "cmdQueuereport" ){
                            retval = cmdQueuereport();  //
                        }else
                            retval == false; //

                        ssout << ResultStr << endl;
                        
                        //retval =  (*commandMap[c].function)();
                    }
                    catch(...){
                      retval = false;
                    }
                }
                break;
            };
            c++;
       }// while..
       // We check all possible commands. Did we find it??
       if (found == false){
           // invalid command
           retval = false;  // always false if we did not find he command.
       }
    }
    return retval;

}

// Set flag to indicate whether a command via a particular interface is OK to execute.
bool okToExecute(rncommand cmd , int device){

    switch (device) {
        case dCLI:
            return (cmd.viaCLI == true);
            break;
        case dINI:
            return (cmd.viaINI == true);
            break;
        case dCigorn:
            return (cmd.viaRNC == true);
            break;
        case dTerminal:
            return (cmd.viaCLI == true);
            break;
    };
    return false;
}


// Copy the initial command structure over the the map. The map is easier to manipulate
// so we use a map to handle the command processing.
int CommandLine::addAllCommands(void){
    int x;
    #include "CommandDefinitions.h"

    for (x=0; x < TOTAL_INITIAL_COMMANDS; x++){
        commandMap[x+1] = ourCommands[x];
    }
    return TOTAL_INITIAL_COMMANDS;

}

// Set/read the radio channel management parameters
bool CommandLine::cmdChparm(void){

    // Set the user name
    if (SX[2] == "asfafd"){
        if (SX[3].size() > 0){
          
        }
        else{
           
        }
        return true;
    }

return true;
}


bool  CommandLine::cmdDBase(void){
    int i=1;
    stringstream ssout;

    // Set the user name
    if (SX[2] == "USERNAME"){
        dbUser = sx[3];
        return true;
    }
    if (SX[2] == "SETPASS") {
        dbPass = sx[3];
        return true;
    }
    if (SX[2] == "TYPE") {
        dbType = sx[3];
        return true;
    }
 ssout << "Databse:" << dbName << "  User:" << dbUser << endl;
 ResultStr = ssout.str();
 return true;

}


bool  CommandLine::cmdShow(void){
    int i = 1;
    int IDlow = 0;
    int IDup = INT_MAX;
    string s;
    stringstream ssout;

#if 0
    if (SX[2] == "KILLME"){
        while(1){

        }
    }

#endif


    // Show the Wireless Devices
    if (SX[2] == "WD"){


       // List all of the wireless devices in our system
       ssout << set_bold(true) << cHEADING << FixedRight(lbl_ID, w_ID) << FixedRight(lbl_enabled, w_enabled)
            << FixedRight(lbl_rssi, w_rssi)
            << FixedRight(lbl_countFm, w_count)
            << FixedRight(lbl_tmLastMsg, w_timestamp)
            << cNONE << set_bold(false) << endl;

       if ((SX[3].size() > 0) && (SX[4].size() > 0)){
           // Limit display range to certain IDs
           IDlow = si[3];
           IDup = si[4];
       }
       // Loop through the table of our Wireless Devices using the map iterator (dit)
       for (dtWD->dit = dtWD->rows.begin(); dtWD->dit != dtWD->rows.end(); dtWD->dit++){
           i = StringToInt(dtWD->GetItem(dtWD->dit->first, fld_ID));
           if ((i >= IDlow) && ( i <= IDup)){
               // dr = dtWD.dit->second;
               ssout << cHEADLINE << FixedRight(dtWD->GetItem(dtWD->dit->first, fld_ID), w_ID)
                 << cITEM << FixedRight(dtWD->GetItem(dtWD->dit->first, fld_enabled), w_enabled)
                 << FixedRight(dtWD->GetItem(dtWD->dit->first, fld_rssi), w_rssi)
                 << FixedRight(dtWD->GetItem(dtWD->dit->first, fld_countFm), w_count)
                 << FixedRight(dtWD->GetItem(dtWD->dit->first, fld_tmLastMsg), w_timestamp)
                 << " " << FixedRight(dtWD->dit->second.RowState(), 5)
                 << cNONE << endl;
           }
       }
       ssout << "WD count = " << dtWD->rows.size() << endl;
       ResultStr = ssout.str();
       return true;
    }

     // Show the route history
     if (SX[2] == "HISTORY"){
         ssout << set_bold(true) << cHEADING << " Route History:" << cNONE << endl;
         for (DataRouter.lrit = DataRouter.LastRoutes.begin(); DataRouter.lrit != DataRouter.LastRoutes.end(); DataRouter.lrit++ ){
            ssout << cHEADLINE << trim(ctime(&DataRouter.lrit->timein)) << "  " << cITEM << "From:" << DataRouter.lrit->srcDevDes << "(ID=" <<  DataRouter.lrit->srcID << ")"
                  << " To:" << DataRouter.lrit->dstDevDes << "(ID=" <<  DataRouter.lrit->destID << ")"
                  << " Bytes:" << DataRouter.lrit->bytecount
                  << cNONE << endl;
         }
         ResultStr = ssout.str();
         return true;
    }

    if (SX[2] == "ELOG"){
         ssout << set_bold(true) << cHEADING << " Error Log:" << cNONE;
         ssout << "  (" << elog.ErrorMessages.size() << " entries)" << set_bold(false)<< endl;
         vector <string>::iterator it;
         // output the error log entries
         for (i=elog.ErrorMessages.size()-1; i>=0; i--){
            ssout << cHEADLINE << i << "  " << cITEM << elog.ErrorMessages[i] << cNONE << endl;
         }
         ResultStr = ssout.str();
         return true;
    }

    // Show all ports used on this machine
    if (SX[2] == "PORTS"){
        ssout << set_bold(true) << cHEADING << " Local IP ports in use:" << cNONE << endl;
        procinfo MyPortInfo;
        MyPortInfo.ReadMyPorts();  // get the port information
        for (MyPortInfo.itr = MyPortInfo.MyPortInfo.begin(); MyPortInfo.itr != MyPortInfo.MyPortInfo.end(); MyPortInfo.itr++){
            ssout << cHEADLINE << MyPortInfo.itr->first << "  " << cITEM << MyPortInfo.itr->second.MyIP4 << ":"
                 << MyPortInfo.itr->second.MyPort << cHEADLINE << " to " << cITEM
                 << MyPortInfo.itr->second.RemAddress << ":" << MyPortInfo.itr->second.RemPort << cNONE << endl;
        }
       ResultStr = ssout.str();
       return true;
    }


    // Show email configuration  myEmail
    if (SX[2] == "EMAIL"){
         ssout << set_bold(true) << cHEADING << " Email settings and status" << cNONE << set_bold(false)<< endl;
         ssout << cHEADLINE << "Email server:               " << cITEM << myEmail.server << cNONE << endl;
         ssout << cHEADLINE << "Server port number:         " << cITEM << myEmail.portnum << cNONE << endl;
         ssout << cHEADLINE << "Email client send state:    " << cITEM << myEmail.mystate << cNONE << endl;
         ssout << cHEADLINE << "Emails sent:                " << cITEM << myEmail.CountSent << cNONE << endl;
         ssout << cHEADLINE << "Emails failed to send:      " << cITEM << myEmail.CountFailed << cNONE << endl;
         ssout << cHEADLINE << "Emails server timeouts:     " << cITEM << myEmail.CountTimout << cNONE << endl;
         ssout << cHEADLINE << "SMTP timeout setting:       " << cITEM << myEmail.smtptimeout << cNONE << endl;
         ResultStr = ssout.str();
         return true;
     }

    // Show the Device Designator Table
    if (SX[2] == "DD"){
        // Loop through the table using the map iterator (dit)
       for (dtEDD->dit = dtEDD->rows.begin(); dtEDD->dit != dtEDD->rows.end(); dtEDD->dit++){
               ssout << cHEADLINE << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_designator), w_devdes)
                 << cITEM << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_device), w_device)
                 << cITEM << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_interface), w_interface)
                 << cITEM << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_port), w_port)
                 << cITEM << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_ipadd), w_ipadd)
                 << cITEM << FixedRight(dtEDD->GetItem(dtEDD->dit->first, fld_comment), w_device)
                 << cNONE << endl;
       }

       ssout << "row count = " << dtEDD->rows.size() << endl;
       ResultStr = ssout.str();
       return true;
    }
    
    // Show the fields defined in our tables.
    if ((SX[2] == "R") || (SX[2] == "ROUTES")){
        ssout << set_bold(true) << cHEADING << "Route Table: " << cNONE << set_bold(false) << endl;
        // Loop through the route table and list the entries
        ssout << cHEADLINE << DataRouter.RouteTableToText()  << cNONE << endl;
        ResultStr = ssout.str();
        return true;
    }

        // Show the list fo other sites we know about GateWays
     if (SX[2] == "CLUSTER"){
        ssout << "Hot-stanby Cluster Information" << endl;
        ssout << cHEADLINE << "This Site's Configuration    (" << cITEM << Me.MyName << ")" << endl;

        if (Me.gaterole == Standby){
            ssout << cHEADLINE << " My Site Name         :  " << cITEM <<  Me.MyName << endl;
            ssout << cHEADLINE << " Gateway Role         :  " << cITEM <<  "Standby" << endl;
            ssout << cHEADLINE << "   Active now?        :  " << cITEM << BoolToString(Me.IsActive) << endl;
            ssout << cHEADLINE << "   Primary Gateway    :  " << cITEM << Me.PrimaryName << endl;
            ssout << cHEADLINE << "   In sync w/Primary? :  " << cITEM << BoolToString(InSyncWithPrimary) << endl;
            ssout << cHEADLINE << "   Synchronized at    :  " << cITEM <<  DateTimeString(TimeOfLastPrimaryDBSync)  << endl;
            ssout << cHEADLINE << "   Wait time          :  " << cITEM << intToString(hotcutovertime)<< endl;
            ssout << cHEADLINE << "   Pri Check Interval :  " << cITEM << intToString(PrimaryTestInterval)<< endl;
            ssout << cHEADLINE << "   Primary GW IP      :  " << cITEM << PrimaryGWipadd << endl;
            ssout << cHEADLINE << "   Primary DB IP      :  " << cITEM << PrimaryDBipadd << endl;
            ssout << cHEADLINE << "Cluster Communications   " << cITEM  << endl;
            ssout << cHEADLINE << "   Connected          :  " << cITEM << BoolToString(CigSocketOut.connected) << endl;
            ssout << cHEADLINE << "   Out port           :  " << cITEM << intToString(CigSocketOut.portnum) << endl;
            ssout << cHEADLINE << "   Primary port       :  " << cITEM << intToString(CigSocketOut.hostport) << endl;
            ssout << cHEADLINE << "   Primary IP         :  " << cITEM << CigSocketOut.hostIPaddress << endl;
            ssout << cHEADLINE << "   Msg count in       :  " << cITEM << intToString(CigSocketOut.msg_in) << endl;
            ssout << cHEADLINE << "   Msg count out      :  " << cITEM << intToString(CigSocketOut.msg_out) << endl;
            ssout << cHEADLINE << "   State #            :  " << cITEM << intToString((int)MySyncState) << endl;
            // CigSocketOut
        }else{
            ssout << cHEADLINE << " My Site Name         :  " << cITEM <<  Me.MyName << endl;
            ssout << cHEADLINE << " Gateway Role         :  " << cITEM <<  "Primary" << endl;
            ssout << cHEADLINE << "   Active now?        :  " << cITEM << BoolToString(Me.IsActive) << endl;
            ssout << cHEADLINE << "   Standby Msg count  :  " << cITEM << intToString(MsgCountFromStandby) << endl;
            ssout << cHEADLINE << "Cluster Communications   " << cITEM  << endl;
            ssout << cHEADLINE << "   Cluster connected  :  " << cITEM << BoolToString(CigSocketIn.connected) << endl;
            ssout << cHEADLINE << "   Cluster  port      :  " << cITEM << intToString(CigSocketIn.portnum) << endl;
            ssout << cHEADLINE << "   Cluster  port      :  " << cITEM << intToString(CigSocketIn.portnum) << endl;
            ssout << cHEADLINE << "   State #            :  " << cITEM << intToString((int)MySyncState) << endl;
            // CigSocketOut
        }
        //
   
        ResultStr = ssout.str();
        return true;
     }

    // Show the list fo other sites we know about GateWays
     if (SX[2] == "SITES"){
         ssout << "List of the " << GateWays.size() << " Cigorn Gateways communicating to this site:" << endl;
         ssout << set_bold(true) << cHEADING << " Site Name       Chief       Interface      Address" << cNONE << set_bold(false) << endl;
         map<string, Gateway>::iterator Gwi;
         for (Gwi = GateWays.begin(); Gwi != GateWays.end(); Gwi++ ){
             ssout << cHEADLINE << CenterString(Gwi->second.MyName, 12) << " "
                  << cITEM << CenterString(BoolToString(Gwi->second.IsChief), 12) << " "
                  << CenterString(OurDevices.designator[Gwi->second.DevDes], 12) << " "
                  << CenterString(Gwi->second.IPadd, 15) << " "
                  << cNONE << endl;
         }

        
         ResultStr = ssout.str();
         return true;
     }

     // Show the list fo other sites we know about GateWays
     if (SX[2] == "WNAT"){
         WNATEntry wne;
         ssout << set_bold(true) << cHEADING << "Wireless Network Address Translations" << cNONE << set_bold(false) << endl;
         int pnuml, pnumh, idl, idh;

         for (i = 0; i < WNAT.WNATentries.size(); i++){
             if (WNAT.GetEntry(i, wne)){
                if (wne.PortCount > 0 ){
                     pnuml = OurDevices.getPortNum(OurDevices.IndexOf(wne.Designator));
                     pnumh = pnuml + wne.PortCount - 1;
                     idl = wne.lowerID;
                     idh = idl + wne.PortCount - 1;
                     ssout << cHEADLINE << "Ports:" << wne.Designator << ":" << pnuml << "-" << pnumh << "  translate to IDs:"
                           << cITEM << idl   << "-" << idh   << "  Default DevDes:" << wne.DefaultDevDes
                           << cNONE << endl;
                }
             }
         }
      
         ssout << set_bold(true) << cHEADING << "Wireless NAT History" << cNONE << set_bold(false) << endl;
         i=1;
         for (WNAT.wit = WNAT.history.begin(); WNAT.wit != WNAT.history.end(); WNAT.wit++ ){
                  ssout << cHEADLINE << i << cITEM << " " << *WNAT.wit << cNONE << endl;
                 i++;
         }
         ResultStr = ssout.str();
         return true;
    }


    // Show the Ethernet ports used, and which device designators are assigned to them
    if (SX[2] == "PORTDES"){
         ssout << set_bold(true) << cHEADING << " List of Connected Network Ports" << cNONE << set_bold(false) << endl;
         ssout << "Command removed temporarily" << endl;
         return true;
     }  

    // Show the fields defined in our tables.
    if (SX[2] == "F"){

        ssout << endl << "Table " << dtWD->tablename << endl;

        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(intToString(i), 10);
        }
        ssout << endl;

        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(dtWD->colname[i], 10);
        }
        ssout << endl;
         
        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(intToString(dtWD->type[i]), 10);
        }        
        ssout << endl;
         
        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(myDB.SQLtypes[dtWD->type[i]], 10);
        }

        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(BoolToString(dtWD->readonly[i]), 10);
        }

        ssout << endl << "Table " << dtRT->tablename << endl;

        for (i=0; i<dtRT->colname.size(); i++){
            ssout << FixedRight(dtRT->colname[i], 10);
        }
        ssout << endl;

        for (i=0; i<dtRT->colname.size(); i++){
            ssout << FixedRight(intToString(dtRT->type[i]), 10);
        }
        ssout << endl;
        for (i=0; i<dtWD->colname.size(); i++){
            ssout << FixedRight(myDB.SQLtypes[dtRT->type[i]], 10);
        }

        ssout << endl;
       ResultStr = ssout.str();
       return true;
     }

     if (SX[2] == "TYPES"){
         ssout << set_bold(true) << cHEADING << " Field Types the DB has" << cNONE << set_bold(false)<< endl;
         map<int, string>::iterator itx;
         for (itx = myDB.SQLtypes.begin(); itx != myDB.SQLtypes.end(); itx++){
            ssout << "DB OID: " << itx->first  << " " << itx->second << cITEM << cNONE << endl;
         }
         ResultStr = ssout.str();
         return true;
     }


     // Show web server configuration
     if (SX[2] == "WEB"){
         ssout << set_bold(true) << cHEADING << " WEB Server" << cNONE << set_bold(false)<< endl;
         ssout << cHEADLINE << "Web Server port       :  " << cITEM <<  myWeb.portnum << endl;
         ssout << cHEADLINE << "Connected             :  " << cITEM <<  myWeb.MySocket.connected << endl;
         ssout << cHEADLINE << "Socket fd             :  " << cITEM <<  myWeb.MySocket.sockfd << endl;
         ssout << cHEADLINE << "New Socket fd         :  " << cITEM <<  myWeb.MySocket.newsockfd << endl;
         ssout << cHEADLINE << "Connected to IP       :  " << cITEM <<  myWeb.MySocket.ConnectedToIP << endl;
         ssout << cHEADLINE << "Local port            :  " << cITEM <<  myWeb.portnum << endl;
         ssout << cHEADLINE << "State                 :  " << cITEM <<  myWeb.mystate << endl;
         ResultStr = ssout.str();
         return true;
     }


     // Show the list fo other sites we know about GateWays
     if (SX[2] == "LIMITS"){
         ssout << set_bold(true) << cHEADING << " Various Limitations within this version of Cigorn" << cNONE << set_bold(false)<< endl;
         ssout << cHEADLINE << "TCP sockets:              " << cITEM << intToString( GetSocketStatus().count_max) << cNONE << endl;
         ssout << cHEADLINE << "TTY streams:              " << cITEM << MAX_TTY << cNONE << endl;
         ssout << cHEADLINE << "Gateway Site numbers:     " << cITEM << MAXSITENUM << cNONE << endl;
         ssout << cHEADLINE << "String Size:              " << cITEM << s.max_size() << cNONE << endl;
         BinaryEntry be;
         ssout << cHEADLINE << "Binary/XML message size:  " << cITEM << be.MAXDATA << cNONE << endl;
         ResultStr = ssout.str();
        return true;
     }

    // Show the clock and its calibration
     if (SX[2] == "TDMA"){
         ssout << set_bold(true) << cHEADING << " Internal TDMA Parameters" << cNONE << set_bold(false)<< endl;
         ssout << cHEADLINE << "Seconds since reset:      " << cITEM << SiteManager.MyTicker.Elasped() << cNONE << endl;
         ssout << cHEADLINE << "TDMA epoch duration:      " << cITEM << SiteManager.EpochTime << cNONE << endl;
         ssout << cHEADLINE << "TDMA epoch time:          " << cITEM << SiteManager.MyTicker.EpochTime(SiteManager.EpochTime) << cNONE << endl;
         ssout << cHEADLINE << "TDMA slot number:         " << cITEM << SiteManager.MyTicker.SlotNum(SiteManager.EpochTime, SiteManager.slottime) << cNONE << endl;
         ssout << cHEADLINE << "TDMA slot time:           " << cITEM << SiteManager.slottime << cNONE << endl;
         ResultStr = ssout.str();
         return true;
     }

    // Show the wireless system information
     if (SX[2] == "RF"){
         ssout << set_bold(true) << cHEADING << "Radio System Parameters" << cNONE << set_bold(false)<< endl;
         ssout << cHEADLINE << "Control channel number:      " << cITEM << SiteManager.ControlChan << cNONE << endl;
         ssout << cHEADLINE << "Max number of control ch:    " << cITEM << SiteManager.MaxControlChan << cNONE << endl;
         ssout << cHEADLINE << "Control chan device type:    " << cITEM << SiteManager.DeviceName(SiteManager.ControlChan) << cNONE << endl;
         ssout << cHEADLINE << "Control chan designator:     " << cITEM << SiteManager.DeviceDesignator(SiteManager.ControlChan) << cNONE << endl;
         ssout << cHEADLINE << "TDMA epoch duration:         " << cITEM << SiteManager.EpochTime << cNONE << endl;
         ssout << cHEADLINE << "TDMA slot time:              " << cITEM << SiteManager.slottime << cNONE << endl;
         ssout << cHEADLINE << "Number of on-site RF ports:  " << cITEM << SiteManager.RFports.size() << cNONE << endl;
         ssout << cHEADLINE << "---- RF Port Information ----" << cNONE << endl;
         rfportlist::iterator it;
         // List the info for each rf port
         for (it = SiteManager.RFports.begin(); it != SiteManager.RFports.end(); it++){
            ssout << cHEADLINE << "Info for RF Port  :" << cITEM << it->first << " " << OurDevices.getDevDes(it->second.DevDesIndx)  << cNONE << endl;
            ssout << cHEADLINE << "   Messages in    :" << cITEM << it->second.msgIn << cNONE << endl;
            ssout << cHEADLINE << "   Messages out   :" << cITEM << it->second.msgOut << cNONE << endl;
            ssout << cHEADLINE << "   WNC msgs in    :" << cITEM << it->second.wncin << cNONE << endl;
            ssout << cHEADLINE << "   WNC msgs out   :" << cITEM << it->second.wncout << cNONE << endl;
         }

         ResultStr = ssout.str();
         return true;
     }

    if (SX[2] == "DELAY"){
        ssout  << set_bold(true) << cHEADING << "Routing Delay Information" << cNONE << set_bold(false) << endl;
        ssout << cHEADLINE << "Ave. route time (WMX)      " << cITEM  <<  doubleToString(AverageQ(dlyWMX),6) << "sec" <<  cNONE << endl;
        ssout << cHEADLINE << "Ave. route time (PRAVE)    " << cITEM  <<  doubleToString(AverageQ(dlyPRAVE),6) << "sec" <<  cNONE << endl;
        ssout << cHEADLINE << "Ave. route time (Cigorn)   " << cITEM  <<  doubleToString(AverageQ(dlyCIGORN),6) << "sec" << cNONE << endl;
        bool done = false;
        i = 0;
        while (done == false){
           ssout << cHEADLINE << "Route time (CIGORN PRAVE WMX) " << cITEM  ;
           done = true;
           if (i < dlyCIGORN.size()) {
             ssout << doubleToString(dlyCIGORN.at(i), 6);
             done = false;
           }
           else
             ssout << "--------";
           ssout << "  ";
           if (i<dlyPRAVE.size()) {
             ssout << doubleToString(dlyPRAVE.at(i), 6);
             done = false;
           }
           else
             ssout << "--------";
           ssout << "  ";
           if (i<dlyWMX.size()) {
             ssout << doubleToString(dlyWMX.at(i), 6);
             done = false;
           }
           else
             ssout << "--------";
           ssout << endl;
           i++;
        }
        ResultStr = ssout.str();
        return true;
    }

     
    ssout << "SHOW what?" << endl;
    ResultStr = ssout.str();
    return true;

}

bool  CommandLine::cmdAutoadd(void){
    stringstream ssout;

    // Show the Wireless Devices
    if (SX[2] == "YES"){
        myDB.AutoAddRows = true;
        dtWD->AutoAddRows = true;
        ResultStr = "";
        return true;
    }

    // Show the Wireless Devices
    if (SX[2] == ""
            ""
            "NO"){
        myDB.AutoAddRows = false;
        dtWD->AutoAddRows = false;
        ResultStr = "";
        return true;
    }
    ResultStr = ssout.str();
    return true;
}


bool  CommandLine::cmdShowHelp(void){
    int i=1;
    stringstream ssout;
    try{
        if (commandMap.size() > MAX_NUM_COMMANDS) return false;  // bad error
        ssout << "List of all " << commandMap.size() << " commands:" << endl;
        while (i <= commandMap.size()){
            if (commandMap[i].viaCLI){
                ssout << Right((commandMap[i].command + "            "),12) << "  " << commandMap[i].help << endl;
            }
            i++;
        }
        ResultStr = ssout.str();
        return true;
    }catch(...){
        return false;
    }

    ResultStr = ssout.str();
    return true;

}



bool  CommandLine::cmdEmail(void){

    if (SX[2] == "asdfasdf"){
        return true;
    }

    return false;

}

bool  CommandLine::cmdErrLog(void){

    if (SX[2] == "DEL"){
        return elog.eraseLog();
        elog.store("Deleted error log and created new one.");
        return true;
    }

    return false;

}

bool CommandLine::cmdConfig(void){
    stringstream ssout;
    // Global Statistics
    #ifndef _WIN32
struct rlimit mylimit;
#endif

int i;

    ssout << set_bold(true) << cHEADING << "System  Configuration  " << cNONE << set_bold(false) <<endl;
    ssout << cHEADLINE << "Gateway name          :  " << cITEM << Me.MyName << cNONE;
    if (Me.IsChief)
        ssout << cUNIT " (Chief)";
    ssout << cNONE << endl;
    ssout << cHEADLINE << "Software version      :  " << cITEM << GetVersionNum() << endl;

    ssout << cHEADLINE << "Gateway role          :  " << cITEM;
    if (Me.gaterole == Primary)
        ssout << "Primary";
    else
        ssout << "Standby";

    if (Me.IsActive == true)
        ssout << cUNIT << " (Active Now)" << endl;
    else
        ssout << cUNIT << " (Not Active)" << endl;


    if (Me.gaterole == Standby){
        ssout << cHEADLINE << "   Primary Gateway    :  " << cITEM << Me.PrimaryName << endl;
        ssout << cHEADLINE << "   In sync w/Primary? :  " << cITEM << BoolToString(InSyncWithPrimary) << endl;
        ssout << cHEADLINE << "   Synchronized at    :  " << cITEM <<  DateTimeString(TimeOfLastPrimaryDBSync)  << endl;
        ssout << cHEADLINE << "   Wait time          :  " << cITEM << intToString(hotcutovertime)<< endl;
        ssout << cHEADLINE << "   Pri Check Interval :  " << cITEM << intToString(PrimaryTestInterval)<< endl;
    }

    ssout << cHEADLINE << "Database              :  " << cITEM << dbName << cHEADLINE << "  User:" << cITEM << dbUser << endl;
    ssout << cHEADLINE << "Time                  :  " << cITEM << LocalTimeStamp() << endl;
    ssout << cHEADLINE << "OS user name          :  " << cITEM << get_username() << endl;
    ssout << cHEADLINE << "DB refresh interval   :  " << cITEM << Me.dbPushInterval << endl;
    ssout << cHEADLINE << "Number of RF channels :  " << cITEM << intToString(OurDevices.getChannelCount()) << endl;
    ssout << cHEADLINE << "TCP sockets used      :  " << cITEM << intToString( GetSocketStatus().count_used) << " out of " << intToString( GetSocketStatus().count_max) << endl;
    ssout << cHEADLINE << "TCP sockets connected :  " << cITEM << intToString( GetSocketStatus().count_connected) << endl;
    ssout << cHEADLINE << "Ethernet interfaces   :  " << cITEM << ipaddresses.size() << endl;

    // List all of our IP addresses on this machine
    IPaddList::iterator it;
    for (it=ipaddresses.begin(); it!= ipaddresses.end(); it++){
        ssout << cHEADLINE << "  Interface           : "
             << cITEM << FixedRight(it->second.interface, 5) << "   "
             << it->second.ipaddress << cNONE << CrNl();
    }
    ssout << cHEADLINE << "Msg Queue max count   :  " << cITEM << intToString(maxQcount) << endl;
    ssout << cHEADLINE << "Msg Queue max age     :  " << cITEM << intToString(maxQage) << endl;
    ssout << cHEADLINE << "Email notice level    :  " << cITEM << intToString(emailnotice) << endl;
    ssout << cHEADLINE << "Email notice interval :  " << cITEM <<  doubleToString(statusemailinterval, 2) << endl;
    ssout << cHEADLINE << "Web Server port       :  " << cITEM <<  myWeb.portnum << endl;
    #ifndef _WIN32
if (getrlimit(RLIMIT_NOFILE, &mylimit) == 0) {
    i = static_cast<int>(mylimit.rlim_max);
    ssout << cHEADLINE
          << "Max # of files/ports  :  "
          << cITEM
          << i
          << endl;
}
#else
ssout << cHEADLINE
      << "Max # of files/ports  :  "
      << cITEM
      << "Windows-managed"
      << endl;
#endif
    ssout << cHEADLINE << "My directory          :  " << cITEM <<  GetMyDirectory() << endl;
    ssout << cHEADLINE << "Status message level  :  " << cITEM <<  messagelevel << endl;

    procinfo MyProcInfo;
    ssout << cHEADLINE << "IPV4 TCP Keepalive    :  " << cITEM <<   MyProcInfo.GetKeepAliveSettings() << endl;
    ssout << cHEADLINE << "My Socket client ports:  " << cITEM <<   CLIENT_PORT_BASE <<  " - " << intToString(CLIENT_PORT_BASE+SERVER_PORT_RES) << endl;
    ssout << cNONE  << endl;  // always end with this

    ResultStr = ssout.str();
    return true;

};


bool CommandLine::cmdCon(string& r){

    // Global Statistics

    stringstream ssout;
    //ssout << "result:" << ResultStr << endl;

    ssout << set_bold(true) << cHEADING << "System  Configuration  " << cNONE << set_bold(false) <<endl;
    ssout << cHEADLINE << "Gateway name          :  " << cITEM << Me.MyName << cNONE;
    if (Me.IsChief)
        ssout << cUNIT " (Chief)";
    ssout << cNONE << endl;

    ssout << cHEADLINE << "Gateway Role          :  " << cITEM;
    if (Me.gaterole == Primary)
        cout << "Primary";
    else
        cout << "Standby";

    if (Me.IsActive == true)
        cout << " (Active Now)" << endl;
    else
        cout << " (Not Active)" << endl;

    ssout << cHEADLINE << "Software version      :  " << cITEM << GetVersionNum() << endl;
    ssout << cHEADLINE << "Database              :  " << cITEM << dbName << cHEADLINE << "  User:" << cITEM << dbUser << endl;
    ssout << cHEADLINE << "Time                  :  " << cITEM << LocalTimeStamp() << endl;
    ssout << cHEADLINE << "DB refresh interval   :  " << cITEM << Me.dbPushInterval << endl;
    ssout << cHEADLINE << "Number of RF channels :  " << cITEM << intToString(OurDevices.getChannelCount()) << endl;
    ssout << cHEADLINE << "TCP sockets used      :  " << cITEM << intToString( GetSocketStatus().count_used) << " out of " << intToString( GetSocketStatus().count_max) << endl;
    ssout << cHEADLINE << "TCP sockets connected :  " << cITEM << intToString( GetSocketStatus().count_connected) << endl;
    ssout << cHEADLINE << "Ethernet interfaces   :  " << cITEM << ipaddresses.size() << endl;

    // List all of our IP addresses on this machine
    IPaddList::iterator it;
    for (it=ipaddresses.begin(); it!= ipaddresses.end(); it++){
        ssout << cHEADLINE << "  Interface           : "
             << cITEM << FixedRight(it->second.interface, 5) << "   "
             << it->second.ipaddress << cNONE << CrNl();
    }
    ssout << cHEADLINE << "Number of RF channels :  " << cITEM << intToString(OurDevices.getChannelCount()) << endl;
    ssout << cHEADLINE << "Msg Queue max #       :  " << cITEM << intToString(maxQcount) << endl;
    ssout << cHEADLINE << "Msg Queue max age     :  " << cITEM << intToString(maxQage) << endl;
    ssout << cHEADLINE << "Email notice level    :  " << cITEM << intToString(emailnotice) << endl;
    ssout << cHEADLINE << "Email notice interval :  " << cITEM <<  doubleToString(statusemailinterval, 2) << endl;
    ssout << cNONE  << endl;  // always end with this

    ResultStr = ssout.str();
    return true;

};

bool CommandLine::cmdDevices(void){
    // List all of the devicesdesignators
    string s;
    stringstream ssout;
    int i;
    int SocketIndex;

    ssout << set_bold(true) << cHEADING << "List of Device Designators configured on this site:" << cNONE << set_bold(false) << endl;
    for (i = 0; i < MAXDEVDES ; i++){
        if (OurDevices.devicetypes[i] != dNONE){
            //cout << i << " Interface:" << OurDevices.interfaces[i] << endl;
            // This is a valid device, so list its settings.
            ssout << cHEADLINE << "Device #" << intToString(i) << " " << OurDevices.designator[i] << " "
                 << cITEM << OurDevices.DeviceName(OurDevices.devicetypes[i]) << " (" << OurDevices.interfaces[i] << ")";

            if (OurDevices.IsTTY(i)){
                if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY))
                    ssout << " baud:" <<  intToString(COMport[OurDevices.getBinding(i)].baudrate);
            }
            if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
                SocketIndex = OurDevices.getBinding(i);
                if ((SocketIndex > 0 ) && (SocketIndex < MAXSOCKETS))
                    ssout << " Port:" <<  tcpsockets[SocketIndex].portnum;
                else
                    ssout << " Invalid IP configuration."; //  Socket Index:" << SocketIndex;
            }
            ssout << endl;
        }
    }
   ssout << cNONE  << endl;  // always end with this
   ResultStr = ssout.str();

    return true;

};


// Turn ECHO  ON/OFF on the various interfaces
bool CommandLine::cmdEcho(void){

    // Global Statistics
    string s;
    int i;
    stringstream ssout;
    
    if (SX[2] == "OFF"){
        // User wants to turn local echoing off. disable it on all interfaces.
        for (i=0; i<MAXSOCKETS; i++){
          tcpsockets[i].localecho = false;        // no ech on any socket
        }
        {
    cigorn::PlatformLockGuard lock(ttylock);

    for (i = 0; i < MAX_TTY; i++) {
        COMport[i].localecho = false;
    }

        }
        return true;
    }

    // See if they want to echo a socket
    for (i=0; i<MAXSOCKETS; i++){
        s = StringToUpper(string(tcpsockets[i].interface));
        if ((s == SX[2]) && (tcpsockets[i].portnum == si[3])){
            // User wants to echo the ethernet data on port si[3] to the local console.
            tcpsockets[i].localecho = true;
            return true;
        }
    }

    // see if they want to echo a TTY
    for (i=0; i<MAX_TTY; i++){
        s = StringToUpper(COMport[i].devicename);
        if (s == SX[2]){
            COMport[i].localecho = true;
            return true;
        }
    }

    for (i = 0; i < TOTALdTYPES ; i++){
        if (OurDevices.designator[i] == SX[2]){
            if (OurDevices.IsTTY(i)){
                if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                    COMport[OurDevices.getBinding(i)].localecho = true;  // turn on echo
                    ssout << "Echoing " << OurDevices.designator[i] << "(" <<  OurDevices.interfaces[i] << ")" << endl;
                    ResultStr = ssout.str();
                    return true;
                }
            }
            if (StringLeft(OurDevices.interfaces[i],3) == "eth"){
                if ((OurDevices.getBinding(i) < MAXSOCKETS) && (OurDevices.getBinding(i) > 0)){
                    tcpsockets[OurDevices.getBinding(i)].localecho = true;
                    ssout << "Echoing " << OurDevices.designator[i] << "(" <<  OurDevices.interfaces[i] << " port "
                          << tcpsockets[OurDevices.getBinding(i)].portnum << ")"  << endl;
                    ResultStr = ssout.str();
                    return true;
                }            
            }
        }
    }
    return false;

};


// TCPECHO socket:int  ON/OFF
bool CommandLine::cmdSockets(void){

    // Global Statistics
    string s;
    bool OnlyConnected = true;
    int i;
    stringstream ssout;

    if (SX[2] == "DROP"){
        // Disconnect all sockets that are connected.
        for (i=0; i < MAXSOCKETS; i++){
           if ((tcpsockets[i].myDevDesIndex >= 0) && (tcpsockets[i].myDevDesIndex < MAXDEVDES)){
             if ((tcpsockets[i].connected == true) &&
                 (SX[3] == "ALL") ||
                 (si[3]==tcpsockets[i].portnum) ||
                 ((SX[3] == OurDevices.designator[tcpsockets[i].myDevDesIndex]) && (SX[3].size() > 0))) {
                 tcpsockets[i].DisconnectSocket();
                 s = s + cITEM + OurDevices.designator[tcpsockets[i].myDevDesIndex]
                      + " (" + tcpsockets[i].interface + ")" + " Port:" + intToString(tcpsockets[i].portnum);
                 s = s + cBOLDITEM + " disconnected from:" + tcpsockets[i].ConnectedToIP + ":" + intToString(tcpsockets[i].RemotePort);

             ssout << s <<  cNONE  << endl;
             }
           }
        }
        ResultStr = ssout.str();
        return true;
    }

    if (SX[2] == "ALL"){
        OnlyConnected = false;
        ssout << set_bold(true) << cHEADING <<  "List of All TCP/IP sockets:" << cNONE << set_bold(false) << endl;
    }else{
        ssout << set_bold(true) << cHEADING <<  "List of Connected TCP/IP sockets:" << cNONE << set_bold(false) << endl;
    }

    for (i=0; i < MAXSOCKETS; i++){
       if ((tcpsockets[i].myDevDesIndex >= 0) && (tcpsockets[i].myDevDesIndex < MAXDEVDES)){
         if ((OnlyConnected == false) || (tcpsockets[i].connected == true)) {
             if (tcpsockets[i].protocol == pServer){
                 s = cHEADLINE;
                 s = s + "Server #" + intToString(i) + " ";
                 s = s + cITEM + OurDevices.designator[tcpsockets[i].myDevDesIndex]
                  + " (" + tcpsockets[i].interface + ")" + " Port:" + intToString(tcpsockets[i].portnum);
                 if (tcpsockets[i].connected == true)
                     s = s + cBOLDITEM + " connected to:" + tcpsockets[i].ConnectedToIP + ":" + intToString(tcpsockets[i].RemotePort);
                 else
                     s = s + cBOLDITEM + " waiting";
                 if (SX[3] == "FD"){
                     s = s + " FD:" + intToString(tcpsockets[i].sockfd) + "/" + intToString(tcpsockets[i].newsockfd);
                 }
             }
             if (tcpsockets[i].protocol == pClient){
                 s = cHEADLINE;
                 s = s + "Client #" + intToString(i) + " ";
                 s = s + cITEM + OurDevices.designator[tcpsockets[i].myDevDesIndex] 
                       +  " (" + tcpsockets[i].interface + ")"
                       + " on my port:" + intToString(tcpsockets[i].portnum);
                 if (tcpsockets[i].connected == true){
                     s = s + cBOLDITEM + " connected to:" + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
                 }else{
                     if (tcpsockets[i].newsockfd >=0)
                        s = s + " accessing :" + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
                     else
                        s = s + " calling : " + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
                 }
                 if (tcpsockets[i].clienttimeout > 0)
                     s = s + " timeout=" + intToString(tcpsockets[i].clienttimeout);
             }
             if (tcpsockets[i].protocol == pDGRAMTX){
                 s = cHEADLINE;
                 s = s + "UDP Sender#" + intToString(i) + " ";
                 s = s + cITEM + OurDevices.designator[tcpsockets[i].myDevDesIndex]
                       +  " (" + tcpsockets[i].interface + ")"
                       + " on my port:" + intToString(tcpsockets[i].portnum);
                 if (tcpsockets[i].newsockfd >=0)
                    s = s + " sending to: " + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
                 else
                    s = s + " idle : " + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
             }
             if (tcpsockets[i].protocol == pDGRAMRX){
                 s = cHEADLINE;
                 s = s + "UDP Listen#" + intToString(i) + " ";
                 s = s + cITEM + OurDevices.designator[tcpsockets[i].myDevDesIndex]
                       +  " (" + tcpsockets[i].interface + ")"
                       + " on my port:" + intToString(tcpsockets[i].portnum);
                 if (tcpsockets[i].newsockfd >=0)
                    s = s + " sending to: " + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
                 else
                    s = s + " idle : " + tcpsockets[i].hostaddress + ":" + intToString(tcpsockets[i].hostport);
             }
         s = s + " in/out/conn/q " + LongToString(tcpsockets[i].msg_in)+ "/" + LongToString(tcpsockets[i].msg_out)
               + "/" + intToString(tcpsockets[i].connects) + "/" + intToString(tcpsockets[i].MsgQout.size());
         ssout << s <<  cNONE  << endl;
         }
       }
    }
    ssout << cNONE  << endl;  // always end with this

    ResultStr = ssout.str();
    return true;

};


// TCPECHO socket:int  ON/OFF
bool CommandLine::cmdSerial(void){

    // Global Statistics
    string s;
    int i;
    stringstream ssout;

    ssout << set_bold(true) << cHEADING <<  "List of serial ports:" << cNONE << set_bold(false) <<endl;

    for (i=0; i< MAX_TTY; i++){
       if (COMport[i].handle >= 0){
         ssout << cHEADLINE << "Serial #" << i << " Interface:" << cITEM  << COMport[i].devicename
         << cHEADLINE<< " baud:" << cITEM  << COMport[i].baudrate << " "  << COMport[i].parity << COMport[i].databits << COMport[i].stopbits
         << cHEADLINE << " bytes in/out:" << cITEM  << COMport[i].bytes_in << "/" << COMport[i].bytes_out;
         if (COMport[i].CTSin())
             ssout << " CTS: Y  ";
         else
             ssout << " CTS: N  ";
         if (COMport[i].DSRin())
             ssout << " DSR: Y";
         else
             ssout << " DSR: N";
         ssout <<  cNONE << endl;
       }

    }
    ssout << cNONE  << endl;  // always end with this
    ResultStr = ssout.str();
    return true;

};


bool  CommandLine::DoNothing(void){
    ResultStr = "";
    return true;
}

// SHUTDOWN command.  Terminates the program
bool  CommandLine::cmdExit(void){

    string user_input_hashed;
    stringstream ss;

    cmd_password_hashed = dtSC->LookupData("cmdpass",fld_param1);
    user_input_hashed = b64_encode((const unsigned char*)SX[2].c_str(), SX[2].size());
    //ChangeConfigSetting(fld_varname,"cmdpass", fld_param1, cmd_password_hashed);

    if (cmd_password_hashed.size() < 2 || (cmd_password_hashed == user_input_hashed)){
        ShutDownApplication = true;
        return true;
    }else{
        ss << "Error.  Cannot shutdown. Invalid password." << endl;
    }

    ResultStr = ss.str();

    return false;
}


bool  CommandLine::cmdExitCLI(void){

    MyCLI.MySocket.DisconnectClient();
    ResultStr = "";

    return true;
}

bool CommandLine::cmdStatistics(void){
    stringstream ssout;
    ResultStr = "";

    if (SX[2].size() == 0){
        // Global Statistics
        ssout  << StatisticsString() << endl;
        ssout << cNONE  << endl;  // always end with this
        ResultStr = ssout.str();
        return true;
    }

    
    return false;
};

string StatisticsString(void){

    std::stringstream ss;

    ss  << set_bold(true) << cHEADING << sayGENSTATS << cNONE << set_bold(false) << endl;
    ss << cHEADLINE << "Messages in (WMX)          " << cITEM  <<  longToString(wmxcount_in)<<  cNONE << endl;
    ss << cHEADLINE << "Messages in (PRAVE)        " << cITEM  <<  longToString(pravecount)<<  cNONE << endl;
    ss << cHEADLINE << "Messages in (Cigorn)       " << cITEM  <<  longToString(Cigorncount_in)<<  cNONE << endl;
    ss << cHEADLINE << "Messages in (XML)          " << cITEM  <<  longToString(xmlcount_in)<<  cNONE << endl;
    ss << cHEADLINE << "Messages in (NMEA)         " << cITEM  <<  longToString(nmeacount)<<  cNONE << endl;
    ss << cHEADLINE << "Routed messages            " << cITEM  <<  longToString(routecount)<<  cNONE << endl;
    ss << cHEADLINE << "Discarded duplicates       " << cITEM  <<  longToString(DataRouter.duplicatecount)<<  cNONE << endl;
    ss << cHEADLINE << "Cigorn site messages out   " << cITEM  <<  longToString(Cigorncount_out)<<  cNONE << endl;
    ss << cHEADLINE << "Cigorn site messages in    " << cITEM  <<  longToString(Cigorncount_in)<<  cNONE << endl;
    ss << cHEADLINE << "Number of Gateways heard   " << cITEM  <<  longToString(GateWays.size())<<  cNONE << endl;
    ss << cHEADLINE << "Failed eth delivery        " << cITEM  <<  longToString(FailedSockOut) <<  cNONE << endl;
    ss << cHEADLINE << "Failed tty delivery        " << cITEM  <<  longToString(FailedTTYOut) <<  cNONE << endl;
    ss << cHEADLINE << "Time of last database sync " << cITEM  <<  myDB.lastUpdate <<  cNONE << endl;
    ss << cHEADLINE << "Number of database syncs   " << cITEM  <<  intToString(myDB.NumOfUpdates) <<  cNONE << endl;
    ss << cHEADLINE << "Number of rows added       " << cITEM  <<  intToString(myDB.rowsadded) <<  cNONE << endl;
    ss << cHEADLINE << "Number of rows deleted     " << cITEM  <<  intToString(myDB.rowsdeleted) <<  cNONE << endl;
    ss << cHEADLINE << "Inbound message queue      " << cITEM  <<  intToString(qMSGin.size()) <<  cNONE << endl;
    ss << cHEADLINE << "Ave. route time (WMX)      " << cITEM  <<  doubleToString(AverageQ(dlyWMX),6) << "sec" <<  cNONE << endl;
    ss << cHEADLINE << "Ave. route time (PRAVE)    " << cITEM  <<  doubleToString(AverageQ(dlyPRAVE),6) << "sec" <<  cNONE << endl;
    ss << cHEADLINE << "Ave. route time (Cigorn)   " << cITEM  <<  doubleToString(AverageQ(dlyCIGORN),6) << "sec" << cNONE << endl;
    ss << cHEADLINE << "Main process speed         " << cITEM  <<  doubleToString((double)getMainLoopSpeed()/1000, 3) << "kHz" << endl;
    ss << cHEADLINE << "Com  process speed         " << cITEM  <<  doubleToString((double)getCommLoopSpeed()/1000, 3) << "kHz" << endl;
    ss << cHEADLINE << "ETH  process speed         " << cITEM  <<  doubleToString((double)getTCPLoopSpeed()/1000, 3) << "kHz"<<  cNONE << endl;
    ss << cHEADLINE << "OS Signal: Hangup          " << cITEM  <<  intToString(sigHupCount) << endl;
    ss << cHEADLINE << "OS Signal: Broken Pipe     " << cITEM  <<  intToString(sigPipeCount) << endl;
    return ss.str();

}

bool CommandLine::cmdVer(void){

    // display the version
    string s;
    stringstream ssout;
    ssout << GetVersionNum() << endl;
    ResultStr = ssout.str();
    return true;

};


bool CommandLine::cmdGateway(void){

    // display the version
    string s;
    stringstream ssout;

    cout << "lll" << SX[2] << "---" << SX[3] << endl;
    if ((SX[2] == "SETNAME") && (SX[3].size() > 3)){
        Me.MyName = SX[3];
        return true;
    }
    if (((SX[2] == "CHIEF") || (SX[2] == "MASTER")) && (SX[3] == "YES")){
        Me.IsChief = true;  // this is a master gateway
        return true;
    }

    if (SX[2].size() == 0){
        ssout << "Gateway name:    " << Me.MyName << endl;
    }
    ResultStr = ssout.str();
    return false;

};

// RESTART command  
bool CommandLine::cmdReboot(void){

    // restart the gateway
    string s;
    stringstream ssout;
    string user_input_hashed;

    cmd_password_hashed = dtSC->LookupData("cmdpass",fld_param1);
    user_input_hashed = b64_encode((const unsigned char*)SX[3].c_str(), SX[3].size());
    //ChangeConfigSetting(fld_varname,"cmdpass", fld_param1, cmd_password_hashed);


    if (SX[2] == "NOW"){
        if ((cmd_password_hashed.size() < 2) || (cmd_password_hashed == user_input_hashed)){
            RestartApplication = true;
            return true;
        }else{
            ssout << "Error.  Cannot restart. Invalid password." << endl;
        }
    }

    ResultStr = ssout.str();
    return false;

};

// RESET command
bool CommandLine::cmdReset(void){

    // reset a connection
    string s;
    stringstream ssout;
    bool retval = false;
    
    if (SX[2] == "STATS"){
        // user wants to reset all the communication statistics
        ResetCounters();   // reset the message counters  
        OurDevices.ResetStatistics();
        return true;
    }

    if ((si[2] > 0) && (StringIsInteger(SX[2]))){
        // User wants to close a port
        retval = ResetSocket(si[2]);
        if (retval)
            ssout << "Socket reset OK" << endl;
        else
            ssout << "Socket failed reset.  Port:" << si[2] << endl;
    }

    if (( SX[2].size() > 0) && (retval == false)){
        // User wants to close a device designator
        retval = ResetSocket(SX[2]);
        
        retval = retval || ResetSerial(SX[2]);
        
        if (retval)
            ssout << "Socket reset OK. DevDes:" << SX[2] << endl;
        else
            ssout << "Socket failed reset.  DeviceDes:" << SX[2] << endl;

    }

     ResultStr = ssout.str();
     return retval;

};

/**
 * Pause command. Pauses incoming or outgoing data on a connection for the
 * specified amount of time.
 * @return True if at least one stream was paused.
 */
bool CommandLine::cmdPause(void){
    bool pauseInput = false;
    bool pauseOutput = false;
    int pauseTime;
    
    if(SX[2] == "IN"){
        pauseInput = true;
    }else if(SX[2] == "OUT"){
        pauseOutput = true;
    }else if(SX[2] == "INOUT"){
        pauseInput = true;
        pauseOutput = true;
    }else{
        ResultStr = "Direction must be one of {IN|OUT|INOUT}\n";
        return false;
    }
    
    if(!StringIsInteger(SX[4]) || si[4] <= 0){
        ResultStr = "Please specify a valid pause time\n";
        return false;
    }else{
        pauseTime = si[4];
    }
    
    double unpauseTime = TimeNow() + pauseTime / 1000;
    
    if(DataRouter.pauseStreams(sx[3], pauseInput, pauseOutput, unpauseTime) > 0){
        return true;
    }else{
        return false;
    }
}


/**
 * Unpause command. Unpauses incoming or outgoing data on a connection.
 * @return True if at least one stream was unpaused.
 */
bool CommandLine::cmdUnpause(void){
    bool unpauseInput = false;
    bool unpauseOutput = false;
    
    if(SX[2] == "IN"){
        unpauseInput = true;
    }else if(SX[2] == "OUT"){
        unpauseOutput = true;
    }else if(SX[2] == "INOUT"){
        unpauseInput = true;
        unpauseOutput = true;
    }else{
        ResultStr = "Direction must be one of {IN|OUT|INOUT}\n";
        return false;
    }
    
    if(DataRouter.unpauseStreams(sx[3], unpauseInput, unpauseOutput) > 0){
        return true;
    }else{
        return false;
    }
}

/**
 * Queuereport command. Gives queue status for device designators matching a pattern
 * @return True if at least one stream was unpaused.
 */
bool CommandLine::cmdQueuereport(void){
    stringstream ssout;
    bool headerOutput = false;
    
    string devDesFilter = sx[2];
    if(devDesFilter == ""){
        devDesFilter = "*";
    }
    
    // Find any matching sockets
    for(int i = 0; i < MAXSOCKETS; i++){
        int devIndex = tcpsockets[i].myDevDesIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                if(!headerOutput){
                    headerOutput = true;
                    ssout << "Device,Queue" << endl;
                }
                ssout << OurDevices.designator[devIndex] << "," << 
                    tcpsockets[i].MsgQout.size() << endl;
            }
        }
    }
    
    // Find any matching serial ports
    for(int i = 0; i < MAX_TTY; i++){
        int devIndex = COMport[i].myDevIndex;
        if(devIndex >= 0 && devIndex < MAXDEVDES){
            if(WildCardMatch(devDesFilter, OurDevices.designator[devIndex])){
                if(!headerOutput){
                    headerOutput = true;
                    ssout << "Device,Queue" << endl;
                }
                ssout << OurDevices.designator[devIndex] << "," << 
                    COMport[i].MsgQout.size() << endl;
            }
        }
    }
    
    if(!headerOutput){
        ResultStr = "No matching devices";
    }else{
        ResultStr = ssout.str();
    }
    
    return headerOutput;
}

bool CommandLine::cmdReload(void){
    // restart the gateway
    string s;
    stringstream ssout;

    if ((SX[2] == "ROUTES") || (SX[2] == "ALL")){
        // re-load the data tables
        myDB.LoadTable(dtRT);            // load the table data from the database. Also loads field definitions
        BuildRouteTable();               // rebuild the route table
        Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
        return true;
    }

    if ((SX[2] == "ETHDEVDES")|| (SX[2] == "ALL")){
        // re-load the data tables
        myDB.LoadTable(dtEDD);                 // load the table data from the database. Also loads field definitions
        OurDevices.ClearEth();
        OurDevices.LoadEthDevDesTable(dtEDD);  // read in the eth device designator table
        Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
        Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
        return true;
    }

    if ((SX[2] == "TTYDEVDES")|| (SX[2] == "ALL")){
        // re-load the data tables
        myDB.LoadTable(dtTDD);                 // load the table data from the database. Also loads field definitions
        OurDevices.ClearTty();
        OurDevices.LoadTtyDevDesTable(dtTDD);  // read in the eth device designator table
        Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
       return true;
    }

    if ((SX[2] == "WNAT")|| (SX[2] == "ALL")){
        // re-load the data tables
        myDB.LoadTable(dtWNAT);                // load the table data from the database. Also loads field definitions
        BuildWNATtable();
        Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
        return true;
    }

    if ((SX[2] == "DEVDES")|| (SX[2] == "ALL")){
        // re-load the data tables
        reloadEDDTable = true;
        return true;
    }
    return false;
};


string CommandLine::GetNextCommand(void){
    string s = "";
    stringstream ssout;

    {
    cigorn::PlatformLockGuard lock(cmdqlock);

    if (!qCLIin.empty()) {
        s = qCLIin.front();
        qCLIin.pop();
    }
}

    return s;

};


// add a route to the message route table
bool CommandLine::cmdRoute(void){

    string srcif = sx[3];
    string dstif = sx[4];
    string idlimit = sx[5];
    string s;
    int lowID;
    int upID;
    stringstream ssout;

    int Format = fmtINVALID;

    if (SX[2].size() == 0){
        // User wants to see the route table
        ssout << "Route table: " << endl;
        s = DataRouter.RouteTableToText();
        ssout << s << endl;
        ResultStr = ssout.str();
        return true;
    }

    // See what type of format we are going to be using
    if (SX[2] == fmtWMXtxt){
        Format = fmtWMX;
    }
    // See what type of format we are going to be using
    if (SX[2] == fmtPRAVEtxt){
        Format = fmtPRAVE;
    }
    if (SX[2] == fmtESRI_CSV1txt){
        Format = fmtESRI_CSV1;
    }

    // See what type of format we are going to be using
    if (SX[2] == fmtALLtxt){
        Format = fmtPRAVE; // TODO: Does this make sense for Louisville- they want ESRI?
    }

    // verify the format of the command was OK
    if ((srcif.size() < 1) || (dstif.size() < 1) || (Format == fmtINVALID)){
        ssout << "Invalid command format " << SX[2] << endl;
        ResultStr = ssout.str();
        return false;    // no parameters
    }

    // find the lower and upper IDs
    if (ParseDoublet(idlimit, lowID, upID) == false){
        // There are no ID limits set on this route.
        lowID = -1;
        upID = INT_MAX;
    }

    if(DataRouter.AddRoute( Format, srcif, dstif, lowID, upID ) == true){
        ssout << "Route #" << DataRouter.RouteCount() << ". Route " << DataRouter.ProtocolName(Format) << " on " << srcif << " to " << dstif;
        if (lowID >= 0)
            ssout << " " << lowID << "-" << upID;
        ssout << endl;
    };

    ResultStr = ssout.str();
    return true;

};

// Set/read the radio channel management parameters
bool CommandLine::cmdSet(void){

    stringstream ssout;
    ResultStr = "";
    string pass_new_hashed;
    string pass_old_hashed;
    bool  OkToChange;

    if (SX[2] == "CMDPASS"){
        // Did he enter the old password first???
        cmd_password_hashed = dtSC->LookupData("cmdpass",fld_param1);
        pass_new_hashed = b64_encode((const unsigned char*)SX[4].c_str(), SX[3].size());
        pass_old_hashed = b64_encode((const unsigned char*)SX[3].c_str(), SX[3].size());
        OkToChange = (cmd_password_hashed.size() < 2) || (pass_old_hashed == cmd_password_hashed);

        if (OkToChange){
           cmd_password_hashed = b64_encode((const unsigned char*)SX[4].c_str(), SX[4].size());
           cout << cmd_password_hashed << endl;
           // dtWD->StoreMSupdate(idx, ColumnName, ColumnVal);
           ChangeConfigSetting(fld_varname,"cmdpass", fld_param1, cmd_password_hashed);
        }
    }

    if (SX[2] == "EPOCH"){
        if (SX[3].size() > 0){
           return SiteManager.NewTdmaParms((double)si[3], SiteManager.slottime);
        }
        else{
           ssout << SiteManager.EpochTime << endl;
           ResultStr = ssout.str();
           return true;
        }
    }

    if (SX[2] == "EPOCH"){
        if (SX[3].size() > 0){
           return SiteManager.NewTdmaParms((double)si[3], SiteManager.slottime);
        }
        else{
           ssout << SiteManager.EpochTime << endl;
	   
           ResultStr = ssout.str();
           return true;
        }
    }

    if (SX[2] == "SLOTTIME"){
        if (SX[3].size() > 0){
           SiteManager.slottime = StringToDouble(SX[3]);
           return SiteManager.NewTdmaParms(SiteManager.EpochTime, SiteManager.slottime);
        }
        else{
           ssout << SiteManager.slottime << endl;
           ResultStr = ssout.str();
           return true;
        }
    }

    // MAXCONTROL
    if (SX[2] == "MAXCONTROL"){
        if (SX[3].size() > 0){
           SiteManager.MaxControlChan = si[3];
        }
        else{
           ssout << SiteManager.MaxControlChan << endl;
        }
        ResultStr = ssout.str();
        return true;
    }

    if (SX[2] == "CONTROL"){
        if (SX[3].size() > 0){
           SiteManager.ControlChan = si[3];
        }
        else{
           ssout << SiteManager.ControlChan << endl;
        }
        ResultStr = ssout.str();
        return true;
    }


    if (SX[2] == "ACTIVE"){
        if (SX[3] == "TRUE"){
            Me.IsActive = true;  // put us on-line
            return true;
        }

        if (SX[3] == "FALSE"){
            Me.IsActive = false;  // take it off-line
            return true;
        }
        if (SX[3].size() == 0){
            ssout << BoolToString(Me.IsActive);
            ResultStr = ssout.str();
            return true;
        }
    }

    // ForceTakeOver
    if (SX[2] == "TAKEOVER"){
        if (SX[3] == "TRUE"){
            ForceTakeOver = true;  // takover as the active gateway, shutting down the primary
            return true;
        }
        if (SX[3] == "FALSE"){
            ForceTakeOver = false;  // take it off-line
            return true;
        }
    }
    // Generic SET handler
    if (SX[2].size() > 0)
    {
        ConfigParameter(
            SX[2],
            sx[3],
            sx[4],
            sx[5],
            sx[6],
            sx[7]);

        return true;
    }

    ResultStr = ssout.str();
    return true;
}

// set the debug message level
bool CommandLine::cmdMessage(void){
    
    stringstream ssout;
    ResultStr = ""; 
  
    if (SX[2].size() == 0){
        // Read the debug message level 
        switch(messagelevel){
            case MSG_NONE:
                ssout << "Status messages off. " << endl;
                break;
            case MSG_STATUS:
                ssout << "Status messages on. Level M1" << endl;
                break;
            case MSG_DEBUG:
                ssout << "Status messages on. Debug level M2" << endl;
                break;
            default:
                ssout << "Status messages level unknown." << endl;
                break;
        }
        ResultStr = ssout.str();
        return true;
    }else{
        // See if the user is setting the status message level
        if (SX[2] == "M1"){
            ssout << "Status messages on." << endl;
            ResultStr = ssout.str();
            messagelevel = MSG_STATUS;
            return true;
        }
        if (SX[2] == "ON"){
            ssout << "Status messages on." << endl;
            ResultStr = ssout.str();
            messagelevel = MSG_STATUS;
            return true;
        }
        if (SX[2] == "M2"){
            ssout << "Status messages on, level 2." << endl;
            messagelevel = MSG_DEBUG;
            ResultStr = ssout.str();
            return true;
        }
        if (SX[2] == "M0"){
            ssout << "Status messages off." << endl;
            ResultStr = ssout.str();
            messagelevel =  MSG_NONE;
            return true;
        }
        if (SX[2] == "OFF"){
            ssout << "Status messages off." << endl;
            ResultStr = ssout.str();
            messagelevel =  MSG_NONE;
            return true;
        }
    }

     ResultStr = "";
     return true;
}

// Set/read the radio channel management parameters
bool CommandLine::cmdRadio(void){ 

    ResultStr = "";
    string s = "";
    string t = "";
    string p = "";
    routeEntry re;
    string color;
    int sockindex;
    double activity;

     stringstream ss;
     htmlformatter fm;
     int i = 1;
     bool done = false;

    
    if (SX[2].size() == 0){

        // Loop through the devicedesignator list and show the dd conected to a radio
        ss << set_bold(true) << cHEADING <<  "Radio Channel Statistics:" << cNONE << set_bold(false) <<endl;
        ss << cHEADING <<  "Device Des. Chan# Interface           Msg In/Out            Activity " << cNONE << set_bold(false) <<endl;

        for (i = 0; i < MAXDEVDES ; i++){
           if ((OurDevices.devicetypes[i] == dDataModem) || (OurDevices.devicetypes[i] == dWMXmodem)){
              // This is a valid radio modem (base station)
              p="";
              s="";
              if (OurDevices.IsTTY(i)){
                 if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                     p = LongToString(COMport[OurDevices.getBinding(i)].msg_in) + "/" + LongToString(COMport[OurDevices.getBinding(i)].msg_out);
                     activity = COMport[OurDevices.getBinding(i)].ActivityTime();
                     if (activity >= 0)
                         s = doubleToString(activity, 2);
                     else
                         s = "--";
                     ss << cHEADLINE << FixedRight(OurDevices.designator[i], 10) << cITEM
                        << FixedRight(OurDevices.channels[i], 5)
                        << FixedRight(OurDevices.interfaces[i], 10)
                        << FixedRight(p, 20) << FixedRight(s, 20)
                        <<  cNONE << endl;
                     ResultStr = ss.str();
                 }
              }

                if ((StringLeft(OurDevices.interfaces[i], 3) == "eth") && (OurDevices.getBinding(i) > 0)){
                    sockindex = OurDevices.getBinding(i);
                    if (sockindex < MAXSOCKETS){
                        p = LongToString(tcpsockets[sockindex].msg_in) + "/" + LongToString(tcpsockets[sockindex].msg_out);
                        activity = tcpsockets[sockindex].ActivityTime();
                        if (activity >= 0)
                            s = doubleToString(activity, 2);
                        else
                            s = "--";

                        t = OurDevices.getHealth(i);
                        // if (tcpsockets[socknum].connected == true)

                        ss << cHEADLINE << FixedRight(OurDevices.designator[i],10) << cITEM
                           << FixedRight(OurDevices.channels[i], 5)
                           << FixedRight(OurDevices.interfaces[i], 10)
                           << FixedRight(p, 20) << FixedRight(s, 20)
                           <<  cNONE << endl;
                       ResultStr = ss.str();
                    }
                }
         }
    }
    return true;
 }
    return false;

}


