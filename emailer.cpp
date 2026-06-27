/* 
 * File:   email.cpp
 * Author: john
 * 
 * Created on December 18, 2010, 1:30 PM
 */

#include "emailer.h"
#include "Router.h"
#include <string.h>
#include "GlobalVar.h"
#include "ascii.h"
#include <stdio.h>
#include <iostream>
#include "functions.h"
#include "DeviceList.h"


double call_time;
double laststate_time;

emailer::emailer() {
   server = "";   // the IP address or url of the email server
   username = ""; // the user name to log into the email server
   password = ""; // the password to use when we log into the server
   subject = "";  // text to put on the subject line of every message from this server
   portnum = -1;  // text to put on the subject line of every message from this server
   MySocket.description = "SMTPsocket";
   mystate = smtp_idle;  // always start in this state
   MySocket.myDevType = dMAILserver;
   challenge = "";     // security challenge
   CountSent = 0;
   CountFailed = 0;
   CountTimout = 0;
   smtptimeout = smtp_con_timeout;
}

emailer::emailer(const emailer& orig) {
}

emailer::~emailer() {
}

bool emailer::Enabled(void){

    if ((server.size() > 0 ) && (username.size() > 0) && (password.size() > 0 ) && (portnum >0))
        return true;

    return false;

}
// true if busy. Call as fast and often as possible
// called in main loop
bool emailer::ProcessEmails(void){

    static int laststate = smtp_idle;
    static int statecount = 0;      // count how many times we call this in the same state
    static int okcount = 0;         // count how many times we get a OK response
    static int commandcount = 0;    // count SMTP commands sent this state
    bool busy = true;               // return busy true if we are sending an email. 
    string s;

    // See if the socket is initalized. If it is, run it. 
    if (MySocket.sockfd >= 0 )
        MySocket.tcp_socket();   // always call this when connected to process the socket

    switch(mystate){
        case (smtp_idle):
            if (emails.size() == 0){
                laststate = smtp_idle;
                statecount = 0;
                okcount = 0;
                laststate_time = TimeNow();  // time we last changes states
                commandcount = 0;            // count SMTP commands sent this state
                busy = false;                // nothing to do now.
            }
            else{
                mystate = smtp_callserver;      // start up state machine.
                CurrentEmail = emails.front();  // get the email content
                emails.pop();
            }
            break;
        case (smtp_callserver):
            // Try to connect our socket to the remote email SMTP server
            call_time = TimeNow();             // decimal seconds, mS resolution
            ServerResponses.clear();           // erase the memory of our exchanges
            if (call_server(server) == false)
                mystate = smtp_fail_wait;      // failed to connect.  Wait some time, and try again later.
            else{
                mystate = smtp_waitconnect;    // called OK.  Wait for the socket to connect
            }
            break;
        case (smtp_waitconnect):
            if (MySocket.connected){
               mystate = smtp_sendhelo;
            }
            if ((TimeNow() - call_time) > smtptimeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            break;
        case (smtp_sendhelo):
            if ((TimeNow() - laststate_time) > smtp_ex_timeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                if (GetResponse() == 220){
                    // Server responded OK and is ready to talk to us.
                    SendMessage("EHLO\r\n");   // send the HELO to the SMTP server
                    mystate = smtp_sendauth;
                }else{
                    // server did not respond correctly
                    elog.storeTrimmed("SMTP Error. Server response unknown." + LastResponse.message, MAX_ELOG_LEN);
               }
            }
            break;
        case (smtp_sendauth):
            // HELO was sent.  Now login if we get a 250 back
            if ((TimeNow() - laststate_time) > smtp_ex_timeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                switch(GetResponse()){
                    case 250:
                        //Requested mail action okay 'AUTH LOGIN' . "\r\n"
                        SendMessage("AUTH LOGIN\r\n");  // use login authorization
                        //MySocket.sendbytes("AUTH CRAM-MD5\r\n");  //
                        mystate = smtp_sendlogin;
                        okcount++;
                        break;
                    case 504:
                        // Unrecognized authentication type
                        elog.storeTrimmed("SMTP Error with server response: " + LastResponse.message, MAX_ELOG_LEN);
                        mystate = smtp_fail_wait;
                        break;
                    case 503:
                        // Bad sequence of commands
                        elog.storeTrimmed("SMTP Error with server response: " + LastResponse.message, MAX_ELOG_LEN);
                        mystate = smtp_fail_wait;
                        break;
                }
            }           
            break;
        case (smtp_sendlogin):
            // HELO was sent.  Now login if we get a 250 back smtp_sendauth
            if ((TimeNow() - laststate_time) > smtp_ex_timeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                commandcount++;
                switch(GetResponse()){
                    case 235:
                        // Authentication OK
                        mystate = smtp_sendmailto;
                        SendMessage("MAIL FROM:<" + mailfrom + ">\r\n");
                        break;
                    case 250:
                        //Requested mail action okay 'AUTH LOGIN' . "\r\n"
                        okcount++;
                        SendMessage("AUTH LOGIN\r\n");
                        break;
                    case 334:
                        // We got the challenge back from the server
                        s = StringToUpper(base64_decode(challenge));
                        if (FindString(s.c_str(), "PASS", s.size()) >= 0 )
                            SendMessage(base64_encode(password )+"\r\n");
                        else
                            if (FindString(s.c_str(), "USER", s.size()) >= 0)
                                SendMessage(base64_encode(username )+"\r\n");
                        break;
                    case 354:
                        okcount++;
                        commandcount++;  // go to the next command
                        mystate = smtp_sendmailto;
                        break;
                    case 535:
                        // Authentication failed
                        elog.storeTrimmed("SMTP Error. Authentication failed. Probably invalid username or password: " + LastResponse.message, MAX_ELOG_LEN);
                        mystate = smtp_fail_wait;
                        break;
                    case 503:
                        // Bad sequence of commands
                        elog.storeTrimmed("SMTP Error. Bad command sequence. Sent:"+ LastSent.message + "  From server:" + LastResponse.message, MAX_ELOG_LEN);
                        break;
                }
            }
            break;
        //
        case (smtp_sendmailto):
            // Main email server communication state.  Send messages, get responses, send more messages...
            if ((TimeNow() - laststate_time) > smtp_ex_timeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                 commandcount++;
                 switch(GetResponse()){
                    case 250:
                       //
                       okcount++;
                       break;
                    case 354:
                        okcount++;
                        mystate = smtp_send_content;
                        break;
                    case 503:
                        // Bad sequence of commands
                        mystate = smtp_send_content;  // give up and send the email body
                        break;
                    case 500:
                       // unrecognized command
                         elog.storeTrimmed("SMTP Error. Unrecognized command:" + LastSent.message + "  From server:" + LastResponse.message, MAX_ELOG_LEN);
                      break;
                }
               if (commandcount == 1)  SendMessage("RCPT TO: " + CurrentEmail.to + "\r\n");
               if (commandcount == 2)  SendMessage("DATA\r\n");
               if (commandcount >= 2) mystate = smtp_send_content;
            }
            break;

        case (smtp_send_content):
            // Main email server communication state.  Send messages, get responses, send more messages...
            if ((TimeNow() - laststate_time) > smtp_ex_timeout)
                mystate = smtp_fail_wait;   // failed to connect.  wait a little and try again.
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                switch(GetResponse()){
                    case 250:
                        break;
                    case 354:
                       SendMessage("Subject: " + CurrentEmail.subjectline + "\r\n");
                       SendMessage("MIME-Version: 1.0\r\n");
                       SendMessage("Content-Type: text/html charset=\"iso-8859-1\"\r\n"); //Content-Type: text/html; charset="iso-8859-1"
                       SendMessage(CurrentEmail.content + "\r\n");
                       mystate = smtp_senddot;
                       break;
                    case 503:
                        // Bad sequence of commands
                        elog.storeTrimmed("SMTP Error. Bad command sequence. Sent:"+ LastSent.message + "  From server:" + LastResponse.message, MAX_ELOG_LEN);
                        break;
                }
           }
            break;
        case (smtp_senddot):
            // Main email server communication state.  Send messages, get responses, send more messages...
            SendMessage(".\r\n");
            mystate = smtp_done;
            break;
       case (smtp_done):
            // Main email server communication state.  Send messages, get responses, send more messages...
            if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                // read the data and store the response
                switch(GetResponse()){
                    case 250:
                        //RCPT TO
                        SendMessage("QUIT\r\n");
                        //Requested mail action okay 'AUTH LOGIN' . "\r\n"
                         mystate = smtp_finished;
                       break;
                }
            }
            break;
        case (smtp_finished):
           CountSent ++;
           mystate = smtp_restart;
           LogComment("Success. Exchange completed in " + doubleToString((TimeNow() - call_time), 1) + "seconds.");
           break;
        case (smtp_timeout):
            CountTimout++;
            mystate = smtp_restart;
            LogComment("Failed. Exchange time out. ");
            break;
        case (smtp_fail_wait):
            // wait some time before we try again. 
            MySocket.DisconnectSocket();
            CountFailed++;
            LogComment("Failed. Exchange failed. ");
            mystate = smtp_restartwaiting;
            break;
        case (smtp_restartwaiting):
            if ((TimeNow() - call_time) > 5)
                mystate = smtp_restart;
            break;
        case (smtp_restart):
            // Always restart through this state
            mystate = smtp_idle;
            MySocket.DisconnectSocket();
            // MySocket.
            break;
    }

    // Keep track of how many times we stay in one state
    if (laststate != mystate){
        laststate = mystate;
        statecount = 0;
        okcount = 0;
        laststate_time = TimeNow();  // time we last changes states
        commandcount = 0;
    }else
        statecount++;

    return busy;

}

