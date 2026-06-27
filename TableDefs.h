/* 
 * File:   TableDefs.h
 * Author: john
 *
 * Created on September 20, 2010, 9:31 PM
 */

#ifndef _TABLEDEFS_H
#define	_TABLEDEFS_H

using namespace std;

// database related constants

// The Wireless Device dta table
#define WDEVICE        "wdevice"      // The SQL table name for our table

#define wdeviceINDEX   "ID"           // the index field in the database
#define fld_ID         "ID"
#define fld_system     "system"         // The system numebr this WD belongs to
#define fld_enabled    "enabled"        // Is this guy enabled? (allowed to use the system)
#define fld_countTo    "countTo"        // Number of messages to this WD
#define fld_countFm    "countFm"        // Number of messages from this WD
#define fld_countFmD   "countFmD"       // Daily
#define fld_countToD   "countToD"       // daily
#define fld_tmLastMsg  "tmLastMsg"      // time of the last Data message from this WD
#define fld_rssi       "rssi"
#define fld_site       "site"           // text. the site this WD is currently on
#define fld_serialn    "serialno"       // his serial number
#define fld_model      "model"          // model number index
#define fld_w_slotnum  "w_slotnum"      // The slot num thei WD is using
#define fld_w_channel  "w_channel"      // The radio channel this WD is currently on
#define fld_w_state    "w_state"        // The communication state for the WD
#define fld_w_txrate   "w_txrate"       // The TX rate we set for this WD's GPS position reporting
#define fld_life       "w_life"         // How long this WD is allowed to be on the system
#define fld_operation  "w_operation"    // The mode of operation for this WD (active, sleeping, standby...)
#define fld_monchan    "w_monchan"      // The channel the WD is currently monitoring
#define fld_gpstxchan  "w_gpstxchan"           // The channel the WD was told to use to send GPS data on
#define fld_tmRegistration "tmRegistration"    // The time/date that this WD registered on the site

// The Route Table table
#define RoutesTable    "routes"      // The SQL table name for our table

#define RTindex        "RID"       // the index field in the database
#define fld_RID        "RID"
#define fld_Source     "Source"
#define fld_Dest       "Dest"
#define fld_LowID      "LowID"
#define fld_UpID       "UpID"
#define fld_Protocl    "Protocol"

// The Protocol table
#define ProtocolsTable "protocols"       // The SQL table name for our table

#define PTindex        "protoname"       // the index field in the database
#define fld_protoname  "protoname"
#define fld_soh        "soh"
#define fld_eot        "eot"
#define fld_delim      "delim"
#define fld_ascii      "ascii"
#define fld_maxlen     "maxlen"
#define fld_header     "header"
#define fld_timeout    "timeout"         // end-of-message timeout in mS.  -1 no time out

// The variable settings table  (holds configuration settings for this gateway)
#define SiteConfig     "siteconfig"
#define fld_varname    "varname"
#define fld_param1     "p1"
#define fld_param2     "p2"
#define fld_param3     "p3"
#define fld_param4     "p4"
#define fld_param5     "p5"
#define fld_idx        "idx"   // the key field. auto-numbered


// The ethernet device designator table
#define EthDevDesTable        "ethdevdes"

#define fld_designator      "designator"
#define fld_device          "device"
#define fld_interface       "interface"
#define fld_port            "port"
#define fld_ipadd           "ipadd"
#define fld_comment         "comment"
#define fld_protocol        "protocol"
#define fld_parm1           "parm1"
#define fld_channel         "channel"
#define fld_pcount          "pcount"

// The tty device designator table
#define TtyDevDesTable        "ttydevdes"
#define fld_t_designator      "designator"
#define fld_t_device          "device"
#define fld_t_interface       "interface"
#define fld_t_settings        "settings"
#define fld_t_baudrate        "baudrate"
#define fld_t_comment         "comment"
#define fld_t_protocol        "protocol"
#define fld_t_parm1           "parm1"
#define fld_t_channel         "channel"

// The Wireless NAT table
#define WNATTable             "wnat"
#define fld_wnat_devdes       "designator"
#define fld_wnat_baseid       "baseid"
#define fld_wnat_portcount    "portcount"
#define fld_wnat_comment      "comment"
#define fld_wnat_defaultdes   "defaultdes"

#define PagerDBTable          "pagers"
#define pagerNumberColumn     "pagerNumber" // Primary key
#define pageDataTypeColumn    "pageDataType"
#define capCodeColumn         "capCode"
#define otaProtocolColumn     "otaProtocol"
#define isGroupColumn         "isGroup"
#define isActiveColumn        "isActive"

#endif	/* _TABLEDEFS_H */

