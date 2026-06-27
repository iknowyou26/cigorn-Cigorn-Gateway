/*
 * File:   settings.cpp
 * Author: John
 *
 * Created on August 27, 2012, 9:05 PM
 *
 * Routines related to gateway clustering, synchronization, hot-standby, backup, and switchover.
 * Communications with the other gateways is handled in the CommThread.cpp *threadComm( void *ptr ) routine.
 */
#include <iostream>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "Cigorn.h"
#include "xmlFunctions.h"
#include "xmlDefs.h"
#include "CommThread.h" 
#include "DeviceList.h"
#include "sync-roles.h"
#include "mainsubs1.h"
#include "network.h"

using namespace std;

// Global variables exposed in .h
// Local Variables
stringstream ss;  // for debug
StbySyncState MySyncState = Reset;
StbySyncState LastSyncState = Reset;
time_t TimeOfLastPrimaryDBSync = time(NULL) - 20 * SecondsInAyear;
time_t TimePrimaryHealthOK = time(NULL);                // start out assuming the primary gateway is healthy now

CigornSite NewSiteMessage;
string StandbySiteName = "";  // if we get a message from the standby site, this is his name

int LinkUpAttempts = 0;
safestring MyXMLresponseToQuery;

string PrimaryDBuser = "";
string PrimaryDBpass = "";
string PrimaryDBhost = "";
string PrimaryDBname = "";
string PrimaryDBipadd = "";
string PrimaryGWipadd = "";             // The IP address of the other gateway we will sync to, if we are a standby
int DBmodifyflag = 0;
int LastDBmodFlag = 0;
int hotcutovertime = DEFAULT_CUTOVER_TM;          // number of seconds we go detecting a failed primary before we cut over
int PrimaryTestInterval = DEF_CLUSTER_INTERVAL;   // Seconds between messages between Primary/Standby gateways

bool PrimaryCredentialsOK = false;      // set true once we get good credentials
bool InSyncWithPrimary = false;         //set true when we get in sync
int SuccessfulSyncs = 0;
int SecondsSincePrimaryOK = -1;         // How many secons since we determinged the Primary is OK
bool ForceTakeOver = false;             // set true if we want to force this gateway to take over a primary
int  QueriesInCount = 0;
int MsgCountFromStandby = 0;

// External databases and data tables
database  DBx;               // create an instance of a database interface to hold the mirror of the primary
datatable *dtWDx;            // Our Wireless Devices on the network
datatable *dtRTx;            // Our data routes. Definition of what type of message from where gets routed to who.
datatable *dtSCx;            // SiteConfiguration table.
datatable *dtEDDx;           // Ethernet Device designators
datatable *dtTDDx;           // TTY Device designators
datatable *dtWNATx;          // Wireless NAT table

/*  The Cluster task state machine.  It goes like this  
 *  
 *  Reset
 *      Initialize the machine
 * 
 *
 *  Socket to communicate with the primary/standby gateway is CIGSOCKET  5443
 *
 */