int emailer::LogComment(string s){
    LastSent.code = -1;
    LastSent.message = trim(s);
    LastSent.time = TimeNow();
    LastSent.CorS = '>';
    ServerResponses.insert(ServerResponses.begin(), LastSent);
}


int emailer::SendMessage(string s){
    LastSent.code = -1;
    LastSent.message = trim(s);
    LastSent.time = TimeNow();
    LastSent.CorS = 'c';
    ServerResponses.insert(ServerResponses.begin(), LastSent);
    MySocket.sendbytes(s);
}

// Pull the text out of the buffer, and store it in the response strucutres.
int emailer::GetResponse(void){
    char buff[MAX_PARSE_BUFF];
    int Code = -1;
    int count;

    LastResponse.code = -1;
    LastResponse.message = "";
    LastResponse.time = 0;
    challenge = "";

    count = MySocket.MyParser.rput - MySocket.MyParser.rget;
    if (count > 0)
        MySocket.MyParser.ExtractData(buff, count);
    else
        return -1;

    Code = ExtractFirstNum(buff, count);   // get the response code
    
    if (Code >= 0){
        LastResponse.code = Code;
        LastResponse.message = trim(ToString(buff, count));
        LastResponse.time = TimeNow();
        LastResponse.CorS = 's';
    }else
       Code = -1;

    switch(Code){
        case 334:
            // BASE64 encoded 'challenge'.
            challenge = GetSubString(LastResponse.message , 2);  // get the challenge string
            LastResponse.message = LastResponse.message + "(" + StringToUpper(base64_decode(challenge)) +")";
            break;
        case 503:
            // Bad sequence of commands

            break;
    }

    ServerResponses.insert(ServerResponses.begin(), LastResponse);

    return Code;
    
}

