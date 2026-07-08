/* 
 * File:   mainsubs1.cpp
 * Author: john
 * 
 * Created on September 22, 2010, 5:58 AM
 */

#include "Cigorn.h"
#include "GlobalVar.h"
#include "TCPsocket.h"
#include "serialhandler.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include "emailer.h"
#include "CommandLine.h"
#include "CommThread.h"
#include <sys/resource.h>
#include "sync-roles.h"
#include "PagerTable.h"
#include "repositories/PagerRepository.h"
#include <vector>
#include "repositories/RouteRepository.h"
#include "adapters/RouteTableAdapter.h"
#include "PostgresDatabase.h"
#include "RepositoryManager.h"
#include "adapters/EthDeviceTableAdapter.h"
#include "adapters/WirelessNatTableAdapter.h"

using namespace std;

void SendStatusEmail(emailer* pmailer, string notice){
    std::stringstream ss;
    htmlformatter fm;
    emailcontent newemail;

    if (pmailer->Enabled() == false)
        return;
 
    newemail.to = pmailer->mailto;
    newemail.from = pmailer->mailfrom;
    newemail.subjectline = pmailer->subject + " Cigorn Gateway Status ";

    ss << tag_first << endl;
    ss << fm.TagOpen("html") << endl;
    ss << fm.TagOpen(tag_head) << endl;
    ss << fm.TagClose(tag_head) << endl;

    ss << fm.TagOpen(tag_body) << endl;

    ss << "Status report from Cigorn Gateway: " << Me.MyName << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << "Status Message sent on " << LocalDate() << " at " << LocalTime() << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << "My IP address: " << Me.IPadd << endl;
    ss << fm.TagOpen(tag_para);  // new line
    if (notice.size() > 0){
        ss << "Notice       : " << notice << endl;
        ss << fm.TagOpen(tag_para);  // new line
        ss << " " << endl;
        ss << fm.TagOpen(tag_para);  // new line
    }

    // Make a table with the information
    // dtPagers = new datatable(PagerDBTable, pagerNumberColumn);
    // dtPagers->AutoAddRows = false;
    // myDB.LoadTable(dtPagers);
    // dtPagers->parentdb = aDB;
    // The table data
    ss << BuildStatsHTML();    // make an HTML string of our statistics

    ss << fm.TagOpen(tag_linebreak);
    ss << fm.TagOpen(tag_linebreak);
    ss << fm.htmlformat("Error Log: ", tag_strong) << "  (" << elog.ErrorMessages.size() << " entries)" << endl;
    ss << fm.TagOpen(tag_linebreak);


    int i;
    // output the error log entries
    i=1;
    for (i=elog.ErrorMessages.size()-1; i>=0; i--){
       ss << i << "  " << elog.ErrorMessages[i] << endl;
       ss << fm.TagOpen(tag_linebreak);
    }
    ss << fm.TagOpen(tag_linebreak);

    ss << fm.TagClose(tag_body) << endl;
    ss << fm.TagClose("html") << endl;

    newemail.content = ss.str();
    pmailer->emails.push(newemail);  // put the new email on the queue to be ent out.

}


void SendNoticeEmail(emailer* pmailer, string notice){
    std::stringstream ss;
    htmlformatter fm;
    tabledata2 Table2;
    emailcontent newemail;

    newemail.to = pmailer->mailto;
    newemail.from = pmailer->mailfrom;
    newemail.subjectline = pmailer->subject + " Important Cigorn Gateway Notice! From " + Me.MyName;

    ss << tag_first << endl;
    ss << fm.TagOpen("html") << endl;
    ss << fm.TagOpen(tag_head) << endl;
    ss << fm.TagClose(tag_head) << endl;

    ss << fm.TagOpen(tag_body) << endl;

    ss << "Notice from Cigorn Gateway: " << Me.MyName << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << "Message sent on " << LocalDate() << " at " << LocalTime() << " " << LocalGMTdiff() << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << "My IP address: " << Me.IPadd << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << "Notice       : " << notice << endl;
    ss << fm.TagOpen(tag_para);  // new line
    ss << " " << endl;
    ss << fm.TagOpen(tag_para);  // new line

    // Make a table with the information
    ss << fm.TagOpen(tag_linebreak);

    ss << fm.TagClose(tag_body) << endl;
    ss << fm.TagClose("html") << endl;

    newemail.content = ss.str();
    pmailer->emails.push(newemail);  // put the new email on the queue to be sent out.

}



