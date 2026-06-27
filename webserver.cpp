/* 
 * File:   webserver.cpp
 * Author: john
 * 
 * Created on December 25, 2010, 6:56 PM
 */

#include "webserver.h"
#include "emailer.h"
#include "Router.h"
#include <string.h>
#include "GlobalVar.h"
#include "ascii.h"
#include <stdio.h>
#include <iostream>
#include "functions.h"
#include "DeviceList.h"
#include "htmlformatter.h"
#include "mainsubs1.h"
#include "WebPages.h"
#include "Cigorn.h"

webserver::webserver() {
    httpfrom = "";
    webdata = "";
    mystate = web_startup;
    lastform.clear();
    lastformname = "";
    clientip = "";
    ShowIP = true;
    TimeSinceLastPage = 0;
    web_last_time = time(NULL);
    web_now_time = time(NULL);

}

webserver::webserver(const webserver& orig) {
}

webserver::~webserver() {
}

bool webserver::ProcessWebsite(void){

    static int laststate = web_startup;
    static double start_time;
    static double live_update_time;
    string PageRequested = "";
    bool busy = false;               // return busy true if we are sending an email.
    string s="";
    static string webpage="";
    int i;
    stringstream ss;

    
    // See if the socket is initalized. If it is, run it.
    if (MySocket.sockfd >= 0 ){
        MySocket.tcp_socket();   // always call this when connected to process the socket
    }

    web_now_time = time(NULL);
    if (web_last_time != web_now_time){
        web_last_time = web_now_time;
        // Do this stuff once per scond.
        TimeSinceLastPage++;   // tick up a second.
    }

    // Check the web server watchdog.
    if (TimeSinceLastPage > WEB_SERVER_TIMEOUT) {
        // FAIL??
        MySocket.DisconnectSocket();  // web server failed/stuck.  Restart it.
        CoutM2(ss) << "Web server stuck.  Restart." << endl;
        MyCLI.OutputText(ss.str());   // send the text to the console output
        TimeSinceLastPage = 0;   // restart the watchdog
        mystate = web_startup;
        return busy;
    }

    switch(mystate){
        case web_startup:
            MySocket.portnum = portnum;      //
            MySocket.myDevDesIndex = 0;      // no device designator for this socket
            MySocket.index = 0;              // no indeex either.
            MySocket.init_tries = 0;
            MySocket.interface = "eth0";     // always use eth0
            MySocket.protocol = pServer;     // TCP/Ip socket client
            MySocket.myDevType = dWEBserver;
            MySocket.initialize_socket();
            webpage="";
            mystate = web_startlisten;
            start_time = TimeNow();
            TimeSinceLastPage = 0;  // restart the watchdog timer
            httpfrom = "";  // we're not talking to anyone right now
            //cout << "Web Startup." << endl;
            break;
        case web_startlisten:
            busy = false;
            if (MySocket.sockfd >= 0 ){
                // The socket is listening
                mystate = web_idle;
            }else{
                elog.store("WEBserver failed to open port " + intToString(portnum));
                // cout << "WEBserver failed to open socket on port " << portnum << endl;
                mystate = web_failwait;
            }
            break;
        case web_idle:
           // This is the main idle state for the web server.  Stay here till we get a connection.
           busy = false;
           if (MySocket.sockfd == -1){
                mystate = web_fail;   // socket failed.  Wait a little and try again.
                CoutM2(ss) << "Web server socket restart" << endl;
           }else{
               if (MySocket.newsockfd > 0 ){
                    mystate = web_request;
                    busy = true;
               }
           }
           break;
        case web_request:
           // connetion in progress.  Look for a request
           start_time = TimeNow();
           if (MySocket.MyParser.rput > MySocket.MyParser.rget){
                busy = true;
                GetWEBdata();
                string FileName = trim(TheHTTP.param1);    // the file name the browser is requesting
                // TODO look for the CRLF at the end of the page request
                PageRequested = CheckForPage(FileName, PageFlagText);    // Parse the FileName string,and see if there is a ?page= and return parameter if there is
                CoutM2(ss) << "Web request in from:" << clientip <<  "  " << PageRequested  <<  " " << MySocket.sockfd << "|" << MySocket.newsockfd << endl;
                WebPages MyPages;
                MyPages.UserName = GetUserName(clientip);
                MyPages.ClientIP = clientip;
                MyPages.ShowIP = ShowIP;

                // SCIENCE project .  REMOVE!!!!!!!!!!!!!!!
                //cout << webdata << endl;
                //break;

                switch (TheHTTP.form_method){
                    case page_get:
                        mystate = web_sending;
                        if ((IPisLoggedIn(MySocket.ConnectedToIP) == false) && (trim(webusername).size() !=0)){
                            // this user is not logged in. Get his password
                            webpage = MyPages.LoginPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if(FileName == "/"){
                            // root directory (home page)
                            webpage = MyPages.HomePage(ShowIP);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "stats"){
                            // Create a page with our statistics on it
                            webpage = MyPages.StatsPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "elog"){
                            // Create a page with our error log on it
                            webpage = MyPages.ElogPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "logout"){
                            // user wants to log out
                            LogoutIP(MySocket.ConnectedToIP);
                            MyPages.UserName = "";
                            clientip = "";
                            webpage = MyPages.LoginPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "config"){
                            // Create a page with our configuration
                            webpage = MyPages.ConfigPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "devices"){
                            // Create a page with our configuration
                            webpage = MyPages.DevicesPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "cigorninfo"){
                            // Create a page with our configuration
                            webpage = MyPages.CigornInfoPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "devsetup"){
                            // Create a page with our configuration
                            webpage = MyPages.DevSetupPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "routes"){
                            // Create a page with our configuration
                            webpage = MyPages.RoutesPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "serial"){
                            // Create a page with our statistics on it
                            webpage = MyPages.SerialPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "net"){
                            // Create a page with our statistics on it
                            webpage = MyPages.NetworkPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "wd"){
                            // Create a page with our statistics on it
                            webpage = MyPages.WdPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "radio"){
                            // Create a page with our statistics on it
                            webpage = MyPages.RadioPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "wnat"){
                            // Create a page with our statistics on it
                            webpage = MyPages.WnatPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "sockets"){
                            // Create a page with our statistics on it
                            webpage = MyPages.SocketsPage(TheHTTP.param2);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "ports"){
                            // Create a page with our statistics on it
                            webpage = MyPages.PortsPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "memory"){
                            // Create a page with our statistics on it
                            webpage = MyPages.MemoryPage();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }else if (PageRequested == "test"){
                            // Create a page with our statistics on it
                            webpage = MyPages.TestPage1();
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }
                        else {//TheHTTP.param1
                            webpage = MyPages.Err404page(TheHTTP.param1);
                            TimeSinceLastPage = 0;   // restart the watchdog
                        }
                        busy = true;
                        break;
                    case page_form:
                        break;
                    case page_post:
                        if(lastformname == FORM_security){
                            // Someone is logging into us
                            string UN="";
                            if (SecurityFormData(UN)){
                                // OK login. Send them home
                                MyPages.UserName = UN;
                                webpage = MyPages.HomePage(ShowIP);
                                mystate = web_sending;
                                TimeSinceLastPage = 0;   // restart the watchdog
                            }else{
                                // invalid login
                            }
                        }
                }
                busy = true;
            }
            break;
        case web_sending:            
            webpage = WebPages::AddHeader(webpage);  // put the HTTP header on the page
            mystate = web_header_sent;
            break;
            
        case web_header_sent:
            // Send the web page out
            busy = true;
            if ((TimeNow() - start_time) > web_timeout){
                mystate = web_restart;   // failed to connect.  wait a little and try again.
                CoutM2(ss) << "session timeout" << endl;
            }
            if (MySocket.connected){
                if (MySocket.txcount == 0){
                    // We have room in the Socket buffer for more data
                    if (webpage.size() < MySocket.MaxBuffSize){
                        MySocket.sendbytes(webpage);
                        webpage = "";
                    }else{
                        s = webpage.substr(0, MySocket.MaxBuffSize-1);
                        MySocket.sendbytes(s);
                        webpage = webpage.substr( MySocket.MaxBuffSize-1, webpage.size() - (MySocket.MaxBuffSize-1) );
                    }
                }
            }else{
                mystate = web_restart;   // failed to connect.  wait a little and try again.
            }
            // See if the whole page is out
            if (webpage.size() == 0)
                 mystate = web_continue;
            break;
        case web_live:
            break;
        case web_query:
            if ((TimeNow() - start_time) > web_timeout)
                mystate = web_restart;   // failed to connect.  wait a little and try again.
            break;
        case web_fail:
            // We no longer have a connection
            mystate = web_failwait;           // go wait a little while before trying again.
            break;
        case web_failwait:
            if ((TimeNow() - start_time) > 1){
                mystate = web_restart;
            }
            break;
        case web_continue:
            // We had a successful transaction. continue using this socket
            // Always go through this state to continue with the connection
            mystate = web_restart;
            busy = true;
            CoutM2(ss) << "Web page out.  Continue serving." << endl;
            break;
        case web_restart:
            // Always go here to reset the socket and start over.
            //cout << "Restart Web Server Socket" << endl;
            MySocket.DisconnectClient();
            mystate = web_startup;
            busy = true;
            break;
    }

    // Output the debug text if there is any
    if (ss.str().size() > 0){
       MyCLI.OutputText(ss.str());   // send the text to the console output
       ss.str("");                   // clear the buffer
    }

    return busy;
}

// Parse the FileName string,and see if there is a ?flag= and return parameter if there is
string webserver::CheckForPage(string s, string flag){
    int x;
    int eos;
    string ret="";

    if (s.size() < 3 )
            return "";

    flag = trim(flag);
    string Rtext = "/?" + flag + "=";
    x = s.find(Rtext, 0);
    if (x >= 0) {
        eos = s.size()-1;
        // X is pointing to the location of the /  char. Now go to the = sign
        x = s.find("=",x);
        x= x + 1;  // move past the = sign
        if (eos > x){
            ret = s.substr(x,eos);
            ret = trim(ret);
        }
    }
    return ret;

};

// Pull the text out of the buffer, and store it in the response structures.
bool webserver::GetWEBdata(void){
    char buff[MAX_PARSE_BUFF];
    int Code = -1;
    int count;
    int linecount = 0;
    string s;
    string p1;
    string c1;
    bool foundblank = false;

    count = MySocket.MyParser.rput - MySocket.MyParser.rget;
    if (count > 0)
        MySocket.MyParser.ExtractData(buff, count);
    else
        return -1;

    clientip =  MySocket.ConnectedToIP;

    webdata =  ToString(buff, count);  // store the request
    TheHTTP.form_method = page_unknown;

    TheHTTP.iline = ExtractLine(webdata,1);             // get the first line
    TheHTTP.header[1] = ExtractLine(webdata,2);         // get the first header line
    TheHTTP.method = GetSubString(TheHTTP.iline, 1);
    TheHTTP.param1 = GetSubString(TheHTTP.iline, 2);
    TheHTTP.param2 = GetSubString(TheHTTP.iline, 3);

    //cout << "P1:" << TheHTTP.param1 << "  P2:" << TheHTTP.param2 << endl;

    // What type of HTTP page is this?
    if (StringToUpper(TheHTTP.method) == "GET" )
        TheHTTP.form_method = page_get;
    if (StringToUpper(TheHTTP.method) == "FORM" )
        TheHTTP.form_method = page_form;
    if (StringToUpper(TheHTTP.method) == "POST" ){
        TheHTTP.form_method = page_post;
        lastformname = TheHTTP.param1;
        // Get rid of leading / if it is there
        if ((lastformname.size() > 1) && (lastformname[0] == '/'))
            lastformname = lastformname.substr(1, lastformname.size()-1);
    }

    linecount = 1;


    std::stringstream ss;
    ss << webdata;

    // read each line in the web page
    while (ss.getline(buff, MAX_PARSE_BUFF))   {
        // cout << "line:" << linecount << " " << buff << endl;
        s = ToString(buff,MAX_PARSE_BUFF);
        if (trim(s).size() == 0)
            s="";
        if (s.size() > 0 ){
            c1 = StringToUpper(GetSubString(s, 1));  // first parameter
            p1 = GetSubString(s, 2);                 // its value
            switch (TheHTTP.form_method){
                case page_get:
                    break;
                case page_form:
                    break;
                case page_post:
                    if (foundblank){
                        // here comes the data for the post
                        ParseFormData(s);  // get the form data and put it in the lastform structure
                    }
                    break;
            }
        }else{
            foundblank = true;
            lastform.clear();  // new parameters comming in
        }

        linecount++;
    }
  return true;
}

// See if the user who sent this security data is logged in. If not, log him in
bool webserver::SecurityFormData(string &Uname){
  string pswd = "";
  string s;

  // Look for the IP in the list
  for (itr = Users.begin(); itr != Users.end(); itr++){
        if (itr->second.UserIP == clientip){
            return true;
        }
  }

  // User is not in our list. Is he valid?
  UserInfo NU;
  NU.UserIP = clientip;
  NU.UserName = "";
  s = GetFormDataField(PName_username);
  pswd = GetFormDataField(PName_password);
  if ((s == webusername) && (pswd == webpassword)){
      // Valid login or no user name required
     NU.UserName = s;   // get the data he typed into the field of the login form
     NU.LoginTime = TimeNow();
     Users.insert(make_pair(Users.size(), NU));
     Uname = s;
     cout << "New User:" << NU.UserName << endl;
     return true;
  }

  elog.store("Web Interface login failed. User:" + s);
  return false;

}

// Go through the data fields in the form, and get the field data for this field
string webserver::GetFormDataField(string fname){

    for (lfi = lastform.begin(); lfi !=  lastform.end(); lfi++){
      if (lfi->name == fname){
         return trim(lfi->response);   // get the data he typed into the field of the login form
      }
    }
   return "";
}

// read the data fields in the posted form response
void webserver::ParseFormData(string s){
    formfields fm;
    int fnum = 0;  // filed number
    int i = 0;
    bool done = false;
    int state = 0;

    if (s.size() == 0)
        return;

    formfields ff;
    ff.name = "";
    ff.response = "";

    i=0;
    while(i<s.size()){
        if (s[i] == '='){
            state = 1;  // read data state
            i++;
        }
        if ((s[i] == '&' ) || (s[i] == CR )){
            // done with this entry
            if (ff.name.size() > 0)
                lastform.push_back(ff);   // store the new form data in the structure
            //cout << "Formdata:" << ff.name << " is= " << ff.response << endl;
            ff.name="";
            ff.response = "";
            state = 0;
            i++;
        }
        if (i < s.size()){
          switch (state){
                case 0:
                    // Reading in the name
                    if (s[i] == '+')
                        ff.name += ' ';
                    else
                        ff.name = ff.name + s[i];
                    break;
                case 1:
                    // Reading in the data for field "name"
                    if (s[i] == '+')
                        ff.response += ' ';
                    else
                        ff.response = ff.response + s[i];
                    break;
            }
        }
        if (i == (s.size()-1)){
            // done with this complete line
            if (ff.name.size() > 0)
                lastform.push_back(ff);   // store the new form data in the structure
            // cout << "Formdata:" << ff.name << " is= " << ff.response << endl;
            ff.name="";
            ff.response = "";
            state = 0;
        }
        i++;

     }

}

// remove this client from our list
void webserver::LogoutIP(string IP){
    int i = -1;

    for (itr = Users.begin(); itr != Users.end(); itr++){
        if (itr->second.UserIP == IP){
            i = itr->first;
        }
    }
    if (i>=0){
        Users.erase(i);
    }
    
}

// see if this IP address is in our list of logged in users.
bool webserver::IPisLoggedIn(string ip){

    for (itr = Users.begin(); itr != Users.end(); itr++){
        if (itr->second.UserIP == ip){
            return true;
        }
    }
    return false;

}

// See if this IP address is in our list of logged in users.
// Return the user name if it is, or "" if not.
string webserver::GetUserName(string ip){

    for (itr = Users.begin(); itr != Users.end(); itr++){
        if (itr->second.UserIP == ip){
            return itr->second.UserName;
        }
    }
    return "";
}