// Try to connect to the email server
bool emailer::call_server(string server_url){

    MySocket.hostport = portnum;     // the smtp port (typical 25) to connect to.
    MySocket.portnum = 49999;        // why not
    MySocket.myDevDesIndex = 0;         // no cevice designator for this socket
    MySocket.index = 0;              // no indeex either.
    MySocket.init_tries = 0;
    MySocket.interface = "eth0";     // always use eth0
    MySocket.hostaddress = server;
    MySocket.protocol = pClient;     // TCP/Ip socket client

    return MySocket.initialize_socket();    // go initialize the socket
    
};

/*  SMTP Commands
HELO      - Initial State Identification
MAIL- Mail Sender Reverse Path
RCPT      - One Recipient’s Forward Path
DATA      - Mail Message Text State
RSET      - Abort Transaction and Reset all buffers
NOOP     - No Operation
QUIT- Commit Message and Close Channel
*/

// Text versions of the response codes
string  emailer::response_code(int code)
{
    switch (code) {
        case 211: return "System status, or system help reply\r\n";
        case 214: return "Help message\r\n";
        case 220: return "%s Service ready\r\n";
        case 221: return "%s Service closing transmission channel\r\n";
        case 250: return "Requested mail action okay, completed\r\n";
        case 251: return "User not local; will forward to nowhere.net\r\n";
        case 354: return "Start mail input; end with <CRLF>.<CRLF>\r\n";
        case 421: return "Not Available. Closing transmission channel\r\n";
        case 450: return "Requested mail action not taken: mailbox unavailable\r\n";
        case 451: return "Requested action aborted: local error in processing\r\n";
        case 452: return "Requested action not taken: insufficient system storage\r\n";
        case 500: return "Syntax error, command unrecognized\r\n";
        case 501: return "Syntax error in parameters or arguments\r\n";
        case 502: return "Command not implemented\r\n";
        case 503: return "Bad sequence of commands\r\n";
        case 504: return "Command parameter not implemented\r\n";
        case 550: return "Requested action not taken: mailbox unavailable\r\n";
        case 551: return "User not local; please try nowhere.net\r\n";
        case 552: return "Requested mail action aborted: exceeded storage allocation\r\n";
        case 553: return "Requested action not taken: mailbox name not allowed\r\n";
        case 554: return "Transaction failed\r\n";
        default:  return "";
    }
}

#include <iostream>
#include <string>
#include <assert.h>

std::string emailer::base64_encode(const std::string& src){
    static const char base64_table[] ={
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/"
    };
    std::string dst;
    dst.reserve(((src.size()+2)/3)*4);

    for(unsigned src_pos = 0; src_pos < src.size(); src_pos += 3){
        const unsigned filling =
        (src_pos + 3)>src.size() ?
        src.size() - src_pos :
        3;

        dst += base64_table[unsigned(src[src_pos + 0] & 0xFC) >> 2]; // FC = 11111100
        if(filling == 1){
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4)]; // 03 = 11
            dst += '=';
            dst += '=';
        }
        else if(filling == 2)
        {
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4) | (unsigned(src[src_pos + 1] & 0xF0) >> 4)]; // 03 = 11
            dst += base64_table[((src[src_pos + 1] & 0x0F) << 2) | (unsigned(src[src_pos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
            dst += '=';
        }
        else
        {
            assert(filling == 3);
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4) | (unsigned(src[src_pos + 1] & 0xF0) >> 4)]; // 03 = 11
            dst += base64_table[((src[src_pos + 1] & 0x0F) << 2) | (unsigned(src[src_pos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
            dst += base64_table[src[src_pos + 2] & 0x3F]; // 3F = 111111
        }
    }
    return dst;
}

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
   return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}

std::string emailer::base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

