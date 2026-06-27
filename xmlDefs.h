/* 
 * File:   xmlDefs.h
 * Author: john
 *
 * Created on October 17, 2010, 12:18 AM
 */

#ifndef _XMLDEFS_H
#define	_XMLDEFS_H

// *****************************************
// XML elemet difinitions
// *****************************************
// Roots
#define xmlCIGORN           "CigornIS"     // inter-site Cigorn message
#define xmlSECUREQRY        "CigornSQ"     // a secure query
#define xmlSECUREREPLY      "CigornSR"     // a secure reply
#define xmlCOMMAND         "CigornCMD"    // Commands

// Elements
#define xmlDestination      "Destination"
#define xmlSoftVersion      "SWversion"
#define xmlQueries          "QueryItems"
#define xmlCommands         "CommandItems"

#define xmlPublicKey        "PublicKey"
#define xmlQuerieItem       "SQItem"
#define xmlSQResponses      "SQResponse"
#define xmlCommandItem      "CMDItem"

#define xmlWirelessDevices  "WirelessDevices"
#define xmlWDdevice         "WD"
#define xmlWDenabled        "enabled"
#define xmlWDid             "ID"
#define xmlWDsystem         "system"
#define xmlTimeStamp        "TimeStamp"            // The time & date a message is created
#define xmlSiteProperties   "SiteProperties"
#define xmlSiteStatistics   "SiteStatistics"
#define xmlTableUpdate      "TableUpdate"
#define xmlRowUpdate        "RowUpdate"
#define xmlTableNameAttrib  "tablename"
#define xmlSocketCount      "SocketCount"   // number of connected sockets
#define xmlHrsSinceTtyIn    "TtyTimeIn"     // hourse since we got a tty message in
#define xmlHrsSinceEthIn    "EthTimeIn"
#define xmlMsgInCount       "MsgInCount"
#define xmlMsgOutCount      "MsgOutCount"
#define xmlIndexAttrib      "index"
#define xmlTableColumn      "Col"
#define xmlSiteName         "SiteName"
#define xmlSourceSite       "SourceSite"
#define xmlNameAttrib       "name"
#define xmlIsMaster         "Master"
#define xmlRole             "Role"
#define xmlRolePrimary      "Primary"
#define xmlRoleBackup       "Backup"
#define xmlIsActive         "Active"
#define xmlDBmodified       "DBmodified"
#define xmlTime             "time"
#define xmlName             "name"
#define xmlDate             "date"
#define xmlIDrange          "IDrange"
#define xmlFirstID          "first"
#define xmlLastID           "last"

#define SR_REQ_DBCREDENTIALS  "DBcredentials"
#define SR_REQ_DESTINATION    "SentToSite"
#define SR_RPLY_DBPASSWORD  "SRDBpass"
#define RPLY_DBUSER         "DBuser"
#define RPLY_DBhost         "DBhost"
#define RPLY_DBname         "DBname"
#define RPLY_DBIPaddress    "DBIPadd"
#define RPLY_DBmodflag      "DBmodflag"

#define COMMAND_BE_INACTIVE "BeInactiveNow"
#define COMMAND_BE_ACTIVE   "BeActiveNow"


struct XMLvector{
    std::string NodeName;
    std::string NodeValue;
};

typedef std::vector<XMLvector> XMLnodeList;

// The information that we transfer between sites to identify gateways.
struct CigornSite{
    bool ready;                  // set true when the parser finishes loading this
    bool valid;                  // the structure loaded OK.
    std::string description;
    time_t timein;
    std::string sitetime;
    std::string sitedate;
    int ver_major;
    int ver_minor;
    int ver_build;
    std::string sitename;
    bool ischief;
    bool isactive;
    int socketcount;
    int msgincount;
    int msgoutcount;
    int DBmodFlag;
};

// The information that we transfer between sites to identify gateways.
struct CigornIntersite{
    bool ready;                  // set true when the parser finishes loading this
    time_t timein;
    std::string sitetime;
    std::string sitedate;
    std::string sitename;
    std::string MessageType;
    XMLnodeList XMLnodes;
};

#endif	/* _XMLDEFS_H */

