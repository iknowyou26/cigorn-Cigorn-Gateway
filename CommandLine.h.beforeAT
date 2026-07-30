/* 
 * File:   CommandLine.h
 * Author: john
 *
 * Created on August 2, 2010, 4:56 PM
 */

#ifndef _COMMANDLINE_H
#define	_COMMANDLINE_H

#include <queue>
#include <string>
#include <sstream>
#include <map>
#include "rfport.h"

using namespace std;


#define MAX_NUM_COMMANDS 500          // arbitrary limit we'll create

typedef bool(*cmdFUNCTION) (void);

// Structure used to initialize the RaveonNet command interpreter
struct rncommand {
    int subroutine;        // an index into which subroutine to run for this command
    string command;        // The command text to type
    string help;
    string function;       // The name of the function to execute
    bool  viaCLI;          // true to allow execution via .ini file parser
    bool  viaINI;          // true to allow execution via CLI
    bool  viaRNC;          // true to allow execution via RaveonNet Communication with other gateways.
};

// Some public functions.
string StatisticsString(void);

class CommandLine {

public:
    CommandLine();
    CommandLine(const CommandLine& orig);
    virtual ~CommandLine();

    void SendToConsole(string);
    string GetNextCommand(void);
    int CommandQcount(void);
    void IntoCommandQ(string s);
    int ConsoleQcount(void);
    string getConsoleText(void);
    bool processCommand(string, int);
    bool  cmdShowHelp(void);
    bool DoNothing(void);
    bool cmdExit(void);
    bool cmdExitCLI(void);
    bool  cmdEmail(void);
    bool cmdStatistics(void);
    bool cmdConfig(void);
    bool cmdCon(string& );

    bool cmdVer(void);
    bool cmdEcho(void);
    bool cmdSockets(void);
    bool cmdSerial(void);
    bool cmdErrLog(void);
    bool cmdDevices(void);
    bool cmdChparm(void);
    bool cmdSet(void);
    bool cmdRoute(void);
    bool cmdGateway(void);
    bool cmdDBase(void);
    bool cmdShow(void);
    bool cmdAutoadd(void);
    bool cmdReboot(void);
    bool cmdReload(void);
    bool cmdRadio(void);
    bool cmdMessage(void);
    bool cmdReset(void);
    bool cmdPause(void);
    bool cmdUnpause(void);
    bool cmdQueuereport(void);

    int addAllCommands(void);

    string mycommand;
    string ResultStr;

    map<int, rncommand> commandMap;       // all of our commands in the map array

};

// Parameter types
#define   noParm  0
#define  strParm  1
#define  dblParm  2
#define  intParm  3

extern CommandLine cli;                 // create the defaul CLI object to the interface to the user

#endif	/* _COMMANDLINE_H */