void ResetCounters(void){
    nmeacount = 0;
    pravecount = 0;
    wmxcount_in = 0;
    xmlcount_in = 0;
    Cigorncount_in = 0;
    Cigorncount_out = 0;
    routecount = 0;
    FailedSockOut = 0;
    FailedTTYOut = 0;
}

// Create a html string with our statistics in it
string BuildStatsHTML(void){

    std::stringstream ss;
    htmlformatter fm;
    //string s = ctime(&boot_time);

    fm.ToTable("Program started at", ctime(&boot_time));
    fm.ToTable("Running for ", DeltaTime(time(NULL), boot_time));
    fm.ToTable("Messages in (PRAVE)", longToString(pravecount));
    fm.ToTable("Messages in (XML) ", longToString(xmlcount_in));
    fm.ToTable("Messages in (NMEA)", longToString(nmeacount));
    fm.ToTable("Messages in (WMX)", longToString(wmxcount_in)); //
    fm.ToTable("Routed messages", longToString(routecount) );
    fm.ToTable("Discarded duplicates", longToString(DataRouter.duplicatecount));
    fm.ToTable("Cigorn inter-site messages out", longToString(Cigorncount_out));
    fm.ToTable("Cigorn inter-site messages in", longToString(Cigorncount_in));
    fm.ToTable("Number of gateways heard", longToString(GateWays.size()));
    fm.ToTable("Database updates stored", intToString(myDB.NumOfUpdates));
    fm.ToTable("Primary Gateway reads ", intToString(SuccessfulSyncs));
    fm.ToTable("Number of Radio Channels", intToString(OurDevices.getChannelCount()));
    fm.ToTable("Active Radio Channels", intToString(OurDevices.getActiveChannelCount()));
    fm.ToTable("Failed eth delivery", longToString(FailedSockOut));
    fm.ToTable("Failed tty delivery", longToString(FailedTTYOut));
    fm.ToTable("Time of last database sync", myDB.lastUpdate);
    fm.ToTable("Number of WDs added", intToString(myDB.rowsadded));
    fm.ToTable("Number of WDs deleted", intToString(myDB.rowsdeleted));
    fm.ToTable("main process speed",doubleToString((double)getMainLoopSpeed()/1000, 2) + "kHz");
    fm.ToTable("ttyS process speed", doubleToString((double)getCommLoopSpeed()/1000, 2) + "kHz");
    fm.ToTable("eth  process speed", doubleToString((double)getTCPLoopSpeed()/1000, 2) + "kHz");
    fm.ToTable("Ave. route time (WMX)", doubleToString(AverageQ(dlyWMX),6) + "sec");
    fm.ToTable("Ave. route time (PRAVE)", doubleToString(AverageQ(dlyPRAVE),6) + "sec");

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("General Statistics", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    return ss.str(); //  fixed width font

}

void send_reboot_email(emailer* pmailer){

    int i;
    std::stringstream ss;
    std::string s;
    htmlformatter fm;
    emailcontent newemail;

    newemail.to = "rl@raveon.com";
    newemail.from = "cigorn@ravtrack.com";
    newemail.subjectline = pmailer->subject + " System Rebooted ";

    ss << tag_first << endl;
    ss << fm.TagOpen("html") << endl;
    ss << fm.TagOpen(tag_head) << endl;
    ss << fm.TagClose(tag_head) << endl;

    // Begin Body
    ss << "<BODY LANG=\"en-US\" DIR=\"LTR\">";
    ss << fm.TagOpen(tag_para);  // new line
    ss << "Email notice from Cigorn Gateway: " << fm.htmlformat(Me.MyName,tag_bold);
    if (Me.IsChief)
       ss << fm.htmlformat("(Chief)", tag_strong) << endl;
    else
        ss << endl;
    
    ss << fm.TagOpen(tag_linebreak);
    ss << "Rebooted on " << LocalDate() << " at " << LocalTime() << endl;

    ss << fm.TagOpen(tag_linebreak);
    ss << "My IP address: " << Me.IPadd << endl;

    ss << fm.TagOpen(tag_linebreak);
    ss << "Software version: " << GetVersionText() << endl;

    ss << fm.TagOpen(tag_linebreak);
    ss << fm.TagOpen(tag_linebreak);
    ss << fm.htmlformat("Error Log: ", tag_strong) << "  (" << elog.ErrorMessages.size() << " entries)" << endl;
    ss << fm.TagOpen(tag_linebreak);


    vector <string>::iterator it;
    // output the error log entries
    i=1;
    for (i=0; i< elog.ErrorMessages.size(); i++){
       ss << i << "  " << elog.ErrorMessages[i] << endl;
       ss << fm.TagOpen(tag_linebreak);
    }

    ss << fm.TagOpen(tag_linebreak);


    ss << fm.TagClose(tag_para) << endl;
    ss << fm.TagClose(tag_body) << endl;
    ss << fm.TagClose("html") << endl;

    newemail.content = ss.str();
    pmailer->emails.push(newemail);  // put the new email on the queue to be ent out.

    mlog.store("Sent email to " + newemail.to + " subject:" + newemail.subjectline);
   
}

int BuildRouteTable(void)
{
    DataRouter.ClearAll();

    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        cout << "BuildRouteTable DB connect failed: "
             << db.LastError() << endl;
        return -1;
    }

    RouteRepository repo(&db);
    RouteTableAdapter adapter(&repo);

    if (!adapter.Load())
    {
        cout << "BuildRouteTable route load failed: "
             << db.LastError() << endl;
        return -1;
    }

    for (int i = 0; i < adapter.RowCount(); i++)
    {
        routeEntry NewEntry;

        NewEntry.srcDevDes = adapter.GetString(i, 1);
        NewEntry.dstDevDes = adapter.GetString(i, 2);
        NewEntry.lowerID = adapter.GetInt(i, 3);
        NewEntry.upperID = adapter.GetInt(i, 4);
        NewEntry.format = DataRouter.ToMsgFormat(adapter.GetString(i, 5));

        DataRouter.AddRoute(NewEntry);
    }

    return adapter.RowCount();
}
int BuildWNATtable(void)
{
    int baseid, portcount;
    string Designator = "";
    string comment = "";
    string DefDevDes = "";
    stringstream ss;

    WNAT.ClearAll();

    PostgresDatabase db;

    if (!db.Connect(myDB.LastConnInfo))
    {
        cout << "BuildWNATtable DB connect failed: "
             << db.LastError() << endl;
        return -1;
    }

    RepositoryManager repos(&db);
    WirelessNatTableAdapter adapter(&repos.WirelessNat());

    if (!adapter.Load())
    {
        cout << "BuildWNATtable WNAT load failed: "
             << db.LastError() << endl;
        return -1;
    }

    for (int i = 0; i < adapter.RowCount(); i++)
    {
        Designator = adapter.GetString(i, 0);
        baseid = adapter.GetInt(i, 1);
        DefDevDes = adapter.GetString(i, 2);
        portcount = adapter.GetInt(i, 3);
        comment = adapter.GetString(i, 4);

        if ((portcount > 0) && (Designator.size() > 0) && (baseid > 0))
        {
            WNAT.AddWNAT(Designator, portcount, baseid, DefDevDes, comment);
            CoutM1(ss) << "Read WNAT entry " << Designator << " IDs"
                       << baseid << "-" << (baseid + portcount) << endl;
        }
        else
        {
            ss << "Error 075. WNAT Entry Invalid. "
               << Designator << " IDs"
               << baseid << "-" << (baseid + portcount) << endl;
            elog.store(("Error 075. WNAT Entry Invalid" + Designator));
        }
    }

    if (ss.str().size() > 0)
    {
        MyCLI.OutputText(ss.str());
        ss.str("");
    }

    return adapter.RowCount();
}
int BuildPagerTable(void)
{
    int pagersLoaded = 0;

    Pagers.ClearAll();

    //PagerRepository repo(myDB.dbManager.Database());
    //std::vector<PagerTableEntry> entries;

    //if (!repo.LoadEntries(entries))
    //{
        //cout << "Failed to load pagers through PagerRepository." << endl;
        //return -1;
    //}
    RepositoryManager repos(myDB.dbManager.Database());

    std::vector<PagerTableEntry> entries;

   if (!repos.Pagers().LoadEntries(entries))
   {
    	cout << "Failed to load pagers through PagerRepository." << endl;
   	 return -1;
   }
    for (int i = 0; i < (int)entries.size(); i++)
    {
        Pagers.AddPager(entries[i]);
        pagersLoaded++;
    }

    return pagersLoaded;
}

