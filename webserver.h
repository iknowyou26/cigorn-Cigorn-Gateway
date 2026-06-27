/* 
 * File:   webserver.h
 * Author: john
 *
 * Created on December 25, 2010, 6:56 PM
 */

#ifndef WEBSERVER_H
#define	WEBSERVER_H

#include <string>
#include <queue>
#include "TCPsocket.h"
#include <vector>

enum webstates{
    web_startup,
    web_startlisten,
    web_idle,
    web_request,
    web_query,
    web_sending,
    web_header_sent,
    web_live,
    web_fail,
    web_failwait,
    web_continue,
    web_restart
    };

struct objhttp{
    string iline;      // The linitial line of the http message
    string method;     // the method specified int he initial line
    int    form_method;// an index to the type of method (post, get, form...)
    string param1;     // the file name or first parameter of the iline
    string param2;     // the second parameter
    map <int,string> header;   // the header lines
    string body;               // the body of the message
};

//
struct formfields{
    string name;
    string response;
};

struct UserInfo{
    string UserName;
    string UserIP;
    double LoginTime;
};

// The types of web pages that may come in
#define page_none      -1
#define page_unknown    0
#define page_get        1
#define page_form       2
#define page_post       3
#define WEB_SERVER_TIMEOUT  60

#define OKresponse "HTTP/1.0 200 OK"
#define web_timeout 2

#define PName_username    "j_username"
#define PName_password    "j_password"

#define PageFlagText      "page"

class webserver {
public:
    webserver();
    webserver(const webserver& orig);
    virtual ~webserver();
    bool ProcessWebsite(void);
    bool GetWEBdata(void);     // put the web data from the parser and store it in the webdata string
    string AddHeader(string);
    bool IPisLoggedIn(string);
    string GetUserName(string);
    void ParseFormData(string);
    bool SecurityFormData(string&);
    string GetFormDataField(string);
    void LogoutIP(string);
    string CheckForPage(string, string);

    int mystate;
    int portnum;
    tcpnet MySocket;         // the socket object to communicate with
    tcpnet MySecondSocket;  // the socket object to communicate
    string webdata;   // the last data to come into our web port
    objhttp TheHTTP;  // The parsed HTTP obejct
    string httpfrom;  // who is looking at our website
    string clientip;  // the IP address/port of the client we talk to
    bool ShowIP;      // true if we want to display the IP address on the web interface

    map <int, UserInfo> Users;      // list of logged in users
    map <int, UserInfo>::iterator itr;
    vector<formfields> lastform;    // the fields in the last form that we sent out, and the reponses to them, if we got a response
    vector<formfields>::iterator lfi;
    string lastformname;
    int TimeSinceLastPage;            // used for watchdog of the web server
    time_t web_now_time;
    time_t web_last_time;

private:

};

#endif	/* WEBSERVER_H */