// ***********************************************************************************************
// Called from main() loop about 1000/second
// Tasks releated to synchronizing to/from another gateway
// Note: Site Identifiers are asynchonously sent by the communications thread at some preset interval
// ***********************************************************************************************
void DoClusterTasks(void){
  time_t nowtime = time(NULL);
  static time_t StateTimer = time(NULL);
  static time_t RequestTime = time(NULL);    // the time a request is made to something
  static time_t NewStateTime = time(NULL);
  static time_t lastMessageTime = time(NULL);
  static int TimeInStateS = 0;
  static int HealthCheckTimer = 0;
  static int XMLreadycount = 0;
  static int DBloginAttempts = 0;
  static int DBloginFailures = 0;
  static int CredentialReqFails = 0;
  static unsigned int state_loops = 0;            // inc every state machine call
  static int PriFailDetectCount = 0;     // once we detect the primary is failed, and we want to cut over, count this pass through the machine
  int i = -1;
  string s;
  std::stringstream sr;


  // See if we need to send a response to some query.
  if (MyXMLresponseToQuery.ready == true){
      // We built a response to someone else's query.  Go send it.
      CoutM1(sr) << "Responding to Cigorn query." << endl;
      //cout << MyXMLresponseToQuery.thestring << endl;
      //SendCigornXML(MyXMLresponseToQuery.thestring);
      SendClusterText(MyXMLresponseToQuery.thestring);
      MyXMLresponseToQuery.clear();
      MyCLI.Display(&sr); // send the text to the console output
      sr.str("");                   // clear the buffer
  }
  
  

  if (StateTimer == nowtime)
      return;    // Only do sync tasks once a second


  StateTimer = time(NULL);  // restart the timer that determines how often the machine is executed

  // Se if we changed states last tick
  if (LastSyncState != MySyncState){
      LastSyncState = MySyncState; // remmeber when we change states
      NewStateTime = time(NULL);
  }

  TimeInStateS = time(NULL) - NewStateTime;

  HealthCheckTimer++;
  if (HealthCheckTimer >= StandbyHealthTime){
      // check the health of the primary gateway.
      HealthCheckTimer = 0;
  }

  state_loops++;

  // Keep a running parameter of how long its been since we know the Primary was OK
  SecondsSincePrimaryOK = time(NULL) - TimePrimaryHealthOK;


  if (Me.gaterole == Standby){
     // ***************************************************************
     // We are a Standby Gateway.  Do tasks to sync to the primary
     // ***************************************************************

      // See if a inter-site message came in from some other gateway.
      if (NewSiteMessage.valid){
          // A new message came in from some gateway.  See if it is from the primary
          //CoutM2(sr) << "Cigorn site message from:" << NewSiteMessage.sitename ;
          GateWays[NewSiteMessage.sitename].MessagesCountIn++;  // count messages in
          if (IsSameText(NewSiteMessage.sitename, Me.PrimaryName) == true){
              // New message NOT from the primary gateway that we are to sync to.
              CoutM2(sr) << "(my Primary)";
              TimePrimaryHealthOK = time(NULL);
              // This is an inter-site message from my Primary gateway
              if (NewSiteMessage.DBmodFlag != LastDBmodFlag){
                  // The Primary DB has updated a table and we need to re-sync to it
                  LastDBmodFlag = NewSiteMessage.DBmodFlag;
                  CoutM2(sr) <<  "{Cluster13} Primary tables changed. Must resync.";
                  InSyncWithPrimary = false;    // we are out of date
              }

          }

          ClearSiteMessage(NewSiteMessage);
      }


      switch (MySyncState){
          case Reset:
              // run once at program reset-startup
              CoutM1(sr) <<  "{Cluster0} Restarting state machine." << endl;
              MyCLI.Display(&ss);  // send the text to the console output
              MySyncState = Startup;
              MyXMLresponseToQuery.clear();  // clear our response buffer
              DBloginAttempts = 0;
              state_loops = 0;
              //dtRTx = dtRT;
              //dtEDDx = dtEDD;
              dtTDDx = dtTDD;
              dtWNATx = dtWNAT;
              // clear out the mirrored dB tables we will get from the Primary gateway
              dtRTx = new datatable(RoutesTable, fld_RID);      // Create an instance of the routes
              dtRTx->AutoAddRows = DBx.AutoAddRows;             // use the default autoadd setting for this table
              dtSCx = new datatable(SiteConfig, fld_varname);      // Create an instance of the routes
              dtSCx->AutoAddRows = DBx.AutoAddRows;             // use the default autoadd setting for this table
              dtEDDx = new datatable(EthDevDesTable, fld_designator);
              dtEDDx->AutoAddRows = DBx.AutoAddRows;             // use the default autoadd setting for this table
              dtTDDx = new datatable(TtyDevDesTable, fld_designator);
              dtTDDx->AutoAddRows = DBx.AutoAddRows;             // use the default autoadd setting for this table
              dtWNATx = new datatable(WNATTable , fld_designator);
              dtWNATx->AutoAddRows = DBx.AutoAddRows;             // use the default autoadd setting for this table
              InSyncWithPrimary = false;
              PriFailDetectCount = 0;
              TimePrimaryHealthOK = time(NULL);                   // assume it is OK now.
              CredentialReqFails = 0;
              break;
          case Startup:
              // Here when the state machine restarts due to communication issues or switching back to Standby.
              MySyncState = FindPrimary;
              ClearSiteMessage(NewSiteMessage);   // start be resetting the new message buffer
              RequestTime = time(NULL);           // go request the primary gateways public key
              LinkUpAttempts++;
              DBloginAttempts = 0;
              state_loops = 0;
              PriFailDetectCount = 0;
              break;
          case FindPrimary:
              // Stay here while we wait to hear from the Primary gateway
              i = GetPrimaryIndex(Me.PrimaryName);
              if (i >= 0 ){
                  // we have heard from the primary gateway
                  MySyncState = RequestCredentials;  // we heard our Primary.  Get his credentials.
              }else{

              }
              // MySyncState = RequestCredentials;  // we heard our Primary.  Get his credentials.

              if (StateTimer > (RequestTime + TIME_FOR_RESPONSE)){
                   // No answer back from the primary gateway
                   MySyncState = CantContactPrimary;  // go see if we should take over as Active gateway
              }
              break;
          case RequestCredentials:
              // ask for the credentials from the primary Gateway        fm.ToTable("Seconds since Primary verified",  DateTimeString(TimeOfLastPrimaryDBSync) );   // TimeOfLastSync

              CoutM1(sr) << "{Cluster1} Requesting credentials";
              MyCLI.Display(&sr);  // send the text to the console output
              s = MakeSecureRequestXML(SR_REQ_DBCREDENTIALS, Me.PrimaryName );
              //cout << "Text:" << s << endl;
              //SendCigornXML(s);
              SendPrimarySomeText(s);
              MySyncState = WaitForCredentials;
              RequestTime = time(NULL);           // restart the responsetimer
              break;
          case WaitForCredentials:
   
              if (PrimaryCredentialsOK){
                  // We got valid credentials  cHEADING
                  CoutM1(sr) <<  cBOLDITEM << "{Cluster4} Primary credentials OK from " << Me.PrimaryName
                             << " DB:" << PrimaryDBname << cNONE;
                  MyCLI.Display(&ss);  // send the text to the console output
                  MySyncState = ReceivedCredentials;
                  RequestTime = time(NULL);           // restart the responsetimer
                  Me.PrimaryDB = PrimaryDBname;
                  LastDBmodFlag = DBmodifyflag;       // remember the version of the Primary DB tables we are retrieving
                  DBloginAttempts = 0;
                  CredentialReqFails = 0;
              }
              if (StateTimer > (RequestTime + CREDENTIAL_TIMEOUT)){
                   // No answer back from the primary gateway
                   MySyncState = Startup;
                   CredentialReqFails++;
                   if (CredentialReqFails > CREDENTIAL_FAIL_MAX){
                       MySyncState =  WaitForPrimaryToFail;
                       CoutM2(sr) <<  "{Cluster3} Failed to get credentials from Primary." << endl;
                   }
              }
              break;
          case ReceivedCredentials:
              MySyncState = ConnenctToDB;
              break;
          case ConnenctToDB:
              // We got all we need to login to the Primary Gateway
              // Initialize the connection to the external Primary gateway database
              if(DBx.ConnectionOK == false){
                  // The connection is not working, so go re-start it
                  if ((DBloginAttempts < 2) || ((state_loops % 5) == 0)){
                      DBx.connect(PrimaryDBipadd, PrimaryDBname, PrimaryDBuser, PrimaryDBpass);
                      //DBx.connect("127.0.0.1", PrimaryDBname, PrimaryDBuser, PrimaryDBpass);
                      DBloginAttempts++;
                  }
              }
              if (DBx.ConnectionOK){
                  MySyncState = Synchronizing;  // go read the remote database
                  CoutM1(sr) <<  cBOLDITEM << "{Cluster5} Connected to Primary DB:" << PrimaryDBname << cNONE;
                  DBloginAttempts = 0 ;
              }
              if (StateTimer > (RequestTime + RETRY_DBX_TIME)){
                   // We're here long enough
                   MySyncState = FailedToAccessDB;  // restart trying to connect to Primary database
                   DBloginAttempts = 0;
              }
              break;
          case FailedToAccessDB:
              // We can't access the Postgres database on the primary server. Restart connection
              DBloginAttempts = 0;
              DBloginFailures++;
              // Disconnect from DB
              DBx.close();  // close and free memory
              if (DBloginFailures < 10)
                  elog.store("Error 531. Failed to connect to Primary DataBase:" + PrimaryDBname + " on IP:" + PrimaryDBipadd);
              CoutM1(sr) <<  "Error 531. Failed to connect to Primary DataBase:" << PrimaryDBname << " on IP:" << PrimaryDBipadd;

              DBx.connect(PrimaryDBipadd, PrimaryDBname, PrimaryDBuser, PrimaryDBpass); // try once more
              if (DBx.ConnectionOK){
                  MySyncState = Synchronizing;  // go read the remote database
                  CoutM1(sr) <<  cBOLDITEM << "{Cluster7} Connected to Primary DB:" << PrimaryDBname << cNONE;
                  DBloginAttempts = 0 ;
              }else{
                   // No response. Go check if Primary is OK.
                   MySyncState = WaitForPrimaryToFail;
              }
              break;
          case   Synchronizing:
              // Get the table data from the Primary gateway's Database
              bool successful;
              successful = true;
              CoutM2(sr) <<  "Reading Primary gateway tables" << endl;

              // ***************************************
              // Get the tables from the Primary Gateway
              // ***************************************
              // Routing Table
              LoadTable(&DBx, dtRTx, RoutesTable);    // get the routing table from the Primary Gateway
              CoutM2(sr) <<  "         Table:" << RoutesTable << "  " << dtRTx->rows.size() << " rows." << endl;
              if (dtRTx->rows.size() == 0)
                  successful = false;
              
              // Site COnfiguratoin table
              if (LoadTable(&DBx, dtSCx, SiteConfig) < 0)    // get the routing table from the Primary Gateway
                  successful = false;
              CoutM2(sr) <<  "         Table:" << SiteConfig << "  " << dtSCx->rows.size() << " rows." << endl;

              if(LoadTable(&DBx, dtEDDx, EthDevDesTable) < 0)    // get the routing table from the Primary Gateway
                  successful = false;
              CoutM2(sr) <<  "         Table:" << EthDevDesTable << "  " << dtEDDx->rows.size() << " rows." << endl;

              if (LoadTable(&DBx, dtTDDx, TtyDevDesTable) < 0)   // get the routing table from the Primary Gateway
                  successful = false;
              CoutM2(sr) <<  "         Table:" << TtyDevDesTable << "  " << dtTDDx->rows.size() << " rows." << endl;

              if (LoadTable(&DBx, dtWNATx, WNATTable ) < 0)   // get the routing table from the Primary Gateway
                  successful = false;
              CoutM2(sr) <<  "         Table:" << WNATTable  << "  " << dtWNATx->rows.size() << " rows." << endl;

              if (successful){
                  // All the tables were read in properly
                  MySyncState = MergeTables;
                  InSyncWithPrimary = true;
                  SuccessfulSyncs++;
            }
              else{
                  MySyncState = FailedReadTables;
                  InSyncWithPrimary = false;
              }

              break;
          case FailedReadTables:
              DBloginFailures++;
              if (DBloginFailures < 10)
                  elog.store("Error 533. Failed to read Primary tables.");
              CoutM1(sr) <<  "Error 533. Failed to read Primary tables.";
              MySyncState = WaitForPrimaryToFail;   // Primary may be down.  Go see.
              DBx.close();  // close and free memory
              break;
          case MergeTables:
              CoutM2(sr) <<  "{Cluster8} Importing Tables from Primary.";
              TimeOfLastPrimaryDBSync = time(NULL);
              dtRT->Import(dtRTx);
              CoutM2(sr) <<  " routes,";
              dtEDD->Import(dtEDDx);
              CoutM2(sr) <<  " ethdevdes,";
              dtTDD->Import(dtTDDx);
              CoutM2(sr) <<  " ttydevdes,";
              dtWNAT->Import(dtWNATx);
              CoutM2(sr) <<  " wnat";
              MySyncState = StoreTables;
              break;
          case StoreTables:
              // Store/update the tables into our Postgres DB
              myDB.StoreTableToDB(dtRTx);
              myDB.StoreTableToDB(dtEDDx);
              myDB.StoreTableToDB(dtTDDx);
              myDB.StoreTableToDB(dtWNATx);
              CoutM1(sr) <<  cBOLDITEM << "{Cluster9} All tables Imported from Primary:" << Me.PrimaryName << cNONE;

              // Update the site configuration parameters that should be changed. Not all.
              MergeSiteConfig(dtSCx);
              
              // Send a signal to reload the relevant tables
              reloadEDDTable = true;

              MySyncState = ActivateTables;
              break;
          case ActivateTables:
              // Wait here until tables have been reloaded
              if(!reloadEDDTable){
                MySyncState = WaitForPrimaryToFail;
                BuildRouteTable();               // rebuild the route table
              }
              if (TimeInStateS > TIME_REBUILD_TABLES){
                   // We're here long enough. No response. Go check if Primary is OK.
                   MySyncState = WaitForPrimaryToFail;
              }
              break;
          case CantContactPrimary:
              // Come here when we tried but cannot contact the primary gateway's database 
              
              MySyncState = WaitForPrimaryToFail;
              break;
          case    WaitForPrimaryToFail:
              // *************************************************************************************************
              // Wait here for Primary to go down, or a change in a table. We are not active.
              // *************************************************************************************************

              // See if we have gone a long-time and not got a response from the Primary gateway.
              if ( SecondsSincePrimaryOK > hotcutovertime){
                  // Primary is unresponsive.  Take over as Active gateway
                  MySyncState = TakingOver;
                  CoutM2(sr) <<  "{Cluster20} Primary unresponsive.  Going Active";
              }else{
                  PriFailDetectCount = 0;   // count how many times we think we detected the Primary failing.
              }
              if (ForceTakeOver){
                  // Someone commanded us to take over as the Active gateway
                  MySyncState = TakingOver;
                  ForceTakeOver = false;
                  CoutM2(sr) <<  "{Cluster21} Commanded to to Active and take Primary off-line.";
              }
              if (!InSyncWithPrimary){
                  // We do not have a current copy of the Primary Database.  Periodically re-try to see if it comes up.
                  if (TimeInStateS > (hotcutovertime + RETRY_DBX_TIME)){
                      // We cant get ahold of the primary gateway to get credentials, but it probably has not failed.
                      MySyncState = Startup;   // Go restart and try to get in sync with primary
                      CoutM2(sr) <<  "{Cluster28} Primary DB changed.  Downloading new tables.";
                  }
              }
              break;
          case     TakingOver:
              // The the primary to Be Incactive Now
              s = MakePrimaryCommand(COMMAND_BE_INACTIVE, Me.PrimaryName );
              SendPrimarySomeText(s);     // Just in case it can hear us, tell it to shut up and go off-line
              MySyncState = TakeOverWait;
              break;
          case TakeOverWait:
              // Wait a little bit before we go online
              if (TimeInStateS > TAKEOVER_WAIT_TIME){
                   // We're here long enough
                   MySyncState = RunActive;
                   GoActiveNow();           // Go active !!!!!!!!!!!!!!!
                   CoutM1(sr) <<  cBOLDITEM << " * * * * * * * * * * * * * * * * * * *" << cNONE << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
                   CoutM1(sr) <<  cBOLDITEM << "{Cluster22} Running as Active Gateway." << cNONE << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
                   CoutM1(sr) <<  cBOLDITEM << " * * * * * * * * * * * * * * * * * * *" << cNONE << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
                   s = "Primary gateway " + Me.PrimaryName + " failed. " + Me.MyName + " taking over as active gateway.";
                   if (emailnotice > 0)
                      SendNoticeEmail(&myEmail, s);
              }
              if ( SecondsSincePrimaryOK < hotcutovertime){
                   // Primary came back alive.  Go restart watching it. Maybe it was restarted.
                   MySyncState = Startup;
                   CoutM2(sr) <<  "{Cluster23} Primary came back up.  Leaving it alone.";
                  }
              break;
          case    RunActive:
              // We are Active taking over for the primary.
              if ((TimeInStateS % TAKEOVER_REMIND_TIME) == 1){
                  // Remind the primary, if he's listening, that he should be inactive.
                  s = MakePrimaryCommand(COMMAND_BE_INACTIVE, Me.PrimaryName );
                  SendPrimarySomeText(s);     // Just in case it can hear us, tell it to shut up and go off-line
              }
              if ( SecondsSincePrimaryOK < hotcutovertime){
                   // Primary came back alive.  Go restart watching it.
                   MySyncState = HandBackOver;
                   CoutM1(sr) <<  cBOLDITEM << " * * * * * * * * * * * * * * * * * * * * * * * * * * *" << cNONE << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
                   CoutM2(sr) <<  "{Cluster24} Primary back up.  Hand active responsibility back." << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
                   CoutM1(sr) <<  cBOLDITEM << " * * * * * * * * * * * * * * * * * * * * * * * * * * *" << cNONE << endl;
                   MyCLI.Display(&sr);  // send the text to the console output
              }
              break;
          case    HandBackOver:
              if (TimeInStateS > HANDOVER_DELAY_TIME){
                  MySyncState = Startup;
                  Me.IsActive = false;
                  s = MakePrimaryCommand(COMMAND_BE_ACTIVE, Me.PrimaryName );
                  SendPrimarySomeText(s);     // Just in case it can hear us, tell it to go active immediately
                  s = "Standby gateway " + Me.MyName + " going inactive. " + Me.PrimaryName + " taking over as active gateway.";
                  if (emailnotice > 0)
                      SendNoticeEmail(&myEmail, s);
              }
              break;
          default:
              MySyncState = Reset;  // bad error.  Never get here. 
              break;
      }
      
    }else if (Me.gaterole == Primary){

         // ************************************************************************************
         // We are a Primary Gateway.
         // ************************************************************************************
          TimePrimaryHealthOK = time(NULL);   // We are a primary, and we are OK.

          // See if a inter-site message came in from some other gateway.
          if ( NewSiteMessage.valid ){
              CoutM2(sr) <<  "{Cluster50} New Site message in from." << NewSiteMessage.sitename << " via" << NewSiteMessage.description << endl;
              string activeString = NewSiteMessage.isactive ? "active" : "inactive";
              CoutM2(sr) <<  "{Cluster51} " << NewSiteMessage.sitename << " is " << activeString << endl;
              
              MsgCountFromStandby++;
              // Are we inactive and the the Standby inactive?? If so, go active now.
              if ((Me.IsActive == false) && (!NewSiteMessage.isactive ) && IsSameText(NewSiteMessage.sitename, StandbySiteName)){
                  // The standby gateway is not active, so go active.
                  CoutM2(sr) << "{Cluster52} No active gateways. Assuming control.";

                  GoActiveNow();
              }

              ClearSiteMessage(NewSiteMessage);  // reset message buffer
              lastMessageTime = time(NULL); // For tracking how long since we've gotten a site message
          } else{
              // No message received

              if(Me.IsActive == false && time(NULL) - lastMessageTime > PRIMARY_TAKEOVER_REJECT_TIME){
                  // We've been forced off and nobody is talking to us. We're forcing back on
                  CoutM2(sr) << "{Cluster53} Assuming control. Standby is not communicating.";

                  s = "Primary gateway " + Me.MyName + " detected a secondary failure during standby takeover and has resumed control.";
                  if (emailnotice > 0)
                     SendNoticeEmail(&myEmail, s);

                  GoActiveNow();
              }
          }
    }

      // Se if we changed states last tick
      if (LastSyncState != MySyncState){
          LastSyncState = MySyncState; // remmeber when we change states
          NewStateTime = time(NULL);
      }

     // Now do a watchdog on the response string in case it is ignored.
     if (MyXMLresponseToQuery.ready){
          XMLreadycount++;
          if (XMLreadycount > 10){
            CoutM2(sr) << "{Cluster29} Failed to send response.";
            MyXMLresponseToQuery.clear();   // just clear it. Nothing is using it and it must get freed up
            XMLreadycount = 0;
          }
      }

    // Output the debug text if there is any
    if (sr.str().size() > 0){
        sr << endl;
        MyCLI.Display(&sr);  // send the text to the console output
        sr.str("");                   // clear the buffer
    }


};

