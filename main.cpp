/*********************************extern int TestDeviceRepository();***********************/
// RaveonNet.cpp
//
//
/********************************************************/

using namespace std;

#include <fstream>
#include <sys/stat.h>

#ifndef _WIN32
#include <syslog.h>
#else
#include <cstdarg>

constexpr int LOG_PID  = 0;
constexpr int LOG_USER = 0;
constexpr int LOG_INFO = 0;
constexpr int LOG_ERR  = 0;
constexpr int LOG_CRIT = 0;

inline void openlog(const char*, int, int)
{
}

inline void syslog(int, const char*, ...)
{
}
#endif
#include "platform/thread/PlatformMutex.h"
#include "platform/thread/PlatformLockGuard.h"
#include "platform/thread/PlatformThread.h"
#include "platform/time/PlatformTime.h"
#include <filesystem>
#include "Cigorn.h"     // Our application-specific constants and headers
#include "GlobalVar.h"
#include "TCPsocket.h"
#include "CommandLine.h"
#include "CommThread.h"
#include "Console.h"
#include "Router.h"
#include "health.h"
#include "database.h"
#include "datatable.h"
#include "datarow.h"
#include "mainsubs1.h"
#include "microsleeper.h"
#include "emailer.h"
#include "cypher.h"
#include "SocketThread.h"
#include "webserver.h"
#include "logger.h"
#include "network.h"

#include "modulator.h"
#include "demodulate.h"
#include "UserCLI.h"
#include "sync-roles.h"
#include <signal.h>
#include "Test1.h"
#include "Security.h"

// Local prototypes
char *entry, letter, choice[2];
int ascii, len, binary[8], total;
int our_second = 0;

Gateway Me;                        // the settings for this gateway
map<string, Gateway> GateWays;     // The list of all other gateways we know about

bool ShutDownApplication = false;
bool RestartApplication = true;    // set true to start or "Restart Now" the application.
bool AppIsRunning = false;         // Global variables for statistics
int maincounter = 0;
long nmeacount = 0;
long pravecount = 0;
long wmxcount_in = 0;
long xmlcount_in = 0;
long Cigorncount_in = 0;
long Cigorncount_out = 0;
long routecount = 0;
long FailedSockOut = 0;
long FailedTTYOut = 0;
string cmd_password_hashed = "";

std::streambuf* old_cout;
std::ofstream  my_cout;

// Define the class to record errors.
logger elog(ERRORLOGFILE, LOGEXTENSION);   // create with default file name for our errorlogfile
logger mlog(EMAILLOGFILE, LOGEXTENSION);   // email log file

bool iniOK = false;

health SysHealth;  // create the object that reports system health.

// Our object locks.  This is a multi-tasking program and many structures are manipulated on various threads
// Use pthreads to lock access to shared structures as we manipulate them.
cigorn::PlatformMutex qlock;
cigorn::PlatformMutex devlock;
cigorn::PlatformMutex ttylock;
cigorn::PlatformMutex socklock;
cigorn::PlatformMutex addlistlock;
cigorn::PlatformMutex cmdqlock;
cigorn::PlatformMutex dlyvlock;
int maxQcount = 100;         // Maximum number of entries we allow in a queue
int maxQage = 60;            // Maximum age of messages in the queue, in seconds

Router DataRouter;           // Create an instance of our data router

database myDB;               // create an instance of our database interface
emailer myEmail;             // create an email client to send email notices out
cypher  myCrypto;            // basic crypto functions
webserver myWeb;             // www interface to cigorn
//health myHealth;
UserCLI  MyCLI;

string Application = APP_TITLE;
string AppVersion;
bool maininitialized = false;
bool consolethreadcreated = false;
string dbType = "PostgreSQL";
string dbUser = "postgres";
string dbPass = "administrator";
string dbName = "cigorn";
string dbHost = "localhost";
time_t boot_time;

// Command line processor and local console user interface
CommandLine cli;            // create the defaul CLI object to the interface to the user
//CommandLine *pcli;

