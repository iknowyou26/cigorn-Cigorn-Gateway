/* 
 * File:   CommandDefinitions.h
 * Author: john
 * Define all of the basic commands used by the RaveonNet gateway.
 * Created on August 27, 2010, 10:33 PM
 */

#ifndef _COMMANDDEFINITIONS_H
#define	_COMMANDDEFINITIONS_H

#define xxx  false

// Command text must be capital letters.
rncommand ourCommands[] = {
  //                                                                                                    devide types that may execute
  //     Comand     Help text            function pointer to execute    `dCLI     dINI    dRNC
    {1, "AUTOADD", "Enable AUTOADD, y/n.",             "cmdAutoadd" ,    true ,  true,    xxx},
    {2, "CONFIG",  "Display Configuration",            "cmdConfig" ,     true ,  true,   true},
    {3, "CONNECT", "Assign a device to an interface.", "cmdConnect" ,    true ,  true,   true},
    {4, "CHPARM",  "Configure a channel parameter.",   "cmdChparm" ,      xxx ,  true,   true},
    {5, "SET",     "Configure a gateway parameter.",   "cmdSet" ,        true ,  true,   true},
    {6, "DBASE",   "Configure database.",              "cmdDBase",        xxx ,  true,    xxx},
    {7, "EMAIL",   "Email status or site info.",       "cmdEmail"  ,     true ,   xxx,   true},
    {8, "ERRLOG",  "Manage the error log.",            "cmdErrLog"  ,    true ,   xxx,   true},
    {9, "SHUTDOWN","Stop execution this program.",     "cmdExit"  ,      true ,   xxx,    xxx},
    {10,"GATEWAY", "Configure gateway.",               "cmdGateway" ,     xxx ,  true,    xxx},
    {11,"DEVICES", "List all devices.",                "cmdDevices" ,    true ,   xxx,    xxx},
    {12,"HELP",    "List all commands",                "cmdShowHelp"  ,  true ,   xxx,    xxx},
    {13,"HELLO",   "Hello World",                      "DoNothing"  ,    true ,  true,   true},
    {14,"SERIAL",  "Serial port information",          "cmdSerial",      true ,   xxx,   true},
    {15,"SHOW",    "Show {WD|ROUTES|SITES|LIMITS|TDMA}", "cmdShow",      true ,   xxx,   true},
    {16,"STATS",   "General System Statistics",        "cmdStatistics",  true ,   xxx,   true},
    {17,"RFPORT",  "Set an interface to radio.",       "cmdRFport"  ,     xxx ,  true,   true},
    {18,"ECHO",    "Turn on data echo to the console.","cmdEcho",        true ,   xxx,    xxx},
    {19,"SOCKETS", "Get socket information.",          "cmdSockets",     true ,   xxx,    xxx},
    {20,"VER",     "Read the software version.",       "cmdVer"  ,       true ,  true,   true},
    {20,"VERSION", "Read the software version.",       "cmdVer"  ,       true ,  true,   true},
    {21,"ROUTE",   "Put an entry in the routing table.", "cmdRoute",      xxx ,  true,   true},
    {22,"RESTART", "Restart the gateway.",             "cmdReboot",      true ,   xxx,   true},
    {23,"RELOAD",  "Reload the gateway tables.",       "cmdReload",      true ,   xxx,   true},
    {24,"RADIO",   "Radio related stats and settings. ","cmdRadio",      true ,  true,   true},
    {25,"MESSAGELEVEL", "Set the debug message level M0,M1,M2 ","cmdMessage",    true ,  true,   true},
    {26,"ML", "Set the debug message level M0,M1,M2 ","cmdMessage",    true ,  true,   true},
    {27,"EXIT", "Exit the command interpreter session. ","cmdExitCLI",   true ,  true,   true},
    {28,"RESET", "Re-initialize a connection. ","cmdReset",              true ,  true,   true},
    {29,"PAUSE", "Pause the input or output of a connection.","cmdPause",true ,   xxx,   true},
    {30,"UNPAUSE", "Unpause the input or output of a connection.","cmdUnpause",true ,   xxx,   true},
    {31,"QUEUEREPORT", "Get queue report for a connection","cmdQueuereport",true ,   xxx,   true}
};

#define TOTAL_INITIAL_COMMANDS (sizeof(ourCommands) / sizeof(rncommand))


#endif	/* _COMMANDDEFINITIONS_H */