// Go active right now.
void GoActiveNow(void){

     Me.IsActive = true;  // we must be active if the backup is not active.

}
// Return the index into the list of gateways we here that is for the Primary Gateway
int GetPrimaryIndex(string PrimGate){
    int i;
    for (i=0; i<GateWays.size(); i++){
        if (StringToUpper(trim(PrimGate)) == StringToUpper(trim(Me.PrimaryName)))
            return i;
    }
    return -1;

}

// This is called from a background communication thread!
// store all info in the NewSiteMessage structure
void InterSiteMessageIn(BinaryEntry& msg){
    Gateway Gw;
    CigornSite SiteMessage;
    CigornIntersite Cim;
    ss.str("");                        // clear the debug message buffer
    XMLnodeList ReplyNodes; // The XML nodes we will reply to a query with
    string CigMssgType = "";
    stringstream ssr;  // for debug

    ClearSiteMessage(SiteMessage);

    CigMssgType = GetCigornMessageType(msg);

    CoutM2(ssr) << "{Intersite}" << CigMssgType << " via port:" << msg.PortIn;
    if (CigMssgType == xmlCIGORN){
            ProcessInterSiteXML(msg, SiteMessage);   // parse the XML and return the data in the gateway data structure.
            if (SiteMessage.valid == true ){
                Gw.MyName = SiteMessage.sitename;
                Gw.IsChief = SiteMessage.ischief;
                Gw.IsActive = SiteMessage.isactive;
                Gw.DevDes = msg.SrcDevDesIndex;
                Gw.IPadd = OurDevices.getSourceIPaddress(msg.SrcDevDesIndex);
                Gw.dbPushInterval = -1;      // unknown
                GateWays[Gw.MyName] = Gw;
                if ((msg.PortIn == CIGSERVPORT) && (Me.gaterole == Primary))
                    StandbySiteName = SiteMessage.sitename;   // this was from our standby gateway
                NewSiteMessage = SiteMessage;  // copy the mesage contents over.
                CoutM2(ssr) << " from:" << SiteMessage.sitename << " time:"
                <<  SiteMessage.sitetime << "  " << SiteMessage.socketcount;
            }
    }

    if (CigMssgType == xmlSECUREQRY){
        // We just got a Secure Query in from someone wanting a paramter from us.
        if (MyXMLresponseToQuery.ready == false){
            // If the string buffer to hold our response in is available, then we can build up a response
            ProcessSecureXML(msg, Cim);                         // Parse the XML in into a structure
            BuildSecureReplies(Cim.XMLnodes, ReplyNodes, msg);  // Build a reply back to the site that sent it
            MyXMLresponseToQuery.store(MakeSecureResponseXML(ReplyNodes, Cim.sitename));    // Build a new XML message with the responses in it
            CoutM2(ssr) << " from:" << Cim.sitename  << "  #in:" << Cim.XMLnodes.size() << " #out" << ReplyNodes.size();
            //cout << MyXMLresponseToQuery.thestring << endl;
        }else{
            CoutM2(ssr) << " Fail. Response buffer in use." ;
            MyXMLresponseToQuery.clear();
        }

    }

    // xmlSECUREREPLY   is a reply to a query.  Probably from us.xmlSQResponses
    if (CigMssgType == xmlSECUREREPLY){
        // We just got a Secure Reply in from someone a request.
        ProcessSecureXML(msg, Cim);              // Parse the XML in into a structure
        ProcessResponse(Cim);                    // Move the values to our variables
        CoutM2(ssr) << " from:" << Cim.sitename << "  #in:" << Cim.XMLnodes.size();
    }

    if (CigMssgType == xmlCOMMAND){
        // We just got a Secure Query in from someone wanting a paramter from us.
        ProcessXMLcommand(msg, Cim);                         // Parse the XML in into a structure
        ProcessCommandsIn(Cim);                    // Move the values to our variables
        CoutM2(ssr) << " from:" << Cim.sitename
                    << "  #in:" << Cim.XMLnodes.size() << " via:" << OurDevices.getDevDes(msg.SrcDevDesIndex) <<  " #out" << ReplyNodes.size();
    }

    
    // Output the debug text if there is any
    if (ssr.str().size() > 0){
        ssr << endl;
        MyCLI.Display(&ssr);   // send the text to the console output
        ssr.str("");                   // clear the buffer
    }

}