datatable *dtWD;            // Our Wireless Devices on the network
datatable *dtRT;            // Our data routes. Definition of what type of message from where gets routed to who.
//datatable *dtPR;            // Our protocol.  List of typical protocols we parse and route.
datatable *dtSC;            // SiteConfiguration table.
datatable *dtEDD;           // Ethernet Device designators
datatable *dtTDD;           // TTY Device designators
datatable *dtWNAT;          // Wireless NAT table
datatable *dtPagers;        // Pagers table

// Syste-wide settings, read from the data table holding our settings
int TicksPerSecond = CLOCKS_PER_SEC;  // our goabl clock calibration value
int emailnotice = 0;           // email notification level.  0=none, 1=minimal, 2=normal, 3=verbose.
double statusemailinterval = 24;  // number of hours between email notices.  -1 means never.
double statusemailtimer = 0;
string webusername = "";
string webpassword = "";
mlevel messagelevel =  MSG_NONE;
int consoleport = 2300;             // default to the telent port as our CLI

int StandbyHealthTime = 30;       // how often we check the health of the primary gateway if we are a hot-standby
Security SyncSecurity;              // handles data encryption for the inter-cigorn communication

// Signal counters
int sigHupCount = 0;
int sigPipeCount = 0;

microsleeper  MainSleeper(MAINSLEEP, 2);   // does microsecond sleeping. Stay awake N loops if we are busy, or sleep X uS if not.

void sig_hup_handler(int signum)
{
    sigHupCount++;
    ShutDownApplication = true;
}

/**
 * SIGPIPE handler. We ignore SIGPIPEs and catch the failed writes after
 * an attempted write. Here we count them
 * @param signum
 */
void sig_pipe_handler(int signum)
{
    sigPipeCount++;
}

