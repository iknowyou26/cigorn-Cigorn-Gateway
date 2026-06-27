/* 
 * File:   sync-roles.h
 * Author: john
 *
 * Created on August 27, 2012, 8:47 PM
 */

#ifndef SYNC_ROLES_H
#define	SYNC_ROLES_H

#include "xmlDefs.h"

#define TIME_TO_SYNC_DB    12*60*60 // 12 hours to retrieve the data from the primary DB
#define RETRY_DBX_TIME         30   // If we can't connect to remote Database, wait and retry after this interval
#define TIME_FOR_RESPONSE       5   // 5 seconds to wait for an XML response to a request from Primary
#define CREDENTIAL_TIMEOUT      5   //
#define TIME_DB_FAIL_WAIT       5   // 5 seconds to wait for an XML response to a request from Primary
#define TIME_REBUILD_TABLES     5   // Time to wait while rebuilding tables
#define DEFAULT_CUTOVER_TM    120   // 2 minutes of detecting a failed primary before we cut over.
#define DEF_CLUSTER_INTERVAL   10   //  Number of seconds between Primary/Standby gateway communications
#define TAKEOVER_WAIT_TIME      3   // wait a few seconds for the Primary to go off-line before going active
#define TAKEOVER_REMIND_TIME   15   // every x seconds, tell the primary to go inactive
#define HANDOVER_DELAY_TIME    10   // after handing control back to Primary, wait this long before restarting health checks.
#define CREDENTIAL_FAIL_MAX     4   // Number of credential requests failed before we give up.
#define PRIMARY_TAKEOVER_REJECT_TIME 120 // Number of seconds before the primary will forcefully take back over because it hasn't heard from the secondary after being forced off

// Sync states to use when we are a standby gateway
enum StbySyncState{
    Reset,            // start-up conidition.  Nothing initialezed yet
    Startup,
    FindPrimary,
    Synchronizing,
    RequestCredentials,
    WaitForCredentials,
    FailedToAccessDB,
    ReceivedCredentials,
    ConnenctToDB,
    FailedReadTables,
    MergeTables,
    StoreTables,
    ActivateTables,
    CantContactPrimary,
    WaitForPrimaryToFail,
    TakingOver,
    TakeOverWait,
    RunActive,
    HandBackOver
};


void DoClusterTasks(void);
void InterSiteMessageIn(BinaryEntry&);
void RoleUpdated(void);
std::string GetRole(GateRoleType);
void  ClearSiteMessage(CigornSite& );
void  BuildSecureReplies(XMLnodeList&, XMLnodeList&, BinaryEntry&);
std::string GetCigornValue(std::string);
void ProcessResponse(CigornIntersite);
void ProcessCommandsIn(CigornIntersite);
int GetPrimaryIndex(std::string);
void ClusterSockets(void);
void SendIdentifier(void);
void SendClusterText(std::string );
bool SendPrimarySomeText(std::string);
void MergeSiteConfig(datatable*);
void GoActiveNow(void);

extern std::string PrimaryGWipadd;
extern std::string PrimaryDBipadd;
extern time_t TimeOfLastPrimaryDBSync;
extern bool InSyncWithPrimary;         //set true when we get in sync
extern int SuccessfulSyncs;
extern int hotcutovertime;             // number of seconds we go detecting a failed primary before we cut over
extern int PrimaryTestInterval;        // Seconds between messages between Primary/Standby gateways
extern bool ForceTakeOver;             // set true if we want to force this gateway to take over a primary
extern int SecondsSincePrimaryOK;      // How many secons since we determinged the Primary is OK
extern int MsgCountFromStandby;
extern StbySyncState MySyncState;

extern bool PrimaryDBconnection;

#endif	/* SYNC_ROLES_H */