//Process Commands that someone sent us. Usually the Standby Gateway
void ProcessCommandsIn(CigornIntersite Cm){
    int i=0;
    Gateway gw;

    if (Cm.sitename.size() == 0)
        return;

    //cout << GateWays.size() << "=S" << endl;
    gw = GateWays[Cm.sitename];   // Find the info for the gateway that sent this response

    if (gw.MyName.size() < 1){
        // new gateway.  We have not heard from this one yet...
        GateWays[Cm.sitename].MyName = Cm.sitename;
    }

   GateWays[Cm.sitename] = gw;

   if (Cm.XMLnodes.size() == 0 ){
       return;
   }

   // This must have been a command message
   if (Cm.MessageType != xmlCOMMAND)
       return;

   // Run throught the list of commands and execute them
   for (i=0; i< Cm.XMLnodes.size(); i++){
        // CoutM2(ss) << "{Response}" <<  Cm.XMLnodes[i].NodeName << endl;
        if (Cm.XMLnodes[i].NodeName == xmlCommandItem){
            // Database user name
            if (IsSameText(Cm.XMLnodes[i].NodeValue, COMMAND_BE_INACTIVE)){
                // Standby gatewqay took us off line
                Me.IsActive = false;
                CoutM2(ss) << cBOLDITEM << " - - - - - - - - - - - - - - - - - - - - - - - - - - " << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
                CoutM1(ss) << cBOLDITEM << "Forced INACTIVE by" << Cm.sitename << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
                CoutM2(ss) << cBOLDITEM << " - - - - - - - - - - - - - - - - - - - - - - - - - - " << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
            }
            if (IsSameText(Cm.XMLnodes[i].NodeValue, COMMAND_BE_ACTIVE)){
                // Standby gatewqay wants us active now.
                GoActiveNow();
                CoutM2(ss) << cBOLDITEM << " - - - - - - - - - - - - - - - - - - - - - - - - - - " << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
                CoutM1(ss) << cBOLDITEM << "Forced ACTIVE by" << Cm.sitename << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
                CoutM2(ss) << cBOLDITEM << " - - - - - - - - - - - - - - - - - - - - - - - - - - " << cNONE << endl;
                MyCLI.Display(&ss);  // send the text to the console output
            }
        }


    }

    // Output the debug text if there is any
    MyCLI.Display(&ss);  // send the text to the console output
    ss.str("");                   // clear the buffer

};


