/* 
 * File:   settings.cpp
 * Author: john
 * 
 * Created on August 26, 2010, 9:05 PM
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <stdio.h>
#include "Cigorn.h"
#include "CommThread.h"
#include "Console.h"
#include "CommandLine.h"
#include "WirelessNAT.h"
#include "sync-roles.h"

using namespace std;

// read the .ini file. Return number of lines, or -1 if invalid
int readini(std::string filename){
    string s="";
    string st = "";
    char fn[1000];
    int i = 0;

    to_cstring(fn, filename, 1000);

    ifstream inFile(fn);
   
    if (!inFile) {
        elog.store("Cannot read command file:" + filename);
        return -1; // terminate with error
    }

    // read each lin in the .ini file, and run them through the command-line interpreter.
    while (getline(inFile , s))   {
        if (s.size() > 0){
            cli.processCommand(s, dINI);     // process this command
            i++;
        }
    }

    inFile.close();
    return i;

}

// Read the configuration information for the email client myEmail
// read the settings from the DB table dt
void ConfigureWeb(datatable* dt, webserver* web){
    string s;

    s = dt->LookupData("webport" ,fld_param1);
    if (s.size() > 0){
        web->portnum = StringToInt(s);
        if (web->portnum <= 0 ){
             web->portnum  = DEFAULTWEBPORT;
        }
    }

    web->MySocket.protocol = pServer;  // we are a TCP server
    web->MySocket.description = "WEB server";
    web->MySocket.sockfd = -1;
    web->mystate = web_startup;
    web->MySecondSocket.protocol = pServer;  // we are a TCP server
    web->MySecondSocket.description = "WEB server";
    web->MySecondSocket.sockfd = -1;
}

// Read the configuration information for the email client myEmail
// read the settings from the DB table dt
void ConfigureEmail(datatable* dt, emailer* em){
    string s;
    stringstream ss;

    em->server = dt->LookupData("mailserver" ,fld_param1);
    if (em->server.size() > 0 )
        ss << "Email server:" << em->server << endl;

    em->username = dt->LookupData("mailusername" ,fld_param1);
    if (em->username.size() > 0 )
        ss << "Email username:" << em->username << endl;

    em->password = dt->LookupData("mailpassword" ,fld_param1);
    if (em->password .size() > 0 )
        ss << "Email password:" << em->password << endl;

    em->subject = dt->LookupData("mailsubject" ,fld_param1);
    if (em->subject.size() > 0 )
        ss << "Email subject:" << em->subject << endl;

    em->mailfrom = dt->LookupData("mailfrom" ,fld_param1);
    if (em->mailfrom.size() > 0 )
        ss << "Emails sent from:" << em->mailfrom << endl;

    //mailto
    em->mailto = dt->LookupData("mailto" ,fld_param1);
    if (em->mailto.size() > 0 )
        ss << "Emails sent to:" << em->mailto << endl;

    //smtptimeout
    s = dt->LookupData("smtptimeout" ,fld_param1);
    if (s.size() > 0){
        em->smtptimeout = StringToInt(s);
        ss << "SMTP timeout:" << em->smtptimeout << endl;
    }    

    s = dt->LookupData("mailport" ,fld_param1);
    if (s.size() > 0){
        em->portnum = StringToInt(s);
        if (em->portnum > 0 ){
            ss << "Email port:" << em->portnum << endl;
        }
        else{
            ss << "Email port: default" << endl;
        }
    }
    else
        em->portnum  = 25;

    MyCLI.OutputText(ss.str());
    ss.str("");

}

// Run through this table and get the configuration variables from the parameter fields
void ReadTable(datatable* dt){
    string s;
    double d;
    int i;
    string s1,s2,s3,s4,s5;

    i = 0;
    while (i < dt->rows.size()){
        s = dt->GetItem(i, dt->IndexCol );  // get the first variable
        if ( s.size() > 0 ){
           s1 = dt->GetItem(i, fld_param1);  // get the parameters as strings
           s2 = dt->GetItem(i, fld_param2);
           s3 = dt->GetItem(i, fld_param3);
           s4 = dt->GetItem(i, fld_param4);
           s5 = dt->GetItem(i, fld_param5);
           ConfigParameter(s, s1, s2, s3, s4, s5);  // configure the given variable with these parameters
        }
        i++;
    }
}

// check the configuration settings stored in the Database table dt  (usually dtSC)
void ConfigParameter(string thevariable, string s1, string s2, string s3, string s4, string s5){
    string s;
    string S1, S2, S3, var;
    stringstream ss;

    double d;
    int i;
    int p1,p2,p3,p4,p5;
 
    // Get the integer versions of the parameters
    var = trim(StringToUpper(thevariable));
    p1= StringToInt(s1);
    S1 = StringToUpper(s1);
    S1 = trim(S1);
    S2 = StringToUpper(s2);
    S2 = trim(S2);
    S3 = StringToUpper(s3);
    S3 = trim(S3);
    p2= StringToInt(s2);
    p3= StringToInt(s3);
    p4= StringToInt(s4);
    p5= StringToInt(s5);

    if ((var.size() == 0) || (s1.size() == 0))
        return;  // nothing to do with a null variable name

    //  Are we a hot-standby gateway
    if ((var == "GATEROLE") && (S1.size() > 0) ){
         if (S1 == StringToUpper("Standby")){
             // this is a standby gateway in a hot-standby configuration
             ss << "Set role to Standby Gateway . Primary=" << S2 << endl;
             Me.gaterole = Standby;
             Me.IsActive = false;     // for starters, assume we are not active.
             if (S2.size() > 0){
                 // User wants to set the Name of the Primary gateway that we will mirror
                 Me.PrimaryName = trim(S2);
             }
             if (S3.size() > 0){
                 // User wants to set the Name of the Primary gateway that we will mirror
                 PrimaryGWipadd = trim(S3);
             }

         }
         RoleUpdated();  // implement our new role
   }

   // Cutover time if we theink the Primary is down. And the interval beteen testing it.
   if (var == "HOTCUTOVERTIME"){
       i = StringToInt(s1);
       if ((i > 2) && (i < 9999))
            hotcutovertime = i;
       i = StringToInt(s2);
       if ((i > 2) && (i < 9999))
            PrimaryTestInterval = i;
   }


    if (var == "SITENAME"){
        Me.MyName = s1;
        return;
    }

    // set Debug message level
    if ((var == "MESSAGELEVEL") && (S1.size() > 0)){
         if ((S1 == "M0") || (p1 == 0))
             messagelevel = MSG_NONE;
         if ((S1 == "M1") || (p1 == 1))
             messagelevel = MSG_STATUS;
         if ((S1 == "M2") || (p1 == 2))
             messagelevel = MSG_DEBUG;
   }

   if ((var == "MASTER") || (var == "CHIEF")){
        if (s.size() > 0)
            Me.IsChief = IsStringTrue(s1);
   }

    if (var == "DBPUSHINTERVAL"){
        if (StringToInt(s1) >= MIN_DBPUSH)
            Me.dbPushInterval = StringToInt(s1);
        else
            Me.dbPushInterval = DEFAULT_DBPUSH;  // default interval
        return;
    }

    // Get the outbound message queue settings  maxQage
    if ((var == "MAXQAGE") || (var == "MAXQAGE")){
        i = StringToInt(s1);
        if (i > 1)
            maxQage =  i;
        else
            elog.store("Invalid maxQage setting.");
        return;
    }

    if (var == "MAXQCOUNT"){
        i = StringToInt(s1);
        if (i > 1)
            maxQcount =  i;
        else
            elog.store("Invalid maxQcount setting.");
        return;
    }

    // statusemailinterval
    if (var == "STATUSEMAILINTERVAL"){
        d = StringToDouble(s1);
        // range check
        if ((d >= .01) || (d=-1))
            statusemailinterval = d;
        return;
    }

    // SiteIdentifyInterval
    if (var == "SITEIDENTIFYINTERVAL"){
        i = StringToInt(s1);
        SiteIdentifyInterval = i;
        return;
    }

    // webusername
    if (var == "WEBUSERNAME"){
        webusername = trim(s1);
        return;
    }

    // webpassword
    if (var == "WEBPASSWORD"){
        webpassword = trim(s1);
        return;
    }

    if (var == "CONSOLEPORT"){
        consoleport = StringToInt(s1);
        return;
    }

    MyCLI.OutputText(ss.str());
    ss.str("");

}

