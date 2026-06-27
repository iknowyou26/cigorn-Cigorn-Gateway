/* 
 * File:   SocketThread.h
 * Author: john
 *
 * Created on December 21, 2010, 6:43 AM
 */

#ifndef SOCKETTHREAD_H
#define	SOCKETTHREAD_H

#include "dataparser.h"
#include <netdb.h>


struct EthernetInterface{
    string interface;
    string ipaddress;
};


typedef map<int, EthernetInterface> IPaddList;

extern bool SocketThreadRunning;
extern bool RunSocketThread;

// Prototype the functions
void* threadTCPserver(void*);
int getIPv4(const char *, char *);
void GetMyIPaddressList(IPaddList&);
int get_iface_list(struct ifconf *);
string AddressListString(void);
bool InterfaceExists(string);
int getIPv4(string);
string GetInterfaceIPaddress(string);
int GetSocketIndex(int );
extern IPaddList ipaddresses;   // a list of all of the ethernet interfaces and their IP addresses.
bool ResetSocket(int);
bool ResetSocket(string);
extern bool RunSocketThread;
extern bool deconstructSockets;

#endif	/* SOCKETTHREAD_H */