// Process the response to a request we made
void ProcessResponse(CigornIntersite Cm){
    int i=0;
    Gateway gw; 

    if (Cm.sitename.size() == 0)
        return;

    //cout << GateWays.size() << "=S" << endl;
    gw = GateWays[Cm.sitename];   // Find the info for the gateway that sent this response

    if (gw.MyName.size() < 1){
        // new gateway.  We have not heard from this one yet...
        GateWays[Cm.sitename].MyName = Cm.sitename;
    }

   GateWays[Cm.sitename] = gw;

   if (Cm.XMLnodes.size() == 0 ){
       return;
   }

   for (i=0; i< Cm.XMLnodes.size(); i++){
        // CoutM2(ss) << "{Response}" <<  Cm.XMLnodes[i].NodeName << endl;
        if (Cm.XMLnodes[i].NodeName == RPLY_DBUSER){
            // Database user name
            PrimaryDBuser = Cm.XMLnodes[i].NodeValue;
        }
        if (Cm.XMLnodes[i].NodeName == SR_RPLY_DBPASSWORD){
            // Database password. Encrypted
            PrimaryDBpass = Cm.XMLnodes[i].NodeValue;
        }
        if (Cm.XMLnodes[i].NodeName == RPLY_DBname){
            // Database name
            PrimaryDBname = Cm.XMLnodes[i].NodeValue;
        }
        if (Cm.XMLnodes[i].NodeName == RPLY_DBIPaddress){
            // Database IP add
            PrimaryDBipadd = Cm.XMLnodes[i].NodeValue;
        }
        if (Cm.XMLnodes[i].NodeName == RPLY_DBmodflag){
            // Database IP add
            DBmodifyflag = StringToInt(Cm.XMLnodes[i].NodeValue);
        }

        if (Cm.XMLnodes[i].NodeName == RPLY_DBhost){
            // Database IP add
            PrimaryDBhost = Cm.XMLnodes[i].NodeValue;
        }
        if ((PrimaryDBuser.size()>0) &&
            (PrimaryDBpass.size()>0) &&
            (PrimaryDBname.size()>0) &&
            (PrimaryDBipadd.size()>0) &&
            (PrimaryDBhost.size()>0)){
                PrimaryCredentialsOK = true;  // we got all the credentials.  OK to use them now
       }else{
       }
    }

    // Output the debug text if there is any
    if (ss.str().size() > 0){
        ss << endl;
        MyCLI.Display(&ss);  // send the text to the console output
        ss.str("");                   // clear the buffer
    }

};


