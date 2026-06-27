/* 
 * File:   ourstructures.h
 * Author: john
 *
 * Created on September 10, 2010, 9:43 PM
 */

#ifndef _OURSTRUCTURES_H
#define	_OURSTRUCTURES_H
#include <string>

using namespace std;

//#define MAX_DATA        9999

enum GateRoleType{Primary, Standby};

// Describes a gateway. It could be me, or another one in the system
struct Gateway{
    string MyName;           // The name of this gateway in the system. Must be unique
    bool IsChief;            // True if this is the ONE master/cheif gateway in the system, or a backuo-to the Chief
    bool IsActive;           // True if this gateway is actively running/routing
    GateRoleType gaterole;   // Primary is the main gateway.  The primary gateway's backup is called teh Standby.
    string IPadd;            // The IP Address of this gateway
    int DevDes;              // The device designator index we talk to this gaeway on.
    int dbPushInterval;      // How often this gateway updates its database tables
    int DBmodifyflag;        // the time in seconds when we modified the database
    string PrimaryName;      // the Name of the primary gateway if we are a standby
    string PrimaryDB;        // The name of the database to mirror in the primary box if we are a standby
    string PublicKey;        // The RSA Public Key
    int MessagesCountIn;     // how many messages came in from this gateway
};

struct WDupdate{
    int ID;
    int System;
    bool Enabled;
};


#endif	/* _OURSTRUCTURES_H */