// dwl is a list of WDs that need their records updated. Usually because we got an update via an XML document \
// from the Chief site.
void UpdateWDtable(WDupdateList& wdl, int A, int Z){
    int i;
    WDupdateList::iterator it;

    vector<int> idlist;
    vector<int>::iterator idit;

    vector<int> deletelist;

    // Start by making a list of all WDs in our current table from A to Z. Load it in a vector
    dtWD->BuildIndexList(idlist, A, Z);

    // Now make the list of WDs that should not be in our table
    // idlist is the list of WDs in our current table of WDs. Comapre it to the entries from the Chief
    for (idit = idlist.begin(); idit != idlist.end(); idit++){

    }

    // Now go through the list of updates (wdl) and remove any that exatally match our data so we don't need to change anything.
    for (it = wdl.begin(); it != wdl.end(); it++){
      //

    }



}

void CountMessage(int fmt){
    // Keep system statistics on messages in
    switch (fmt){
        case fmtPRAVE:
            // Raveon PRAVE message came in
            pravecount = pravecount +1;
            break;
        case fmtWMX:
            // Raveon WMX message came in
            wmxcount_in = wmxcount_in +1;
            break;
        case fmtNMEA:
            // Generic NMEA message, no IDS.
            nmeacount++;
            break;
        case fmtCigorn:
            // XML from a Cigorn site
            Cigorncount_in++;
            break;
        case fmtXML:
            // XML document
            xmlcount_in++;
            break;
    }

}