// Take in a list of requests, and build-up a list of responses to the requests.
void  BuildSecureReplies(XMLnodeList& Cin, XMLnodeList& Replies, BinaryEntry& be){
    int i;
    in_addr_t ip = 0;
    in_addr_t mask = 0;
 
    XMLvector Anode;

    if (Cin.size() == 0){
        Replies.clear();
        return;
    }

    for (i=0; i < Cin.size(); i++){
        //cout << "Node**" << Cin[i].NodeName << " " << Cin[i].NodeValue << endl;
        if (Cin[i].NodeName == xmlQuerieItem){
            // Someone want a parameter from us, securley encrypted
            //cout << "Node++" << Cin[i].NodeName << " " << Cin[i].NodeValue << endl;
            if (Cin[i].NodeValue == SR_REQ_DBCREDENTIALS){
                // They want the database login credentials (dbHost, dbName, dbUser, dbPass)
                Anode.NodeName = SR_RPLY_DBPASSWORD;
                Anode.NodeValue = dbPass;
                Replies.insert(Replies.end(), Anode);  // add this response
                Anode.NodeName = RPLY_DBUSER;
                Anode.NodeValue = dbUser;
                Replies.insert(Replies.end(), Anode);  // add this response
                Anode.NodeName = RPLY_DBhost;
                Anode.NodeValue = dbHost;
                Replies.insert(Replies.end(), Anode);  // add this response
                Anode.NodeName = RPLY_DBname;
                Anode.NodeValue = dbName;
                Replies.insert(Replies.end(), Anode);  // add this response
                Anode.NodeName = RPLY_DBIPaddress;
                Anode.NodeValue = GetIP("eth0", &ip, &mask);
                Replies.insert(Replies.end(), Anode);  // add this response RPLY_IPaddress
                Anode.NodeName = RPLY_DBmodflag;
                Anode.NodeValue = intToString(Me.DBmodifyflag);
                Replies.insert(Replies.end(), Anode);  // add this response RPLY_IPaddress

            }
        }
    }

}

