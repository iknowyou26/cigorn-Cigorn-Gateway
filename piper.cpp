/* 
 * File:   pipe.cpp
 * Author: john
 * 
 * Created on January 18, 2011, 11:04 PM
 */

#include <iostream>
#include <string>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include <ios>
#include <limits>

#include "GlobalVar.h"
#include "piper.h"
#include "functions.h"
#include "ascii.h"
#include "BinaryEntry.h"
#include "DeviceList.h"
#include "CommThread.h"

using namespace std;


piper::piper() {

    PipeTo = "";
    PipeFrom = "";
    PipingNow = false;
    StartTime = -1;         // when we started this pipe
    MyState = pipe_Idle;
    pipetoindex = -1;
}

piper::piper(const piper& orig) {
}

piper::~piper() {
}

// Some data came in via a terminal interface that is designated as a pipe source
// see if the user wants to create a pipe and send data to/from this interface to
// some other device designator.
void piper::DataFromTerminal(BinaryEntry& be){

    int src = be.SrcDevDesIndex;
    string s = "";
    stringstream ss;

    switch(MyState){
        case (pipe_Idle):
            StoreText(be , Prompt());   // send the prompt back
            DataRouter.RouteBinTo(src, be.data, be.bcount);   // loop text back
            MyState = pipe_WaitCmd;
            break;
        case pipe_WaitCmd:
            s = ToString(be.data, be.bcount);   // copy the data to a string
            NewCommand(s);                      // parse the string
            if (PX[1] == "PIPE"){
                // Someone wants to start a pipe
                PipeTo = px[2];
                pipetoindex = OurDevices.IndexOf(PipeTo);
                if (pipetoindex >= 0){
                    PipeFrom = OurDevices.getDevDes(be.SrcDevDesIndex);
                    StartTime = TimeNow();
                    PipingNow = true;
                    MyState = pipe_Piping;
                    ss <<  "Piping to" << PipeTo << " via " << PipeFrom << endl;
                    StoreText(be, ss.str());   // send the message back
                    DataRouter.RouteBinTo(src, be.data, be.bcount);   // loop text back
                }else{
                    ss << "Error. None-existant device designator" << endl;
                    StoreText(be , ss.str());   // send the prompt back
                    DataRouter.RouteBinTo(src, be.data, be.bcount);   // loop text back
                    StoreText(be , Prompt());   // send the prompt back
                    DataRouter.RouteBinTo(src, be.data, be.bcount);   // loop text back
                    MyState =  pipe_WaitCmd;
                    pipetoindex = -1;
                    PipingNow = false;
                }
            }
            break;
        case pipe_Piping:
            cout << "Piping to:" << pipetoindex << be.data << endl;
            DataRouter.RouteBinTo(pipetoindex, be.data, be.bcount);   // loop data to the piped interface
            if ((TimeNow() - StartTime) > MAXPIPETIME){
                MyState = pipe_Done;
                StoreText(be , Prompt());   // send the prompt back
                DataRouter.RouteBinTo(src, be.data, be.bcount);   // loop text back
                MyState = pipe_WaitCmd;
                PipingNow = false;
                pipetoindex = -1;
                PipeTo="";
            }
            break;
        case pipe_Done:
            MyState = pipe_WaitCmd;
            PipingNow = false;
            pipetoindex = -1;
            PipeTo="";
            break;
    }

}

// If we are piping, check the endo fo the pipe for incomming data so we can direct it to the other 
// end of the pipe
bool piper::CheckForNewInput(void){
    // Are we piping now?
    if (PipingNow == false)
        return false; //no

    if ((pipetoindex >=0 ) && (pipetoindex < MAXDEVDES)){
        // This is the interface that we are piping to.
        if (OurDevices.IsTTY(pipetoindex)){
            // Serial interface

        }
        if (StringLeft(OurDevices.interfaces[pipetoindex], 3) == "eth"){
            // Ethernet interface
            
        }



    }


}

bool  piper::PipeBinary(BinaryEntry& be){
    if(be.DstDevDesIndex != -1){
        // Someone already specified where this goes in the parser
        DataRouter.RouteBinTo(be.DstDevDesIndex, be.data, be.bcount);
        // Now erase the binary entry so it cannot be routed anywhere else.
        be.bcount = 0;
        be.format = -1;
        be.SrcDevDesIndex = -1;
        be.PortIn = -1;
        return true;
    }
    
    // Are we piping now?
    if (PipingNow == false)
        return false; //no
    
    // See if we have a pipe open to this devicedesignator
    if (pipetoindex == be.SrcDevDesIndex){
        // This data came from an interface that is the one we are piping to/from
        int i = OurDevices.IndexOf(PipeFrom);
        DataRouter.RouteBinTo(i, be.data, be.bcount);   // loop data back to the piping interface the originated the pipe.
        // Now erase the binary entry so it cannot be routed anywhere else.
        be.bcount = 0;
        be.format = -1;
        be.SrcDevDesIndex = -1;
        be.PortIn = -1;
        return true;
    }

    return false;

}


std::string piper::Prompt(void){

    return CigornPrompt;

    }


// put the string into the binaryentry.data[]
void piper::StoreText(BinaryEntry& be, std::string s){

    if (s.size() > 0){
        to_cstring(be.data, s, be.MAXDATA);
        be.bcount = s.size();
    }else{
        // Nothing in the string
        be.data[0] = NUL;
        be.bcount = 0;
    }
}

void piper::NewCommand(std::string cmd){

    int i;

    // Erase  the old parameters
    for (i = 0; i < MAXPIPEPARMS; i++){
        px[i] = "";
        PX[i] = "";
        pi[i] = 0;
    }

    cmd = trim(cmd);
    
    // Parse the parameters
    if (cmd.length() > 0 ){
        i = 1;
        while (i < MAXPIPEPARMS){
            px[i] = GetSubString(cmd, i);  // get each individual paramter on the line
            PX[i] = StringToUpper(px[i]);  // UPPERCASE version of the paramters.
            pi[i] = StringToInt(px[i]);    // convert to interger if possible.
            if (px[i].length() <= 0)
                break;  // no more paramters
            i++;
        }
    }


}