string StatisticsToHTML(void){
    htmlformatter fm;

    std::stringstream ss;
    ss << "<BODY LANG=\"en-US\" DIR=\"LTR\">";

    ss  << fm.htmlformat(sayGENSTATS, tag_bold ) << endl;
    ss << fm.TagOpen(tag_para);
    ss << fm.htmlformat("main process speed         ",tag_emphasis)  << doubleToString((double)getMainLoopSpeed()/1000, 2) << "kHz" << endl;
    ss << fm.TagClose(tag_para);
    //ss << cHEADLINE << "ttyS process speed         " << cITEM  << doubleToString((double)commloopspeed/1000, 2) << "kHz" << endl;
    //ss << cHEADLINE << "eth process speed          " << cITEM  <<  doubleToString((double)tcploopspeed/1000, 2) << "kHz"<<  cNONE << endl;
    //ss << cHEADLINE << "Ave. route time (WMX)      " << cITEM  <<  doubleToString(AverageQ(dlyWMX),6) << "sec" <<  cNONE << endl;
    //ss << cHEADLINE << "Ave. route time (PRAVE)    " << cITEM  <<  doubleToString(AverageQ(dlyPRAVE),6) << "sec" <<  cNONE << endl;
    //ss << cHEADLINE << "Ave. route time (Cigorn)   " << cITEM  <<  doubleToString(AverageQ(dlyCIGORN),6) << "sec" << cNONE << endl;
    //ss << cHEADLINE << "Messages in (WMX)          " << cITEM  <<  longToString(wmxcount_in)<<  cNONE << endl;
    //ss << cHEADLINE << "Messages in (PRAVE)        " << cITEM  <<  longToString(pravecount)<<  cNONE << endl;
    //ss << cHEADLINE << "Messages in (Cigorn)       " << cITEM  <<  longToString(Cigorncount_in)<<  cNONE << endl;
    //ss << cHEADLINE << "Messages in (XML)          " << cITEM  <<  longToString(xmlcount_in)<<  cNONE << endl;
    //ss << cHEADLINE << "Messages in (NMEA)         " << cITEM  <<  longToString(nmeacount)<<  cNONE << endl;
    //ss << cHEADLINE << "Routed messages            " << cITEM  <<  longToString(routecount)<<  cNONE << endl;
    //ss << cHEADLINE << "Discarded duplicates       " << cITEM  <<  longToString(DataRouter.duplicatecount)<<  cNONE << endl;
    //ss << cHEADLINE << "Cigorn site messages out   " << cITEM  <<  longToString(Cigorncount_out)<<  cNONE << endl;
    //ss << cHEADLINE << "Cigorn site messages in    " << cITEM  <<  longToString(Cigorncount_in)<<  cNONE << endl;
    //ss << cHEADLINE << "Failed eth delivery        " << cITEM  <<  longToString(FailedSockOut) <<  cNONE << endl;
    //ss << cHEADLINE << "Failed tty delivery        " << cITEM  <<  longToString(FailedTTYOut) <<  cNONE << endl;
    //ss << cHEADLINE << "Time of last database sync " << cITEM  <<  myDB.lastUpdate <<  cNONE << endl;
    //ss << cHEADLINE << "Number of database syncs   " << cITEM  <<  intToString(myDB.NumOfUpdates) <<  cNONE << endl;
    //ss << cHEADLINE << "Number of rows added       " << cITEM  <<  intToString(myDB.rowsadded) <<  cNONE << endl;
    //ss << cHEADLINE << "Number of rows deleted     " << cITEM  <<  intToString(myDB.rowsdeleted) <<  cNONE << endl;

    return ss.str();
}


int GetFileDescrptorLimit(void){
    int i;
    rlimit mylimit;

    if (getrlimit(RLIMIT_NOFILE, &mylimit)==0){
        i = mylimit.rlim_max;
        return i;
    }

    return -1;  // function failed

}