// Given a parameter name, return its value in a string
string GetCigornValue(string Pname){

    if (Pname == SR_REQ_DBCREDENTIALS){
       // dbHost, dbName, dbUser, dbPass
       return dbPass;
    }

    return "";

}

void  ClearSiteMessage(CigornSite& SM){
    SM.valid = false;      // set true once we get a good mesage in from another gateway
    SM.ready = false;
    SM.description = "";
    SM.ischief = false;
    SM.msgincount = 0;
    SM.msgoutcount = 0 ;
    SM.sitedate = "";
    SM.sitename = "";
    SM.socketcount = 0;
    SM.timein = time(NULL);
    SM.ver_build = 0;
    SM.ver_major = 0;
    SM.ver_minor = 0;
    SM.DBmodFlag = 0;
    SM.isactive = false;
}

// Run when this gateway's rile has changed.
void RoleUpdated(void){
    static bool first = true;

    if (first){
        // First time through this routine
        if (Me.IsActive == true){
            // We are active, turn on all of the sockets and communication channels

        }else{
            // We are off-line waiting. Make sure our sockets are closed and serial COMM off.
            
        }
    }
    first=false;
}

string GetRole(GateRoleType gr){
    if (gr == Primary)
        return "Primary";
    else
        return "Standby";
}

