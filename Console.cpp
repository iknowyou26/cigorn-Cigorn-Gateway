// **************************************************************
// ConsoleThread.cpp
//
// **************************************************************

#include <iostream>
#include <string>
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "Cigorn.h"     // Our application-specific constants
#include "serialhandler.h"
#include "CommandLine.h"
#include "microsleeper.h"
#include "Console.h"

// Local prototypes

using namespace std;
bool InputAvailable(void);

void *threadConsole( void *ptr )
{
    // Handle the input from the console/CLI
    string s = "";
    string CMDresult = "";
    bool busy = false;
    microsleeper  MySleeper(500, 2);   // does microsecond sleeping. Stay awake N loops if we are busy, or sleep X uS if not.
    time_t now_time = time(NULL);
    time_t last_time = time(NULL);
    static string LastCommand = "";

    last_time = time(NULL);

    sleep(0.05);  // wait for the main loop to get running and finish its initialization
    MyCLI.MySocket.sendtext(CLI_PROMPT);

    cout << "CLI running" << "\r\n";
    
    while (AppIsRunning){
         // Come here when all commands are processed, and we want to look for another

         if (last_time != now_time){
              // Run these routines every second
              last_time = time(NULL);
         }

        s="";

        MyCLI.InputOutput();

        // See if there are charactors in the parser for the command line interface socket
        if (MyCLI.GetNewCommand(&s)){
            // A new command came in
            // cout << "Command in: '" << s << "'  Num of bytes=" << s.size() << " First=" << (int)s[0] << " Second=" << (int)s[1]<< endl;
            if ( s.size() == 2 && s[0]==ESC && s[1] == CR)
                s = LastCommand;  // repeat the last command
            else
                s = trim(s);
            
            // See if we should translate NL to CRLF on UNIX machines
            if (MyCLI.MySocket.MyParser.EndOfLineChar == CR){
                // Unix type terminal. Turn echo on.
                MyCLI.MySocket.NLtoCRLF = true;
                MyCLI.MySocket.echoinput = true;
            }else{
                MyCLI.MySocket.NLtoCRLF = false;
                MyCLI.MySocket.echoinput = false;
            }
            if (s.size() > 0 ){
                LastCommand = s;
                if (cli.processCommand(s, dCLI)){
                    CMDresult = cli.ResultStr;
                    MyCLI.MySocket.sendtext(CMDresult);
                    MyCLI.MySocket.sendtext(CLI_PROMPT);
                 }else{
                    MyCLI.MySocket.sendtext(CLI_ERROR);
                    MyCLI.MySocket.sendtext("\n");
                    if(cli.ResultStr != ""){
                        MyCLI.MySocket.sendtext(cli.ResultStr);
                    }
                    MyCLI.MySocket.sendtext(CLI_PROMPT);
                 }
            }else{
                MyCLI.MySocket.sendtext(CLI_PROMPT);
            }
            //cli.IntoCommandQ(s);   // put the new command into the command q to execute
        }
        
        MySleeper.DozeOff(busy);          // sleep a little incase we don't block.
     }

    MyCLI.MySocket.sendtext("Shut down Console thread.\r\n");
    MyCLI.MySocket.sendtext("Goodbye.\r\n");
    MyCLI.InputOutput();
    MyCLI.InputOutput();
    MyCLI.InputOutput();
    cout << "Shut down Console thread." << "\r\n";
    MyCLI.MySocket.DisconnectClient();
  
}


// Checks cin for a complete input line
bool InputAvailable(void)
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}



