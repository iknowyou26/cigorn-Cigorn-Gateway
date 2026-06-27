/* 
 * File:   procinfo.cpp
 * Reac the process info from the linux proc file to get machine information
 * Author: john
 * 
 * Created on January 10, 2011, 8:39 PM
 */

#include "procinfo.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "functions.h"
#include <arpa/inet.h>    // Utilities for networking

procinfo::procinfo() {
}

procinfo::procinfo(const procinfo& orig) {
}

procinfo::~procinfo() {
}


void procinfo::ReadMyPorts(void){

    string s;
    string v;
    char fn[1000] = "/proc/net/tcp";
    int i = 0;
    int x;
    PortConn pc;

    ifstream inFile(fn);

    if (!inFile) {
        //elog.store("Cannot read /proc/net/tcp file.");
        return; // terminate with error
    }

    // read each line
    while (getline(inFile , s))   {
        i = StringToInt(GetSubString(s, 1));
        x = HexToInt(GetSubString(s, 2));  // The local IP address
        pc.MyIP4 = IntToIP4(x);
        pc.MyPort = HexToInt(GetSubString(s, 3));

        x = HexToInt(GetSubString(s, 4));  // The local IP address
        pc.RemAddress = IntToIP4(x);
        pc.RemPort = HexToInt(GetSubString(s, 5));

        MyPortInfo[i] = pc;
        i++;
    }

    inFile.close();

}


void procinfo::ReadProcFile(string file){

    string s = "/proc/" + file;
    string v;
    char fn[1000];
    int i = 0;
    Fields mf;

    to_cstring(fn, s,1000);  // create the file name
    ifstream inFile(fn);

    if (!inFile) {
        //elog.store("Cannot read /proc/net/tcp file.");
        return; // terminate with error
    }

    // read each line in the proc file,
    while (getline(inFile , s))   {
        mf.Field1 = GetSubString(s, 1);
        mf.Field2 = GetSubString(s, 2);
        mf.Field3 = GetSubString(s, 3);
        mf.Field4 = GetSubString(s, 4);
        MyFields[i] = mf;
        i++;
    }

    inFile.close();

}

string procinfo::GetKeepAliveSettings(void){

    string s = "/proc/sys/net/ipv4/tcp_keepalive_time";
    string retval = "";

    char fn[1000];
    int i = 0;
    Fields mf;

    to_cstring(fn, s,1000);  // create the file name
    ifstream inFile(fn);

    if (!inFile) {
        return "No file:" + s; // terminate with error
    }
    // read each line in the proc file,
    getline(inFile , s);
    retval = "Time = " + s;
    inFile.close();

    s = "/proc/sys/net/ipv4/tcp_keepalive_intvl";
    to_cstring(fn, s,1000);  // create the file name
    
    inFile.open(fn );

    if (!inFile) {
        return retval; // terminate with error
    }
    // read each line in the proc file,
    getline(inFile , s);
    retval = retval + "  Interval = " + s;
    inFile.close();

    s = "/proc/sys/net/ipv4/tcp_keepalive_probes";
    to_cstring(fn, s,1000);  // create the file name
    inFile.open(fn);
    if (!inFile) {
        return retval; // terminate with error
    }
    // read each line in the proc file,
    getline(inFile , s);
    retval = retval + "  Probes = " + s;
    inFile.close();

    return retval;
}