// Called in CommThread.cpp
// Call rapidly to do the inter-gateway standby/primary communication via a socket
void ClusterSockets(void){
   static time_t LastIdentifier = time(NULL);
   static time_t TimeNow = time(NULL);
   static bool ServerConnected = false;

   ss.str() = "";
    
    // Server for a Primary connection
    if (CigSocketIn.interface.size() == 0){
        CigSocketIn.CreateSocket("eth0", CIGSERVPORT, "SERVER", "", 0, true, dCigorn);
        CigSocketIn.description = "Cigorn Inter-gateway In";
    }


    // Client for a backup connection
    if (CigSocketOut.interface.size() == 0){
        CigSocketOut.CreateSocket("eth0", CIGSERVPORT, "CLIENT", PrimaryGWipadd, 0, true, dCigorn);
        CigSocketOut.description = "Cigorn Inter-gateway Out";
    }
    
   // Check the communications to/from standby/primary gateways
   CigSocketIn.tcp_socket();  // run the socket

   //if (CigSocketOut.sockfd >= 0)
   CigSocketOut.tcp_socket();  // run the socket

   if (ServerConnected != CigSocketIn.connected){
       // Connection state changed
       ServerConnected = CigSocketIn.connected;
       if (CigSocketIn.connected)
           CoutM2(ss) << "[cluster] Connected to client:" << CigSocketIn.ConnectedToIP << ":" << CigSocketIn.RemotePort
                << " on my port:" << CigSocketIn.portnum << endl;
       else
           CoutM2(ss) << "[cluster] Disconnected from client." << endl;
   }

    if (CigSocketIn.MyParser.rput > CigSocketIn.MyParser.rget){
          CoutM2(ss) << "[cluster] Msg from Standby In." << endl;

    }
    if (CigSocketOut.MyParser.rput > CigSocketOut.MyParser.rget){
          CoutM2(ss)  << "[cluster] Msg from Primary in." << endl;
    }

   // See if it is time to identify outselves to the Primary/Standby gateway
   if ((time(NULL) - LastIdentifier) > DEF_CLUSTER_INTERVAL){
        SendIdentifier();
        LastIdentifier = time(NULL);
   }


   // Output the debug text if there is any
   if (ss.str().size() > 0){
        ss << endl;
        MyCLI.Display(&ss);  // send the text to the console output
        ss.str("");                   // clear the buffer
   }

}

// Send and XML formatted site identifier to the other Primary/Standby gateway in our cluster.
void SendIdentifier(void){

    string s;
    stringstream ss;
 
    s = MakeSiteIdentifierXML();         // make our identification
    if (CigSocketIn.connected){
        CigSocketIn.sendbytes(s);
        //cout << s << endl;
        CoutM2(ss) << "Primary sent identifier to:" << CigSocketOut.hostaddress << endl;
        MyCLI.Display(&ss);
    }
    if (CigSocketOut.connected){
        CigSocketOut.sendbytes(s);
        //cout << s << endl;
        CoutM2(ss) << "Standby sent identifier to:" << CigSocketOut.hostaddress << endl;
        MyCLI.Display(&ss);
 }
ss.str("");                   // clear the buffer

}



// Send and XML formatted site identifier to the other gateway in our cluster.
void SendClusterText(string s){

    stringstream ss;
    ss.str()="";
    
    if (s.size() <= 0 )
        return;

    if (CigSocketIn.connected){
        CigSocketIn.sendbytes(s);
        CoutM2(ss) << "Primary sent text to:" << CigSocketOut.hostaddress << ":" << intToString(CigSocketOut.hostport) << endl;
        MyCLI.Display(&ss);
    }
    if (CigSocketOut.connected){
        CigSocketOut.sendbytes(s);
        CoutM2(ss) << "Standby sent text to:" << CigSocketOut.hostaddress << ":" << intToString(CigSocketOut.hostport) << endl;
        MyCLI.Display(&ss);
    }

    MyCLI.Display(&ss);  // send the text to the console output
    ss.str("");                   // clear the buffer
}



// Send and XML formatted site identifier to the other gateway in our cluster.
bool SendPrimarySomeText(string s){

    static int FailCount = 0;
    stringstream ss;
    ss.str()="";


    if (s.size() <= 0 )
        return false;

    //CoutM2(ss) << "Sending..." << endl;
    //MyCLI.Display(&ss);  // send the text to the console output

/*
         // See if we are connected
    if ((CigSocketOut.connected == false) && (FailCount >= 3)){
        // We are not connected to the Primary. Try to
        CoutM2(ss) << " Standby connect attempt to:" << CigSocketOut.hostaddress;
        CigSocketOut.DisconnectSocket();
        CoutM2(ss) << " Discon";
        CigSocketOut.ConnectSocket();
        CoutM2(ss) << " ;";
        if (CigSocketOut.connected == true){
            CoutM2(ss) << " fd=" << intToString(CigSocketOut.sockfd) << " OK" << endl;
        }
        else{
            CoutM2(ss) << " fd=" << intToString(CigSocketOut.sockfd) << " failed." << endl;
        }
    }

  */
    if (CigSocketOut.connected){
        CigSocketOut.sendbytes(s);
        CoutM2(ss) << " Standby sent message to:" << CigSocketOut.hostaddress << endl;
        MyCLI.Display(&ss);
        FailCount = 0;
        return true;
    }else{
        FailCount++;
        CoutM2(ss) << " Standby send failed. Not Connected to:" << CigSocketOut.hostaddress << endl;
    }

    MyCLI.Display(&ss);  // send the text to the console output
    ss.str("");                   // clear the buffer
    return false;

}

// merge the data from the Primary gateway siteconfig table into our site configuration
void MergeSiteConfig(datatable* dt){
    // For now, nothing to copy over. All site configuration items are unique.


};