/* 
 * File:   health.cpp
 * Author: john
 * 
 * Created on August 31, 2010, 8:43 PM
 */

#include "health.h"
#include "ascii.h"
#include <stdio.h>
#include <iostream>
#include "functions.h"
#include "DeviceList.h"
#include "htmlformatter.h"

#define WATCHDOGMSG "STATUSOKAY"

// When we create this, assume all is healthy
// true=health  false=bad or broke

health::health() {
    LogDirectory = true;
    int portnum;
    tcpnet MySocket;  // the socket object to communicate to the smtp mail server with
    string webdata;   // the last data to come into our web port


}

health::health(const health& orig) {
}

health::~health() {
}

void health::ConfigureMonitor(int PortNumber){
    string s;

    LogDirectory = true;
    MySocket.portnum = PortNumber;
    MySocket.myDevDesIndex = 0;      // no device designator for this socket
    MySocket.index = 0;              // no indeex either.
    MySocket.init_tries = 0;
    MySocket.interface = "eth0";     // always use eth0
    MySocket.protocol = pServer;     // TCP/IP socket server
    MySocket.myDevType = dTerminal;
    MySocket.initialize_socket();

    //cout << "Health socket fd = " << MySocket.sockfd << endl;
}

bool health::ProcessMonitor(void){

    static bool NewMonitorConnection = false;

    // See if the socket is initalized. If it is, run it.
    if (MySocket.sockfd >= 0 ){
        MySocket.tcp_socket();    // always call this when connected to process the socket
    }

    if (MySocket.connected){
        // We are connected to remote thing
        if (NewMonitorConnection == false){
            //we have not yet sent the watchdog message
            cout << "FILE: " <<  __FILE__ << ", LINE: " << __LINE__ << endl;
            MySocket.sendbytes(WATCHDOGMSG);
            NewMonitorConnection = true;
        }
    }else{
        // We are not connected
        if (NewMonitorConnection == true){
            // We just discopnnected.
            cout << "Disconnecting... FILE: " <<  __FILE__ << ", LINE: " << __LINE__ << endl;

        }
    }

    if ((MySocket.connected)== false)
        NewMonitorConnection = false;


    return true;
}




