/* 
 * File:   UserCLI.cpp
 * Author: ss
 * 
 * Created on September 15, 2011, 6:54 PM
 */

#include "UserCLI.h"

UserCLI::UserCLI() {
}

UserCLI::UserCLI(const UserCLI& orig) {
}

UserCLI::~UserCLI() {
}


void UserCLI::Configure(int PortNumber){
    string s;

    LogDirectory = true;
    MySocket.portnum = PortNumber;
    MySocket.myDevDesIndex = 0;      // no device designator for this socket
    MySocket.index = 0;              // no indeex either.
    MySocket.init_tries = 0;
    MySocket.interface = "eth0";     // always use eth0
    MySocket.protocol = pServer;     // TCP/IP socket server
    MySocket.myDevType = dCLI;
    MySocket.description = "User Command Line Interface";
    MySocket.initialize_socket();
    ssOutBuffer.str("");             // empty the buffer

}

// Called very often to process the socket
// Return true if busy
bool UserCLI::InputOutput(void){

    static bool NewConnection = false;
    bool retval = false;
    string s = "";
    int max_bytes;
    int n;
    int bytes_read;
    char buffer[MAX_CLI_TRANSFER +1];


    // See if the socket is initalized. If it is, run it.
    if (MySocket.sockfd >= 0 ){
        MySocket.tcp_socket();    // always call this when connected to process the socket
    }else{
        MySocket.initialize_socket();
        // cout << "init socket here" << endl;
    }

    if ((MySocket.connected) && (NewConnection == false)){
        // Run when we first connect to the CLI
        NewConnection = true;
    }

    if (MySocket.connected == false){
        NewConnection = false;
        MySocket.MyParser.initialize();
    }else{
        // The socket is connected
        if (ssOutBuffer.str().size() > 0 ) {
            max_bytes = (MySocket.MaxBuffSize - MySocket.txcount) - 1;   // calculate how many bytes we have room for
            if (max_bytes > MAX_CLI_TRANSFER )
                max_bytes = MAX_CLI_TRANSFER;
            if (max_bytes < 0)
                max_bytes = 0;
            // We queued up some text to send out.
            if (ssOutBuffer.str().size() <= max_bytes){
                s = ssOutBuffer.str();
                ssOutBuffer.str("");
                retval = true;
            }else{
                s = ssOutBuffer.str();
                n = s.size() - max_bytes;   // number of bytes to remove from the string that won't fit in the socket buffer
                if (n < 0)
                    n = 0;
                else{
                     // ssOutBuffer.str().erase(0, max_bytes);
                     bytes_read = ssOutBuffer.readsome(buffer, max_bytes);
                     s = ToString(buffer, bytes_read);
                     retval = true;
                }
            }
            MySocket.sendtext(s);
            s = "";
       }
    }


    return retval;
}


bool UserCLI::GetNewCommand(string *cmdstring){
    string s = "";

    if (MySocket.MyParser.RawRXcount() > 0 ){
        // There are some charactors in the buffer
        if (MySocket.MyParser.look4command(&s)){
            // found a command
            *cmdstring = s;
            MySocket.MyParser.ClearRXbuffer();  // clear out the command buffer.
            return true;
        }else{
            *cmdstring = "";
            return false;
        }
    }
    *cmdstring = "";
    return false;
}


// Output a text string out the user interface. Thread safe.
// Stores text in buffer for later outputting.
bool UserCLI::OutputText(string s){

    if (s.size() == 0)
        return true;  // nothing to do

    if ((ssOutBuffer.str().size() + s.size()) < MAX_CLI_OUT_SIZE){
       ssOutBuffer << LocalTime() << " " << s;
    }else
       ssOutBuffer.str("");  // Clear the buffer

    return true;

}


// Output a text string out the user interface via the connected socket.
// DOES NOT ERASE THE STRING to a NULL!
bool UserCLI::DisplayString(string* s){
    string st;

    if (s->size() == 0)
        return true;  // nothing to do

    //ssOutBuffer
    if (MySocket.connected){
        st = *s;
        st = LocalTime() + "+++++++++++++++ " + st;
        MySocket.sendtext(*s);
        //cout << "Test--" << *s << endl;
    }

   *s = "";

    return true;

}

// Output a text string out the user interface via the connected socket.
// After outputting, it erases the string in the stream.
bool UserCLI::Display(stringstream* ss){
    string s;

    if (ss->str().size() == 0)
        return true;  // nothing to do

    //ssOutBuffer
    if (MySocket.connected){
        s = LocalTime() + " " + ss->str();
        MySocket.sendtext(s);
        //cout << "Test--" << *s << endl;
    }

   ss->str("");
   return true;
}