int main(int argc, char *argv[],char *envp[] )
{
   AppIsRunning = true;
   messagelevel =  MSG_NONE;
   int l = 0;
   int i;
   int PushTimer = 0;
   int ActivittFileSize = 0;
   time_t now_time = time(NULL);
   time_t last_time = time(NULL);
   string today = LocalTime();
   int SecondsRunningMain = 0;
   bool redir = true;
   int commLoopDeadSeconds = 0;
   int tcpLoopDeadSeconds = 0;

   IPaddList::iterator it;

   stringstream ss;   // use for debug message outputting

   cigorn::PlatformThread CommThread;
   cigorn::PlatformThread ConsoleThread;
   char *pmessage = "Thread";

   // Added: Signal handler to terminate
#ifndef _WIN32
   signal(SIGHUP, sig_hup_handler);
   signal(SIGPIPE, sig_pipe_handler);
#endif
   openlog("cigorn", LOG_PID, LOG_USER);

   // Get ourselves ready to write to user.log
   openlog("cigorn", LOG_PID, LOG_USER);

   ss << "   " << endl;
   ss << " - - - STARTUP - - -" << endl;

   // Begin by outputting the application info out the UI
   AppVersion = GetVersionText();

   // Set this sites defaults
   Me.MyName = "DefaultSitename";
   Me.DevDes = -1;
   Me.gaterole = Primary;
   Me.IsChief = true;
   Me.IsActive=true ;          // assume I am unless we find out were a standby
   Me.dbPushInterval = 600;
   Me.PrimaryDB = dbName;      // only set if we are a backup gateway in hot standby mode
   Me.PrimaryName = "";
   Me.PublicKey = "";
   Me.MessagesCountIn = 0;

   ss << AppVersion << endl;
   time_t cpu_time = time(NULL);       // show the start-up time of the firmware
   ss << ctime(&cpu_time) << endl;

   cout << AppVersion << "\r\n";
   cout << ctime(&cpu_time) << "\r\n";

   {
       cigorn::PlatformLockGuard lock(addlistlock);
       GetMyIPaddressList(ipaddresses);
   }
 
   // See if the user does not want to redirect the optuput to a file. Be default we do unless told not to.
   for (i=0; i<argc; i++){
        if (string(argv[i]) == NO_COUT_REDIR)
           redir = false;
        if (StringToUpper(string(argv[i])) == "M1")
           messagelevel = MSG_STATUS;
        if (StringToUpper(string(argv[i])) == "M2")
           messagelevel = MSG_DEBUG;
   }

  redir = false; // For now, no re-direction untill we find a memory-safe way to do it
  // if (redir){
  //     ss << "Redirecting output to " << OUTPUTFILE << endl;
  ///     ConfigureConsole(OUTPUTFILE);       // re-define our console stream for general console message output
  //     ss << AppVersion << endl;           // SHow the version and time in the new file
  //     ss<< ctime(&cpu_time) << endl;
  // }
  
   switch (messagelevel){
       case MSG_NONE:
           ss << "Message Level: minimum" << endl;
           break;
       case MSG_STATUS:
           ss << "Message Level: status" << endl;
           break;
       case MSG_DEBUG:
           ss << "Message Level: debug" << endl;
           break;

   }

   // List the ethernet interfaces on this box
   ss << "List of the " << ipaddresses.size() << " local Ethernet interfaces on this gateway:" << endl;
 
   if (ipaddresses.size() > 0){
       for (it = ipaddresses.begin(); it != ipaddresses.end(); it++){
          if ( trim(it->second.interface).size() > 0){
               ss << "Local ethernet interface " << it->second.interface << " = " << it->second.ipaddress << endl;
               cout << "Local ethernet interface " << it->second.interface << " = " << it->second.ipaddress << "\r\n";
          }
       }
   }

   
   // cout << ss.str() << endl;  // dump to the console in case someone is watching

   // s = GetIP("eth0", &ip, &mask);
   //cout << "etSyncSecurityh0 =" << s << endl;
   
   boot_time = time(NULL);
   // Do not change this key unless you want to make all previous version incompatible with this one
   // This key is the one Cigorn will use internally to store passwords and things it needs to hide from the user.
   myCrypto.newkey("t7&dL~g");     // set the default encryption key for storing passwords. 7 Chars MAX!


   bool busy = false;

   ShutDownApplication = false;  // global variable to shut us down

   // See if the user does not want to redirect the optuput to a file. Be default we do unless told not to.
   for (i=0; i<argc; i++){
        if (string(argv[i]) == NO_COUT_REDIR)
           redir = false;
   }
   
  if (redir){
     ss << "Redirecting output to " << OUTPUTFILE << endl;
     ConfigureConsole(OUTPUTFILE);         // re-define our console stream for general console message output
  }

  // load up the commands in the Comand Line Interpreter
  cli.addAllCommands();

  // Create the log directory if it does not already exist.
  const char dir[] = LOGDIRNAME;
  const std::filesystem::path logDirectory(dir);

  std::error_code directoryError;
  std::filesystem::create_directories(logDirectory, directoryError);

  if (directoryError)
  {
      ss << "Warning: Unable to create log directory "
         << logDirectory.string()
         << ": "
         << directoryError.message()
         << endl;
  }

   

  if (!isadir(dir)){
      SysHealth.LogDirectory = false;  // no log directory
      ss << "Error.  Failed to create log directory:" << LOGDIRNAME << endl;
  }

  // lets list the command-line arguemnts.
  if (argc > 0){
      ss << "Command line arguments ";
      for (i=1; i<argc; i++){
         ss << argv[i] << " ";
      }
      ss << endl;
    }

   ss << "Error log file: " <<  elog.FullFileName() << endl;
   // Put an entry in the error log to time-stamp it
   elog.store(">>  Initializing Program. No error yet.   <<");

   // generate an RSA key once at start up
   SyncSecurity.RSAkeyGen();      // create the RSA key for secure inter-site communications
   Me.PublicKey = SyncSecurity.MyPublicKey;

   // Create a intance hash for the standby gataway to see we restarted
   Me.DBmodifyflag = (int)time(NULL);

   RestartApplication = true;    // always start once.
   
   // Begin the section of code that is run when we
   // first start and when we restart the application
   while (RestartApplication){
       maininitialized = false;      // we are not yet initialized
       RestartApplication = false;   // we started and won't restart unless this flag gets set true

       syslog(LOG_INFO, "Launching processing threads");

       // added by Chad... so we can troubleshoot future issues with Cigorn.ini if at all.
       // cout << "FILE: " <<  __FILE__ << ", LINE: " << __LINE__ << endl;
       //printf("INI DIRECTORY: ");
       //fflush(stdout); // do not remove
       //if (system("pwd"));

       i = readini(INIFILE);
       if (i >= 0){
           ss << "Read " << i << " lines from .ini file." << endl;            // Read the .ini setting file to get our configuration/settings
           cout << "Read " << i << " lines from .ini file." << "\r\n";            // Read the .ini setting file to get our configuration/settings
       }else{
           ss << "Error reading .ini file." << endl;            // Read the .ini setting file to get our configuration/settings
           cout << "Error reading .ini file." << "\r\n";            // Read the .ini setting file to get our configuration/settings
       }
       cout << "LOGIN CONFIG: user=[" << webusername
            << "] password=[" << webpassword << "]" << endl;

       // Initialize the connection to the database
       myDB.connect(dbHost, dbName, dbUser, dbPass);
       Me.DBmodifyflag = (int)time(NULL);                // remmeber we re-read the DB
   
       if (myDB.ConnectionOK){
           ss << "Connected OK to DataBase:" << dbName << endl;
           cout << "Connected OK to DataBase:" << dbName << "\r\n";
//TestSettingsTableAdapter();
       }
       else{
           ss << "Failed to connect to DataBase:" << dbName << endl;
           cout << "Failed to connect to DataBase:" << dbName << "\r\n";
           elog.store("Error 711. Failed to connect to DataBase:" + dbName);
       }

      
       // Create the table to hold site configuration settings
       dtSC = new datatable(SiteConfig, fld_varname);    // Create the table to hold our settings
       myDB.LoadTable(dtSC);
       dtSC->parentdb = &myDB;
       ss << "Initialized data table: " << dtSC->tablename << ". " << dtSC->rows.size() << " rows." << endl;

       LoadTablesFromDB(&myDB);

       ss << "Initialized table: " << dtWD->tablename << ". " << dtWD->rows.size() << " rows." << endl;
       ss << "Initialized route repository." << endl;
       //ss << "Initialized table: " << dtRT->tablename << ". " << dtRT->rows.size() << " rows." << endl;
       //ss << "Initialized table: " << dtPR->tablename << ". " << dtPR->rows.size() << " rows." << endl;
       //ss << "Initialized table: " << dtEDD->tablename << ". " << dtEDD->rows.size() << " rows." << endl;
       //ss << "Initialized table: " << dtTDD->tablename << ". " << dtTDD->rows.size() << " rows." << endl;
       //ss << "Initialized table: " << dtWNAT->tablename << ". " << dtWNAT->rows.size() << " rows." << endl;
       //ss << "Initialized table: " << dtPagers->tablename << ". " << dtPagers->rows.size() << " rows." << endl;
       ss << "Initialized EthDevice repository/adapter." << endl;
       ss << "Initialized TtyDevice repository/adapter." << endl;
       ss << "Initialized WirelessNat repository/adapter." << endl;
       ss << "Initialized Pager repository/adapter." << endl;
       ss << "Initialized pager repository." << endl;
       myDB.lastUpdate = LocalTimeStamp();

       // Now copy the SQL route table over to the the router table structure
       BuildRouteTable();      // take the table and copy it to the route structure. (Faster access)
       
       // Initialize the object interfaces to the serial ports (ttyS)
       try{
           InitializeComPorts();
       }
       catch(string e){
          elog.store("Error 110. Problem initializing serial ports.");
       }

       // Configure the web server before opening up all the sockets.
       ConfigureWeb(dtSC, &myWeb);
       myWeb.ProcessWebsite();
       myWeb.ProcessWebsite();
       myWeb.ProcessWebsite();

       if (consolethreadcreated == false){
           // Create the threads we need to do all the work.

           /* Create independent thread for the console input */
           ConsoleThread = cigorn::PlatformThread([pmessage]() { threadConsole(static_cast<void*>(pmessage)); });
           consolethreadcreated = true;

           /* Create thread to run the communications */
           CommThread = cigorn::PlatformThread([pmessage]() { threadComm(static_cast<void*>(pmessage)); });
           cigorn::SleepMilliseconds(50);
      }else{
           // Now the first time through this initialization process.
           if (ipaddresses.size() > 0){
               for (it = ipaddresses.begin(); it != ipaddresses.end(); it++){
                  if ( trim(it->second.interface).size() > 0){
                       ss << "Local ethernet interface " << it->second.interface << " = " << it->second.ipaddress << endl;
                       cout << "Local ethernet interface " << it->second.interface << " = " << it->second.ipaddress << "\r\n";
                  }
               }
           }


      }


       // Configure our system
       ReadTable(dtSC);                 // Read the settings from the database table SiteConfig (preferred method)

       // Telnet Port for remote commands and user command-line-interface
       ss << "Command Line Interface via port " << consoleport << endl;
       ss << "WEB Interface via port " << myWeb.portnum << endl;
       cout << "Command Line Interface via port " << consoleport << "\r\n";
       cout << "WEB Interface via port " << myWeb.portnum << "\r\n";

       if (MyCLI.MySocket.portnum != consoleport)
           MyCLI.Configure(consoleport);
       
       cout << "CLI port:" << MyCLI.MySocket.portnum <<  "\r\n";

       MyCLI.Display(&ss);
       ss.str("");                   // clear the buffer

       ConfigureEmail(dtSC, &myEmail);

       if (dtSC->EntryExists("emailnotice" ,fld_param1))
          emailnotice = StringToInt(dtSC->LookupData ("emailnotice" ,fld_param1));  // get the first parameter field of the entry for the emailnotice.
       else
          emailnotice = 0;   // no email notices

       cigorn::SleepMilliseconds(250);  // wait for the threads to get going

       ss << endl << ".... Initialization Complete ...." << endl;
       cout << endl << "..... Initialization Complete ....." << "\r\n";

       maininitialized = true;      // tell the other threads we are running now
       RedrawStatDisplay = true;    // redraw the console display because we read the .ini file
       StopCommunications = false;  // let comm thread run if it was halted. 

       if (emailnotice > 0)
           send_reboot_email(&myEmail);       // tell someone that we just rebooted

       statusemailtimer = TimeNow();

       // cout << ss.str() << endl;                   // output to the console in case someone is watching it
       MyCLI.Display(&ss);  // send the text to the console output
       ss.str("");                   // clear the buffer

       syslog(LOG_INFO, "All threads launched");
       // All threads created.  Enter the main loop where we stay forever.
       while ((!ShutDownApplication) && (!RestartApplication)){
              busy = false;
              now_time = time(NULL);
              maincounter++;

              // See if any updated need to be pushed back to the database
              CheckForDBupdates();

              if (last_time != now_time){
                  // Run these routines every second
                  last_time = time(NULL);
                  SecondsRunningMain++;
                  if ((getMainLoopSpeed() < 10) && (SecondsRunningMain > 4))
                      CoutM2(ss) << "Main Loop slow. " << getMainLoopSpeed() << "Hz" <<  endl;

                  if (getCommLoopSpeed() == 0){
                      commLoopDeadSeconds++;
                  }else{
                      commLoopDeadSeconds = 0;
                  }

                  if (getTCPLoopSpeed() == 0){
                      tcpLoopDeadSeconds++;
                  }else{
                      tcpLoopDeadSeconds = 0;
                  }

                  if(tcpLoopDeadSeconds == 2 || commLoopDeadSeconds == 2){
                      // It's starting to look like we'll be shutting down
                      // soon. Log what we can
                      if(tcpLoopDeadSeconds > 0){
                        syslog(LOG_ERR, "TCP Loop dead for %d seconds", tcpLoopDeadSeconds);
                      }

                      if(commLoopDeadSeconds > 0){
                        syslog(LOG_ERR, "Comm Loop dead for %d seconds", commLoopDeadSeconds);
                      }

                      for (int sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
                        if (tcpsockets[sockindex].myDevDesIndex >= 0){
                          // Socket is open and interesting
                            syslog(LOG_ERR, "Socket %d: %s:%d <-> %s:%d, %ld/%ld",
                                    sockindex,
                                    tcpsockets[sockindex].hostIPaddress.c_str(),
                                    tcpsockets[sockindex].hostport,
                                    tcpsockets[sockindex].ConnectedToIP.c_str(),
                                    tcpsockets[sockindex].RemotePort,
                                    tcpsockets[sockindex].bytes_in,
                                    tcpsockets[sockindex].bytes_out);
                        }
                     }
                  }

                  if(tcpLoopDeadSeconds > 60 || commLoopDeadSeconds > 60){
                      syslog(LOG_CRIT, "Cigorn exiting due to lockup condition");
                      exit(1);
                  }
             }

             // process any emails if there are any.
             if (myEmail.ProcessEmails())
                 busy = true;

             // process web requests
             if (myWeb.ProcessWebsite()){
                 busy = true;
             }

             // see if it is time to send a status email
             if (((TimeNow() - statusemailtimer) > (statusemailinterval * 3600)) && (statusemailinterval > 0)){
                 SendStatusEmail(&myEmail, "");
                 statusemailtimer = TimeNow();
                 busy = true;
             }

             // 3.0
             //See if we need to read a different gateway's table because we are a backup database
             if (Me.gaterole == Standby){
                 DoClusterTasks();
             }else{
                 DoClusterTasks();
             }

             // Output the debug text if there is any
             if (ss.str().size() > 0){
                 MyCLI.Display(&ss);;   // send the text to the console output
                 ss.str("");                   // clear the buffer
             }

             MainSleeper.DozeOff(busy);

       }// High-speed inner program loop. Run here till we shutdown or restart

       syslog(LOG_INFO, "Threads halting");
       cout << "LOGIN CONFIG: user=[" << webusername
     << "] password=[" << webpassword << "]" << endl;
       // Disconnect the web server and email sockets
       myWeb.MySocket.DisconnectSocket();
       myEmail.MySocket.DisconnectSocket();

       ss << "Execution paused." << endl;
       MyCLI.Display(&ss);   // send the text to the console output

       // Come here when the application is supposed to shut down or restart
       if (RestartApplication){
           // We need to reload everything and restart the application.
           ss << "Restarting Cigorn Gateway." << endl;
           MyCLI.Display(&ss);   // send the text to the console output
           cout <<  "Restarting Cigorn Gateway." << endl;

           ss << "Shutting down communication threads." << endl;
           StopCommunications = true;           // flag we need to stop comm thread for a bit, and restart it
           MyCLI.Display(&ss);   // send the text to the console output
           cout <<  "Shutting down communication threads." << endl;

           last_time = time(NULL);
           i=0;

           while ((i < 6) && (CommunicationsRunning)){
               // Wait X seconds for communication processes to end
               if (last_time != time(NULL)){
                   // Count the seconds
                   i++;
                   last_time = time(NULL);
                   ss << "Please wait.";
                   for (l=0; l<i; l++)
                       ss << ".";
                   ss << "\r";
                   MyCLI.Display(&ss);   // send the text to the console output
               }
           }
           OurDevices.Clear();   // erase all device designators
           // Output the debug text if there is any
           // pthread_join( CommThread, NULL);    // wait for Comm thread to finish
           cout << "Communications Thread Stopped. " << "\r\n";

           cout << "Restarting application." << "\r\n";
           elog.store("Restarting application.");
       }
       cigorn::SleepMilliseconds(1000);  //

   }// While restart...  MAIN loop including restart

  // We are shutting down
  ss << "Shut down Cigorn main." << endl;
  MyCLI.Display(&ss);   // send the text to the console output
  cout << "Shutting down Cigorn main." << "\r\n";
  cigorn::SleepMilliseconds(1000);
  // wait while other tasks get killed.

  ShutDownApplication = true;
  AppIsRunning = false;  // we are dead.  no more CLI.  
  cigorn::SleepMilliseconds(2000);             // wait while other tasks get killed.
  
  delete dtWD;          // remove the tables from the heap
//  delete dtRT;
//  delete dtPR;

  cigorn::SleepMilliseconds(2000);
  cout<< "Goodby." << "\r\n";
  //exit(1);   // done with the program.  Stop now.

  return 0;
}

/**
 * Gets the current measured main loop speed, in Hz
 * @return Main loop's speed, in Hz
 */
int getMainLoopSpeed(){
    return MainSleeper.getLoopSpeed();
}