bool ChangeConfigSetting(const string& idxfld, const string&  idxval, const string& fieldname, const string&  newval){

    dtSC->ChangeField(idxfld, idxval, fieldname, newval);
    myDB.PushChangesToDB(dtSC);
    mlog.store("Configuration parameter " + idxval + " changed to " + newval);


    return true;
}


// Load the routing and designator tables from a particular database
bool LoadTablesFromDB(database* aDB){
       PostgresDatabase ethDb;

     if (ethDb.Connect(myDB.LastConnInfo))
    {
    RepositoryManager repos(&ethDb);
    EthDeviceTableAdapter ethAdapter(&repos.EthDevices());

    //if (ethAdapter.Load())
    //{
        //cout << "EthDeviceTableAdapter startup loaded "
             //<< ethAdapter.RowCount()
             //<< " ethernet device rows." << endl;
    //}
     
      }
       // create the data table objects that will hold the records from the SQL database
       dtWD = new datatable(WDEVICE, fld_ID);    // Create the table to hold info about our WDs. Index is ID.
       dtWD->AutoAddRows = myDB.AutoAddRows;     // use the default autoadd setting for this table
       aDB->LoadTable(dtWD);
       dtWD->parentdb = aDB;
       dtWD->SetRW(fld_enabled, true);           // Set this field to read-only.
       dtWD->SetRW(fld_system, true);            // Set this field to read-only.
       dtWD->SetMSupdate(fld_enabled, true);     // This field may be modified by the Chief site
       dtWD->SetMSupdate(fld_system, true);      // This field may be modified by the Chief site



       // The RoutesTable route table. dtRT
       //dtRT = new datatable(RoutesTable, fld_RID);      // Create the table to hold info about our WDs. Index is ID.
       //dtRT->AutoAddRows = aDB->AutoAddRows;            // use the default autoadd setting for this table
       //myDB.LoadTable(dtRT);                            // load the table data from the database. Also loads field definitions
       //dtRT->parentdb = aDB;

       // The ETHx devicedesignator table listing all devices this gatway commnunicates with
       dtEDD = new datatable(EthDevDesTable, fld_designator);     // Create the table to hold info about our routed protocols
       dtEDD->AutoAddRows = aDB->AutoAddRows;                  // use the default autoadd setting for this table
       myDB.LoadTable(dtEDD);                                  // load the table data from the database. Also loads field definitions
       dtEDD->parentdb = aDB;

       // The TTYx devicedesignator table listing all devices this gatway commnunicates with
       dtTDD = new datatable(TtyDevDesTable, fld_t_designator);     // Create the table to hold info about our routed protocols
       dtTDD->AutoAddRows = aDB->AutoAddRows;                  // use the default autoadd setting for this table
       myDB.LoadTable(dtTDD);                                  // load the table data from the database. Also loads field definitions
       dtTDD->parentdb = aDB;

       // The Wireless NAT table. Lists PORT <-> ID translations
       dtWNAT = new datatable(WNATTable , fld_designator);           // Create the table to hold info about our routed protocols
       dtTDD->AutoAddRows = aDB->AutoAddRows;                        // use the default autoadd setting for this table
       myDB.LoadTable(dtWNAT);                                        // load the table data from the database. Also loads field definitions
       dtTDD->parentdb = aDB;

       // Pager translation table
       dtPagers = new datatable(PagerDBTable, pagerNumberColumn);
       dtPagers->AutoAddRows = false;
       myDB.LoadTable(dtPagers);
       dtPagers->parentdb = aDB;

       return true;
       }

// Load the
bool LoadTable(database* aDB, datatable *dtDTB, string TableName){
    int i;
       // The RoutesTable route table. dtRT
       dtDTB->AutoAddRows = aDB->AutoAddRows;            // use the default autoadd setting for this table
       i = aDB->LoadTable(dtDTB);                        // load the table data from the database. Also loads field definitions
       dtDTB->parentdb = aDB;

       if (i > 0 )
          return true;
       else
          return false;
}


void CheckForDBupdates(void){
  time_t now_time1 = time(NULL);
  static time_t last_time1 = time(NULL);
  static int PushTimer = 0;

  if (last_time1 != now_time1){
      // Run these routines every second
      last_time1 = time(NULL);
      PushTimer++;
      if (PushTimer >= Me.dbPushInterval){
          // Push updates back to the database if any records have changed.
          PushTimer = 0;
          if ((dtWD->CountRowState(Added) > 0) || (dtWD->CountRowState(Modified) > 0)){
              // Time to update the database with some new or modified entries
              myDB.PushUpdatesToDB(dtWD);
          }
      }
  }
}
