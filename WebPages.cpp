/* 
 * File:   WebPages.cpp
 * Author: john
 * 
 * Created on December 31, 2010, 7:53 AM
 */

#include "WebPages.h"
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
#include "CommThread.h"
#include "Router.h"
#include "procinfo.h"
#include "SocketThread.h"
#include "sync-roles.h"


using namespace std;

WebPages::WebPages() {
    UserName = "";
    ClientIP = "";
    ShowIP = true;
}

WebPages::WebPages(const WebPages& orig) {
}

WebPages::~WebPages() {
}

string WebPages::NetworkPage(string ItemStr){
    string s = "";
    int Item = 0;

    if (StringLeft(ItemStr, 1) == "s"){
        Item = GetIntVal(ItemStr);
        s = s + BuildNetHTML(Item, MaxSocketsPerPage);
    }else
        s = s + BuildNetHTML(0, MaxSocketsPerPage);

    return TemplatePage(s, Me.MyName, 15);

    // return TemplatePage(s, "Network Connections", 15);   // refresh the Net Stats every 5 seconds
}

// display wireless devices
string WebPages::WdPage(string ItemStr){
    string s = "";
    int Item = 0;

    if (StringLeft(ItemStr, 1) == "s"){
        Item = GetIntVal(ItemStr);
        s = s + BuildWdHTML(Item, MaxSocketsPerPage);
    }else
        s = s + BuildWdHTML(0, MaxSocketsPerPage);

    return TemplatePage(s, Me.MyName, 0);

    //return TemplatePage(s, "Wireless Device Status", 0);   // no refresh
}


string WebPages::RadioPage(string ItemStr){
    string s;
    int Item;

    if (ItemStr.size() > 0 )
        Item = StringToInt(ItemStr);
    else
        Item = 0;

    s = s + BuildRadioHTML(Item);

    return TemplatePage(s, Me.MyName, 15 );// refresh the Net Stats every 5 seconds

   // return TemplatePage(s, "Radio Channel Statistics", 15);   // refresh the Net Stats every 5 seconds
}


string WebPages::WnatPage(string ItemStr){
    string s;
    int Item;

    if (ItemStr.size() > 0 )
        Item = StringToInt(ItemStr);
    else
        Item = 0;

    s = s + BuildWnatHTML(Item);
   // return TemplatePage(s, "Wireless Network Address Translation", 15);   // refresh the Net Stats every 5 seconds
      return TemplatePage(s, Me.MyName, 15 );// refresh the Net Stats every 5 seconds
}


string WebPages::SocketsPage(string ItemStr){
    string s = "";
    int Item = 0;

    if (ItemStr.size() > 0 ){
        Item = GetIntVal(ItemStr);

        s = s + BuildSocketsHTML(0, MAXSOCKETS);
    }else
        s = s + BuildSocketsHTML();

    //return TemplatePage(s, "Connected Sockets", 15);   //
    return TemplatePage(s, Me.MyName, 15 );// refresh the Net Stats every 5 seconds
}


string WebPages::StatsPage(void){
    string s;
    s = s + BuildStatsHTML();
    //return TemplatePage(s, "General Statistics", 15);
    return TemplatePage(s, Me.MyName, 15 );// refresh the Net Stats every 5 seconds
}

// Device Communications
string WebPages::DevicesPage(void){
    string s;

    s = s + BuildDevicesHTML();
    // return TemplatePage(s, "Device Designator Statistics", 15);
    return TemplatePage(s, Me.MyName, 15 );// refresh the Net Stats every 5 seconds
}

string WebPages::CigornInfoPage(void){
    string s;

    s = s + BuildCigornInfoHTML();
    return TemplatePage(s, Me.MyName );// refresh the Net Stats every 5 seconds
    //return TemplatePage(s, "Cigorn Operational Info");
}


string WebPages::ConfigPage(void){
    string s;

    s = s + BuildConfigHTML();
    //return TemplatePage(s, "General Gateway Configuration");
    return TemplatePage(s, Me.MyName);// refresh the Net Stats every 5 seconds
}


string WebPages::MemoryPage(void){
    string s;

    s = s + BuildProcHTML("meminfo", 3, "Parameter", "Value", "", "");
    return TemplatePage(s, Me.MyName);
}

//devsetup
string WebPages::DevSetupPage(void){
    string s;
    s = s + BuildDevSetupHTML();
    return TemplatePage(s, Me.MyName);

}


string WebPages::ElogPage(void){
    string s;
    s = s + BuildElogHTML();
    return TemplatePage(s, Me.MyName);

}

string WebPages::SerialPage(void){
    string s;

    s = s + BuildSerialHTML();
    return TemplatePage(s, Me.MyName, 5);
}

string WebPages::RoutesPage(void){
    string s;

    s = s + BuildRouteListHTML();
    return TemplatePage(s, Me.MyName);
}


string WebPages::PortsPage(void){
    string s;
    s = s + BuildPortsHTML();
    return TemplatePage(s, Me.MyName);
}

string WebPages::Err404page(string f){
    string s;
    s = "<html> <body> <h1> 404 error. Page Not found </h1>";
    s = s + "Page:" + f;
    s = s + "</body> </html>";
    return s;
}


// Put the standard HTTP header onto the string s
string WebPages::AddHeader(string s){

    std::stringstream ss;

    ss << OKresponse << endl;
    ss << "Date: " << LocalDate() << " " << LocalTime() << endl;
    ss << "Content-Type: text/html" << endl;

    ss << s;

    return ss.str();

}


// Build the HOME page
string WebPages::HomePage(bool ShowTheIP) {

    string s;
    std::stringstream ss;
    htmlformatter fm;

    fm.ToTable("Site Name", fm.FormatedText( Me.MyName, 4, COLOR_BLACK, "b"));
    fm.ToTable("Active Now?", fm.FormatedText( BoolToString(Me.IsActive), 4, COLOR_BLUE, "b"));  // bold blue

    fm.ToTable("Software Version",  GetVersionText());
    if (ShowTheIP)
        fm.ToTable("This site's IP address",  Me.IPadd);
    if (MyCLI.MySocket.connected)
        fm.ToTable("CLI user's IP",  MyCLI.MySocket.ConnectedToIP);

    if (Me.gaterole == Standby){
        s = GetRole(Me.gaterole) + " (Primary=" + Me.PrimaryName +")";
        fm.ToTable("My Role",  s);
        fm.ToTable("Active Now?",  BoolToString(Me.IsActive));
        fm.ToTable("In sync with Primary?",  BoolToString(InSyncWithPrimary));      // TimeOfLastSync
        fm.ToTable("DB synchronized at",  DateTimeString(TimeOfLastPrimaryDBSync) );   // TimeOfLastSync
        if (Me.IsActive)
            s = "FAILED";
        else
            if (SecondsSincePrimaryOK < (PrimaryTestInterval*2) )
               s = "Good";
            else
               s = "Failing";
  
        fm.ToTable("Primary gateway health ",  s );   // TimeOfLastSync

    }else{
        s = GetRole(Me.gaterole);
        fm.ToTable("My Role",  s);
        fm.ToTable("Active Now?",  BoolToString(Me.IsActive));
    }
    
    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Cigorn Gateway Home Page", tag_caption) << endl;
    //ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    return TemplatePage(ss.str(), Me.MyName);

}


// Build the Login page
string WebPages::LoginPage() {

    string s;
    std::stringstream ss;
    htmlformatter fm;

    fm.ToTable("Site Name",  Me.MyName);
    fm.ToTable("Software Version",  GetVersionText());
    fm.ToTable("This site's IP address",  Me.IPadd);

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Cigorn Gateway Home Page", tag_caption) << endl;
    //ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    //<FORM METHOD=POST ACTION="j_security_check">
    ss << fm.FormOpen(FORM_security) << endl;  // the name of the form we are creating
    ss << html_endl << endl;
    // <font size="2"> <strong> Enter user ID and password: </strong></font>
    ss << "<font size=\"2\"> <strong> Enter user ID and password: </strong></font>" << endl;
    ss << html_linebrk << endl;
    ss << "<strong> User ID</strong> <input type=\"text\" size=\"20\" name=\"" << PName_username << "\">" << endl;
    ss << "<strong> Password </strong>  <input type=\"password\" size=\"20\" name=\"" << PName_password << "\">";
    ss << html_linebrk << endl;
    ss << "<font size=\"2\">  <strong> And then click this button: </strong></font>" << endl;
    ss << "<input type=\"submit\" name=\"login\" value=\"Login\">" << endl;
    ss << html_linebrk << endl;
    ss << fm.TagClose(tag_form) << endl;

    return TemplatePage(ss.str(), "Please Enter User ID and Password");

}


// Build the Login page
string WebPages::TestPage1() {

    string s;
    std::stringstream ss;
    htmlformatter fm;
    ss << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"  \"http://www.w3.org/TR/html4/loose.dtd\"" << endl;
    ss << "<html>" << endl;
    ss << Head(0) << endl;
    ss << "<body bgcolor=\"#bababa\" text=\"333366\" marginewidth = \"20\" marginheight=\"10\" " ;
    ss << " alink= #FF00F0" << " text=" << Color_Text << ">" << endl;
    ss << "<h1> General Configuration </h1>";

    ss << fm.TagOpen(tag_form) << endl;

    
    fm.ToTable("System Name", "RF 7800W" );
    fm.ToTable("IP Address","192.168.23.100");
    fm.ToTable("Subnet Mask","255.255.0.0");

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("General Configuration", tag_caption) << endl;
    //ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableParmsToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    //<FORM METHOD=POST ACTION="j_security_check">
    ss << fm.FormOpen(FORM_security) << endl;  // the name of the form we are creating
    ss << html_endl << endl;
    // <font size="2"> <strong> Enter user ID and password: </strong></font>
    ss << "<input type=\"submit\" value=\"send\"> <INPUT type=\"reset\">" << endl;
    ss << html_linebrk << endl;
    ss << fm.TagClose(tag_form) << endl;

    return ss.str();

}



// Gteway Network status
string WebPages::BuildElogHTML(void){

    string s;
    int i;
    std::stringstream ss;
    htmlformatter fm;

     vector <string>::iterator it;

     // output the error log entries
     for (i=elog.ErrorMessages.size()-1; i>=0; i--){
        fm.ToTable(intToString(i+1), elog.ErrorMessages[i]);
     }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Gateway Error Log", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Entry #", tag_tableheader)+fm.htmlformat("Entry Text",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    return ss.str(); //


}
//


// Gateway Socket status (Network connection list)
string WebPages::BuildSocketsHTML(void){

    return BuildSocketsHTML(0, MAXSOCKETS);

}

// Gateway Socket status (Network connection list)
string WebPages::BuildSocketsHTML(int First, int number){
    std::stringstream ss;
    htmlformatter fm;
    string s = "";
    string s1 = "";
    string remoteIP="";
    string color="";
    string byteinfo = "";
    string msgcount = "";
    int count = 0;

    int i;

    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].myDevDesIndex >= 0){
             if ((i >= First) && (i <= (count + number))){
                 if  (tcpsockets[i].RemotePort > 0)
                     s = intToString(tcpsockets[i].RemotePort);
                 else
                     s = "";
                 if (tcpsockets[i].connected){
                     color = Color_Connected;
                     s1 = intToString(tcpsockets[i].portnum);
                     remoteIP = tcpsockets[i].ConnectedToIP + ":" + intToString(tcpsockets[i].RemotePort);
                     byteinfo = LongToString(tcpsockets[i].bytes_in) + "/" + LongToString(tcpsockets[i].bytes_out);
                     msgcount = LongToString(tcpsockets[i].msg_in) + "/" + LongToString(tcpsockets[i].msg_out);
                     fm.ToTable(fm.FormatedText(tcpsockets[i].description, 3,color),
                                fm.FormatedText(s1, 3,color),
                                fm.FormatedText(remoteIP, 3,color),
                                fm.FormatedText(byteinfo, 3,color),
                                fm.FormatedText(msgcount, 3,color));
                 }
              }
          }
        }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Connected Socket List", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Dev. Des.", tag_tableheader)
            + fm.htmlformat("My Port",tag_tableheader)
            + fm.htmlformat("Remote IP",tag_tableheader)
            + fm.htmlformat("Bytes In/Out",tag_tableheader)
            + fm.htmlformat("Messages In/Out",tag_tableheader)
             ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    return ss.str(); //  fixed width font

}



// Gateway Socket status (Network connection list)
string WebPages::BuildNetHTML(int First, int number){
    std::stringstream ss;
    std::stringstream links;

    htmlformatter fm;
    string s, s1;
    string color;
    string ips="";
    int i;
    int SockCount=0;
    int count = 0;
    int linkcount = 0;

    // Build up a list of network interfaces
    IPaddList::iterator it;

    for (it = ipaddresses.begin(); it!= ipaddresses.end(); it++){
        fm.ToTable("Interface: " + FixedRight(it->second.interface, 5), it->second.ipaddress);
     }
  
    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Network Interface List", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Interface", tag_tableheader)+fm.htmlformat("IP Address", tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;
    
    // Now list the network stats
    fm.mytable.clear();
    fm.ToTable("Network sockets used ", longToString(SocketCount()));
    fm.ToTable("Network sockets connected ", longToString(ConnectedSocketCount()));
    fm.ToTable("Network bytes sent out ", longToString(SocketByteCount()));

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Network Statistics", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;
    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;


    // Now list the things we are connected to
    fm.mytable.clear();

    
    SockCount = SocketCount();
    if (SockCount< MaxSocketsPerPage)
        SockCount = MaxSocketsPerPage;

    for (i = 0; i < SockCount; i = i + MaxSocketsPerPage - 1){
        links << "<a href=\"/?page=net&s=" << i << " \">" << intToString(i+1) << "-" << intToString(i+MaxSocketsPerPage-1) << "</a>" << "  ";
        linkcount++;
        if (linkcount > 3){
            links << html_linebrk;
            linkcount = 0;
        }
    }

    ss << fm.htmlformat(fm.htmlformat("Display Socket Items   " + links.str(), tag_center), tag_small );
    ss << html_linebrk;

   // ss << "<a href=\"/?page=net&s=1 \">" + fm.htmlformat("1 - 50", tag_center);
    // ss << "<a href=\"/?page=net&s=50 \">" + fm.htmlformat("50 - 100", tag_center) + " </a>" << endl;


    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].myDevDesIndex >= 0 ){
             if ((i >= First) && (count <= number)){
                 count ++;
                 if  (tcpsockets[i].RemotePort >0)
                     s = intToString(tcpsockets[i].RemotePort);
                 else
                     s = "";
                 if (tcpsockets[i].connected){
                     color = Color_Connected;
                     ips = tcpsockets[i].ConnectedToIP;
                 }
                 else{
                     color = Color_Disconnected;
                     ips = "";
                 }
                 switch (tcpsockets[i].protocol){
                    case pServer:
                        break;
                    case pClient:
                        ips = tcpsockets[i].hostIPaddress;
                        break;
                    case pDGRAMTX:
                        ips = tcpsockets[i].hostIPaddress;
                        break;
                    case pDGRAMRX:
                        break;

                }// switch

                 s1 = tcpsockets[i].interface + " port " + intToString(tcpsockets[i].portnum);
                 
                 ips = tcpsockets[i].ConnectedToIP;
                 fm.ToTable(fm.FormatedText(tcpsockets[i].description, 3,color),
                            fm.FormatedText(s1, 3,color),
                            fm.FormatedText(ips, 3,color),
                            fm.FormatedText(s, 3,color));
            }
        }
    }

    if ((First > 0 ) || (number < 200))
       s = "Network Socket Connection List Items " + intToString(First+1) + " thru " + intToString(First + number - 1);
    else
       s = "Network Socket Connection List";

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat(s, tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Device Des.", tag_tableheader)
            + fm.htmlformat("My Port",tag_tableheader)
            + fm.htmlformat("Remote IP",tag_tableheader)
            + fm.htmlformat("Remote Port",tag_tableheader)
             ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    return ss.str(); //  fixed width font

}



// List of Wireless Devices on the system
string WebPages::BuildWdHTML(int First, int number){
    std::stringstream ss;
    std::stringstream links;

    htmlformatter fm;
    string s, s1, s2, s3, s4, sID;
    string color;
    int i, itemcount, ID;
    int count = 0;
    int linkcount = 0;
    int IDlow, IDup;

    // Build up a list of network interfaces

    ss << fm.TagOpen(tag_table,"border=\"1\"");

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    fm.mytable.clear();

    itemcount = dtWD->rows.size();
    if (itemcount < MaxWdPerPage)
        itemcount = MaxWdPerPage;

    for (i = 0; i < itemcount; i = i + MaxWdPerPage){
        links << "<a href=\"/?page=wd&s=" << i << " \">" << intToString(i) << "-" << intToString(i+MaxWdPerPage) << "</a>" << "  ";
        linkcount++;
        if (linkcount > 3){
            links << html_linebrk;
            linkcount = 0;
        }
    }

    ss << fm.htmlformat(fm.htmlformat("Display Items   " + links.str(), tag_center), tag_small );
    ss << html_linebrk;

    IDlow = 0;
    IDup = 9999;
    itemcount = dtWD->rows.size();

    // Loop through the table of our Wireless Devices using the map iterator (dit)
    count = 0;
    i=0;
    for (dtWD->dit = dtWD->rows.begin(); dtWD->dit != dtWD->rows.end(); dtWD->dit++){
       ID = StringToInt(dtWD->GetItem(dtWD->dit->first, fld_ID));
       i = i+ 1;
       if ((i >= First) && (count <= number)){
           count ++;
           sID = intToString(ID) + " (" + fm.FormatedText(IntToHex(ID,4), 3,color) + "hex)";
           s1 = dtWD->GetItem(dtWD->dit->first, fld_tmLastMsg);
           s2 = dtWD->GetItem(dtWD->dit->first, fld_rssi);
           s3 = dtWD->GetItem(dtWD->dit->first, fld_countFm);
           s4 = dtWD->GetItem(dtWD->dit->first, fld_countTo);
           fm.ToTable(fm.FormatedText(sID, 3,color),
                        fm.FormatedText(s1, 3,color),
                        fm.FormatedText(s2, 3,color),
                        fm.FormatedText(s3, 3,color),
                        fm.FormatedText(s4, 3,color));
       }
    }

    if ((First > 0 ) || (number < 200))
       s = "Wireless Devices Items " + intToString(First+1) + " thru " + intToString(First + number - 1);
    else
       s = "Wireless Devices List";

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat(s, tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Wireless ID", tag_tableheader)
            + fm.htmlformat("Last Message Time",tag_tableheader)
            + fm.htmlformat("RSSI",tag_tableheader)
            + fm.htmlformat("# From",tag_tableheader)
            + fm.htmlformat("# To",tag_tableheader)
             ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    return ss.str(); //  fixed width font

}



// Gateway Network status
string WebPages::BuildConfigHTML(void){

    string s;
    std::stringstream ss;
    htmlformatter fm;

    fm.ToTable("Site Name",  Me.MyName);
    fm.ToTable("Chief Site Y/N",  BoolToString(Me.IsChief));
    fm.ToTable("Software Version",  GetVersionText());
    fm.ToTable("Compile Date",  __DATE__ );
    fm.ToTable("Console port", intToString(consoleport));
    if (ShowIP)
        fm.ToTable("This site's IP address",  Me.IPadd);

    if (emailnotice)
        s = "YES";
    else
        s = "no";
    fm.ToTable("Send Status via Email",  s);
    fm .ToTable("Database Update Interval (sec)", intToString(Me.dbPushInterval));

    if (Me.gaterole == Standby){
        s = GetRole(Me.gaterole) + " (Primary=" + Me.PrimaryName +")";
        fm.ToTable("My Role",  s);
        fm.ToTable("Wait time before going active",  intToString(hotcutovertime) );   // TimeOfLastSync
        fm.ToTable("Primary Check Interval(sec) ",  intToString(PrimaryTestInterval ) );   // TimeOfLastSync
    }else{
        s = GetRole(Me.gaterole);
        fm.ToTable("My Role",  s);
    }

    fm.ToTable("Router Entry Count", intToString( DataRouter.RouteCount()));
    fm.ToTable("Wireless Devices", intToString(dtWD->rows.size()));
    fm.ToTable("Site Identifier Interval (sec)", intToString(SiteIdentifyInterval));

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("General Gateway Configuration", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    return ss.str(); //  fixed width font

}


// Gateway Network status
string WebPages::BuildCigornInfoHTML(void){

    int i=0;
    int j=-1;
    int k=-1;
    string s;
    std::stringstream ss;
    htmlformatter fm;

    fm.ToTable("Site Name",  Me.MyName);
    fm.ToTable("Chief Site Y/N",  BoolToString(Me.IsChief));
    fm.ToTable("Software Version",  GetVersionText());
    fm.ToTable("# of Gateways we hear ", intToString(GateWays.size()));
    fm.ToTable("Size of Message In queue ", intToString(qMSGin.size()));
    fm.ToTable("Size of TTY Out queue ", intToString(qTTYout.size()));
    fm.ToTable("Size of ETH Out queue ", intToString(qETHout.size()));
    i = GetFileDescrptorLimit();
    if (i >=0 ){
        fm.ToTable("Linux File Descriptor Max ", intToString(i));
    }
    fm.ToTable("Cigorn Max open IP Socket ", intToString(MAXSOCKETS));
    fm.ToTable("Max number tty ports ", intToString(MAX_TTY ));
    fm.ToTable("Route History Map size ", intToString( DataRouter. RecentRouteCount()));

    // Find the largest socket Q and report on it.
    k = -1;
    j = -1;
    for (i=0; i<MAXSOCKETS; i++){
          if (tcpsockets[i].myDevDesIndex >= 0){
              if ((int)tcpsockets[i].MsgQout.size() > j){
                 j = tcpsockets[i].MsgQout.size();
                 k = i;
             }
          }
        }
    if (k >= 0)
       fm.ToTable("Largest Socket Out queue ", intToString(j) + "(" + tcpsockets[k].description + ")");

    k = -1;
    j = -1;
    for (i=0; i< MAX_TTY; i++){
       if (COMport[i].handle >= 0){
             if ((int)COMport[i].MsgQout.size() > j){
                 j = COMport[i].MsgQout.size();
                 k = i;
             }
       }
    }
    if (k >= 0)
       fm.ToTable("Largest TTY Out queue ", intToString(j) + "(" + COMport[k].devicename + ")");

    fm.ToTable("My Directory",  GetMyDirectory());


    ss << fm.TagOpen(tag_table,"border=\"1\"");
//    ss << fm.htmlformat("Cigorn Operational Information", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Parameter", tag_tableheader)+fm.htmlformat("Value",tag_tableheader) ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    return ss.str(); //  fixed width font

}


// Build a page to describe the /procs/file information
string WebPages::BuildProcHTML(string file, int num, string h1, string h2, string h3, string h4){
    std::stringstream ss;
    htmlformatter fm;

    // Build up a list of the IP ports in use
    procinfo MyProcInfo;
    MyProcInfo.ReadProcFile(file);  // get the port information

    for (MyProcInfo.mitr = MyProcInfo.MyFields.begin(); MyProcInfo.mitr != MyProcInfo.MyFields.end(); MyProcInfo.mitr++){
        switch (num){
            case 1:
               fm.ToTable(intToString(MyProcInfo.mitr->first),
                    MyProcInfo.mitr->second.Field1);
                break;
            case 2:
               fm.ToTable(intToString(MyProcInfo.mitr->first),
                    MyProcInfo.mitr->second.Field1,
                    MyProcInfo.mitr->second.Field2);
                break;
            case 3:
               fm.ToTable(intToString(MyProcInfo.mitr->first),
                    MyProcInfo.mitr->second.Field1,
                    MyProcInfo.mitr->second.Field2,
                    MyProcInfo.mitr->second.Field3);
                break;
            case 4:
               fm.ToTable(intToString(MyProcInfo.mitr->first),
                    MyProcInfo.mitr->second.Field1,
                    MyProcInfo.mitr->second.Field2,
                    MyProcInfo.mitr->second.Field3,
                    MyProcInfo.mitr->second.Field4);
                break;
        }
    }
    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Proc Information from "+file, tag_caption) << endl;
    switch (num){
        case 1:
          ss << fm.htmlformat(fm.htmlformat(fm.htmlformat(" #", tag_tableheader)
            +fm.htmlformat(h1, tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;
            break;
        case 2:
          ss << fm.htmlformat(fm.htmlformat(fm.htmlformat(" #", tag_tableheader)
            +fm.htmlformat(h1, tag_tableheader)
            +fm.htmlformat(h2,tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;
            break;
        case 3:
          ss << fm.htmlformat(fm.htmlformat(fm.htmlformat(" #", tag_tableheader)
            +fm.htmlformat(h1, tag_tableheader)
            +fm.htmlformat(h2,tag_tableheader)
            +fm.htmlformat(h3,tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;
            break;
        case 4:
          ss << fm.htmlformat(fm.htmlformat(fm.htmlformat(" #", tag_tableheader)
            +fm.htmlformat(h1, tag_tableheader)
            +fm.htmlformat(h2,tag_tableheader)
            +fm.htmlformat(h3,tag_tableheader)
            +fm.htmlformat(h4,tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;
            break;
    }

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    return ss.str(); //
}

// Gteway Network status
string WebPages::BuildPortsHTML(void){

    std::stringstream ss;
    htmlformatter fm;

    // Build up a list of the IP ports in use
    procinfo MyPortInfo;
    MyPortInfo.ReadMyPorts();  // get the port information
    for (MyPortInfo.itr = MyPortInfo.MyPortInfo.begin(); MyPortInfo.itr != MyPortInfo.MyPortInfo.end(); MyPortInfo.itr++){
        fm.ToTable(intToString(MyPortInfo.itr->first),
                    MyPortInfo.itr->second.MyIP4,
                    intToString(MyPortInfo.itr->second.MyPort),
                    MyPortInfo.itr->second.RemAddress,
                    intToString(MyPortInfo.itr->second.RemPort));
    }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Local Network Ports", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat(" #", tag_tableheader)
            +fm.htmlformat("Local IP", tag_tableheader)
            +fm.htmlformat("Port",tag_tableheader)
            +fm.htmlformat("Remote IP",tag_tableheader)
            +fm.htmlformat("Rem Port",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;
    
    return ss.str(); //

}


// Gteway Network status
string WebPages::BuildSerialHTML(void){

    std::stringstream ss;
    htmlformatter fm;
    string s;
    string dd;
    int i;

   // Build up a list of serial interfaces
    for (i=0; i< MAX_TTY; i++){
       if (COMport[i].handle >= 0){
          if (COMport[i].DSRin())
             s = "YES";
          else
             s = "no";
          dd =  OurDevices.getDevDes(COMport[i].myDevIndex);
          fm.ToTable(dd,
                     COMport[i].devicename ,
                     intToString(COMport[i].baudrate),
                     s,
                     intToString(COMport[i].bytes_in),
                     intToString(COMport[i].bytes_out));
       }
   }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Serial Interface List", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Device Designator", tag_tableheader)
            +fm.htmlformat("Interface", tag_tableheader)
            +fm.htmlformat("Baud",tag_tableheader)
            +fm.htmlformat("DSR",tag_tableheader)
            +fm.htmlformat("Bytes In",tag_tableheader)
            +fm.htmlformat("Bytes Out",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);

    ss << html_linebrk;

    return ss.str(); //  

}

string WebPages::BuildRouteListHTML(void){
     map<unsigned long, routehistory>::iterator it;
     string s = "";
     routeEntry re;

     std::stringstream ss;
     htmlformatter fm;
     int i = 1;
     bool done = false;

    // Loop through the route table to see if this message should be routed.

     while ((done == false) && (i <= DataRouter.RouteCount())){
        // Get an entry from the router table
        re = DataRouter.RouteTableEntry(i);
        if (re.srcDevDes.size() > 0  ){
            if (re.lowerID >= 0){
                s = IDToString(re.lowerID) + " - " + IDToString(re.upperID) ;
            }
            else{
                s = "[ALL]";
            }
            fm.ToTable(intToString(i), re.srcDevDes, re.dstDevDes, DataRouter.ProtocolName(re.format), s," ");
        }else
            done = true;
        i++;
     }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("List of Routes", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Route #", tag_tableheader)
            +fm.htmlformat("Source",tag_tableheader)
            +fm.htmlformat("Destination",tag_tableheader)
            +fm.htmlformat("Protocol",tag_tableheader)
            +fm.htmlformat("IDs to Route",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;

    return ss.str(); //

}


string WebPages::BuildWnatHTML(int Item){
     string s = "";
     string ports = "";
     string ids="";
     WNATEntry wne;
     int BasePort;

     std::stringstream ss;
     htmlformatter fm;
     int i = 0;
     bool done = false;


     // Loop through the route table to see if this message should be routed.

     while (i < WNAT.WNATentries.size()){
        // Get an entry from the router table
        wne = WNAT.WNATentries[i];
        BasePort = OurDevices.getPortNum(OurDevices.IndexOf(wne.Designator));
        ports = intToString(BasePort) + "-" + intToString(BasePort + wne.PortCount);
        ids = IDToString(wne.lowerID) + "-" + IDToString(wne.lowerID + wne.PortCount);
        fm.ToTable(wne.Designator, ports, ids, wne.DefaultDevDes);
        i++;
     }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Wireless NAT Mappings", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Dev, Des.", tag_tableheader)
            +fm.htmlformat("TCP Ports",tag_tableheader)
            +fm.htmlformat("Wireless IDs",tag_tableheader)
            +fm.htmlformat("Default DevDes",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;

    return ss.str(); //

}


// Configuration of all of the devices
string WebPages::BuildDevSetupHTML(void){
     map<unsigned long, routehistory>::iterator it;
     string s = "";
     string t = "";
     string p = "";
     routeEntry re;
     string color;

     std::stringstream ss;
     htmlformatter fm;
     int i = 1;
     bool done = false;
     int SocketIndex = -1;

    // Loop through the route table to see if this message should be routed.

    for (i = 0; i < MAXDEVDES ; i++){
        if ((OurDevices.devicetypes[i] != dNONE) && (OurDevices.getBinding(i) >= 0) &&  (OurDevices.devicetypes[i] != dStatDisplay)){
            // This is a valid device, so list its settings.
            p="";
            s="";
            t = OurDevices.descriptions[i];
            if (OurDevices.IsTTY(i)){
                s = "RS232";
                if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                    //
                    p = intToString(COMport[OurDevices.getBinding(i)].baudrate) + " "
                        + COMport[OurDevices.getBinding(i)].parity + " "
                        + intToString(COMport[OurDevices.getBinding(i)].databits) + " "
                        + intToString(COMport[OurDevices.getBinding(i)].stopbits);
                    fm.ToTable(OurDevices.designator[i],
                        OurDevices.DeviceName(OurDevices.devicetypes[i]),
                        OurDevices.interfaces[i],
                        fm.FormatedText(p, 3,color),
                        fm.FormatedText(s, 3,color),
                        fm.FormatedText(t, 3,color) );
                }
            }

            if (StringLeft(OurDevices.getInterface(i), 3) == "eth"){
                SocketIndex = OurDevices.getBinding(i);
                if ((SocketIndex < MAXSOCKETS) && (SocketIndex > 0)){
                    s = "";
                    p = "";
                    switch (tcpsockets[SocketIndex].protocol){
                        case pServer:
                            s = "TCP Server ";
                            p = "Port:" + intToString(tcpsockets[OurDevices.getBinding(i)].portnum);
                            if (OurDevices.getBindCount(i) > 1)
                                p = p + "-" + intToString(tcpsockets[OurDevices.getBinding(i)].portnum
                                      + OurDevices.getBindCount(i) - 1);
                            break;
                        case pClient:
                            p = tcpsockets[OurDevices.getBinding(i)].hostaddress + ":" + intToString(tcpsockets[OurDevices.getBinding(i)].hostport);
                            if (OurDevices.getBindCount(i) > 1){
                                p = p + "-" + intToString(tcpsockets[OurDevices.getBinding(i)].portnum
                                      + OurDevices.getBindCount(i) - 1);
                            }
                            s = "TCP Client";
                            break;
                        case pDGRAMTX:
                            p = tcpsockets[OurDevices.getBinding(i)].hostaddress + ":" + intToString(tcpsockets[OurDevices.getBinding(i)].hostport);
                            if (OurDevices.getBindCount(i) > 1)
                                p = p + "-" + intToString(tcpsockets[OurDevices.getBinding(i)].portnum
                                      + OurDevices.getBindCount(i) - 1);
                            s = "UDP Sender";
                            break;
                        case pDGRAMRX:
                            p = tcpsockets[OurDevices.getBinding(i)].hostaddress + ":" + intToString(tcpsockets[OurDevices.getBinding(i)].hostport);
                            if (OurDevices.getBindCount(i) > 1)
                                p = p + "-" + intToString(tcpsockets[OurDevices.getBinding(i)].portnum
                                      + OurDevices.getBindCount(i) - 1);
                            s = "UDP Listener";
                            break;
                        }
                    fm.ToTable(fm.FormatedText(OurDevices.designator[i], 3,color),
                            fm.FormatedText(OurDevices.DeviceName(OurDevices.devicetypes[i]), 3,color),
                            fm.FormatedText(OurDevices.interfaces[i], 3,color),
                            fm.FormatedText(p, 3,color),
                            fm.FormatedText(s, 3,color),
                            fm.FormatedText(t, 3,color) );
              }
          }

        }
    }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Device Designator Settings", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("DevDes", tag_tableheader)
            +fm.htmlformat("Device",tag_tableheader)
            +fm.htmlformat("Interface",tag_tableheader)
            +fm.htmlformat("Settings",tag_tableheader)
            +fm.htmlformat("Protocol",tag_tableheader)
            +fm.htmlformat("Description",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;

    return ss.str(); //

}



// Status of all of the devicesdesignators
string WebPages::BuildDevicesHTML(void){
     string s = "";
     string t = "";
     string p = "";
     routeEntry re;
     string color;
     int socknum;

     std::stringstream ss;
     htmlformatter fm;
     int i = 1;
     bool done = false;

    // Loop through the devicedesignator list and show the dd with sockets that are connected

    for (i = 0; i < MAXDEVDES ; i++){
        if ((OurDevices.devicetypes[i] != dNONE) && (OurDevices.getBinding(i) >= 0) &&  (OurDevices.devicetypes[i] != dStatDisplay)){
            // This is a valid device, so list its settings.
            if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                if (OurDevices.IsTTY(i)){
                    p = "Serial";
                    if (COMport[OurDevices.getBinding(i)].DSRin())
                         color = Color_Connected;
                    else
                         color = Color_Disconnected;
                }else{
                    socknum = OurDevices.getBinding(i);
                    if (tcpsockets[socknum].connected == true)
                         color = Color_Connected;
                    else
                         color = Color_Disconnected;
                }
                p = LongToString(OurDevices.BytesInThisMin[i]) + "/" + LongToString(OurDevices.BytesOutThisMin[i]);
                s = LongToString(OurDevices.getBytesIn(i)) + "/" + LongToString(OurDevices.getBytesOut(i) );
                t = OurDevices.getHealth(i);
                fm.ToTable(
                    fm.FormatedText(OurDevices.designator[i], 3,color),
                    fm.FormatedText(OurDevices.DeviceName(OurDevices.devicetypes[i]), 3,color),
                    fm.FormatedText(OurDevices.getInterface(i), 3,color),
                    fm.FormatedText(p, 3,color),
                    fm.FormatedText(s, 3,color),
                    fm.FormatedText(t, 3,color) );
            }
        }
    }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Device Communications", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Dev. Des.", tag_tableheader)
            +fm.htmlformat("Dev Type",tag_tableheader)
            +fm.htmlformat("Interface",tag_tableheader)
            +fm.htmlformat("Bytes In/Out per Min",tag_tableheader)
            +fm.htmlformat("Bytes In/Out",tag_tableheader)
            +fm.htmlformat("Status",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;

    return ss.str(); //

}

// Status of all of the radio channels
string WebPages::BuildRadioHTML(int Item){
     string s = "";
     string t = "";
     string p = "";
     string act = "";
     routeEntry re;
     string color;
     int socknum=0;
     double activity;

     std::stringstream ss;
     htmlformatter fm;
     int i = 1;
     bool done = false;

    // Loop through the devicedesignato list and show the dd with sockets that are connected

    for (i = 0; i < MAXDEVDES ; i++){
        if (((OurDevices.getDevTypeIndex(i) == dDataModem) || (OurDevices.getDevTypeIndex(i) == dWMXmodem))){
            // This is a valid radio modem (base station)
            p="";
            s="";
            if (OurDevices.IsTTY(i)){
                p = "Serial";
                if ((OurDevices.getBinding(i) >=0 ) && (OurDevices.getBinding(i) < MAX_TTY)){
                    if (COMport[OurDevices.getBinding(i)].DSRin())
                         color = Color_Connected;
                    else
                         color = Color_Disconnected;
                    p = LongToString(COMport[OurDevices.getBinding(i)].msg_in) + "/" + LongToString(COMport[OurDevices.getBinding(i)].msg_out);
                    activity = COMport[OurDevices.getBinding(i)].ActivityTime();
                    if (activity >= 0)
                        s = doubleToString(activity, 2);
                    else
                        s = "--";
                    t = OurDevices.getHealth(i);
                    
                    fm.ToTable(
                        fm.FormatedText(OurDevices.designator[i], 3,color),
                        fm.FormatedText(intToString(OurDevices.channels[i]), 3,color),
                        fm.FormatedText(OurDevices.interfaces[i], 3,color),
                        fm.FormatedText(p, 3,color),
                        fm.FormatedText(s, 3,color),
                        fm.FormatedText(t, 3,color));
                }
            }

            if (StringLeft(OurDevices.getInterface(i), 3) == "eth"){
 
                    // coutm2 << i << " Radio " << endl;

                    t = OurDevices.getHealth(i);
                    socknum = OurDevices.getBinding(i);

                    if ((socknum >= 0 ) && (socknum < MAXSOCKETS)){
                        if (tcpsockets[socknum].connected == true)
                             color = Color_Connected;
                        else
                             color = Color_Disconnected;
                        activity = tcpsockets[socknum].ActivityTime();
                        if (activity >= 0)
                           act = doubleToString(activity, 2);
                        else
                           act = "--";
                        p = LongToString(tcpsockets[socknum].msg_in) + "/" + LongToString(tcpsockets[socknum].msg_out);
                    }else{
                        color = Color_Disconnected;
                    }
                    
                   
                    fm.ToTable(fm.FormatedText(OurDevices.getDevDes(i), 3,color),
                            fm.FormatedText(intToString(OurDevices.channels[i]), 3,color),
                            fm.FormatedText(OurDevices.getInterface(i), 3,color),
                            fm.FormatedText(p, 3,color),
                            fm.FormatedText(t, 3,color),
                            fm.FormatedText(act, 3,color) );
              }
          }

       
    }

    ss << fm.TagOpen(tag_table,"border=\"1\"");
    ss << fm.htmlformat("Radio Channels", tag_caption) << endl;
    ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Dev. Des.", tag_tableheader)
            +fm.htmlformat("Chan #",tag_tableheader)
            +fm.htmlformat("Interface",tag_tableheader)
            +fm.htmlformat("Msg In/Out",tag_tableheader)
            +fm.htmlformat("Status",tag_tableheader)
            +fm.htmlformat("Activity",tag_tableheader)
            ,tag_tablerow), tag_thread) << endl;

    ss << fm.TagOpen(tag_tablebody);
    // Build the html version of the table
    ss << fm.TableToHTML(tag_txt_fixed) << endl;
    ss << fm.TagClose(tag_tablebody);
    ss << fm.TagClose(tag_table);
    ss << html_linebrk;

    ss << "'In/Out'    is the number of messages In and Out of the radio." << html_linebrk;
    ss << "'Activity'  is the number of hours since the last message." << html_linebrk;

    return ss.str(); //

}



// Create the template for the page. No auto refresh
string WebPages::TemplatePage(string LeftCol, string lt) {

    return TemplatePage(LeftCol, lt, 0);

}

string WebPages::TemplatePage(string LeftCol, string lt, int RefreshRate) {
   std::stringstream ss;
   htmlformatter fm;
   // <!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
   ss << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << endl;
   ss << "<html>" << endl;
   ss << Head(RefreshRate) << endl;
   ss << "<body bgcolor=#FFFFFF link=" << Color_Link << "vlink=" << Color_Link;
   ss << " alink= #FF00F0" << " text=" << Color_Text << ">" << endl;
   ss << "<h1> Cigorn Gateway </h1>";

   ss << fm.TagOpen(tag_table, "border=\"1\" width=\"960\" ") << endl;
   ss << fm.htmlformat("Gateway Manager", tag_caption) << endl;
   ss << fm.htmlformat(fm.htmlformat(fm.htmlformat("Pages", tag_tableheader)+fm.htmlformat(lt, tag_tableheader) ,tag_tablerow), tag_thread) << endl;
   ss << fm.TagOpen(tag_tablebody);
   ss << fm.TagOpen(tag_tablerow, "valign=\"top\" ");

   // The left Column (List of pages to view)
   ss << "<td style=\"text-align: right; white-space: nowrap;\" width=\"190\"  >" << endl;
   ss << "<span style=\"color:#00FFF; font-weight:bold;\">" << endl;

   ss << "<a href=\"/\" >" + fm.htmlformat("Home",tag_center) + " </a>" << endl;
   ss << html_linebrk << endl;

   // --Statistics--
   ss <<  fm.htmlformat(fm.FormatedText("--Statistics--", 4, Color_Heading), tag_center) << endl;
   ss << "<a href=\"/?page=stats\" >" + fm.htmlformat("Gateway Statistics",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=devices\" >" + fm.htmlformat("Device Communications",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=radio\" >" + fm.htmlformat("Radio Channels", tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=sockets\" >" + fm.htmlformat("Connected Sockets", tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=wd\" >" + fm.htmlformat("Wireless Devices", tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=elog\" >" + fm.htmlformat("Error Log",tag_center) + " </a>" << endl;
   ss << html_linebrk << endl;

   // --Settings--
   ss <<  fm.htmlformat(fm.FormatedText("--Settings--", 4, Color_Heading), tag_center) << endl;
   ss << "<a href=\"/?page=config\" >" + fm.htmlformat("Gateway Config",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=routes\" >" + fm.htmlformat("Router",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=devsetup\" >" + fm.htmlformat("Device Designators",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=net\" >" + fm.htmlformat("Network Connections", tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=serial\" >" + fm.htmlformat("Serial Connections", tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=wnat\" >" + fm.htmlformat("Wireless NAT",tag_center) + " </a>" << endl;
   ss << html_linebrk << endl;

   // --Info--
   ss <<  fm.htmlformat(fm.FormatedText("--Info--", 4, Color_Heading), tag_center) << endl;
   ss << "<a href=\"/?page=ports\" >" + fm.htmlformat("Local Port Usage",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=memory\" >" + fm.htmlformat("Memory Status",tag_center) + " </a>" << endl;
   ss << "<a href=\"/?page=cigorninfo\" >" + fm.htmlformat("Cigorn Info",tag_center) + " </a>" << endl;

   // Science project test page for Harris  Remove from Cigorn
   ss << "<a href=\"/?page=test\" >" + fm.htmlformat("--",tag_center) + " </a>" << endl;


   ss << fm.TagClose(tag_font);
   ss << "</span>" << endl;
   ss << "</td>" << endl;

   // The right Column (information to display to the user
   ss << fm.TagOpen(tag_tabledata);
   ss << LeftCol<<  html_linebrk;
   ss << fm.TagClose(tag_tabledata);


   ss << fm.TagClose(tag_tablerow);
   ss << fm.TagClose(tag_tablebody);
   ss << fm.TagClose(tag_table);

   // The footer
   ss << fm.FormatedText(LocalTimeStamp(), 2,"blue") << endl;
   ss << fm.FormatedText("   |   ", 3,"green") << endl;
   if (ShowIP){
      ss << fm.FormatedText("IP:" + Me.IPadd, 2,"blue") << endl;
      ss << fm.FormatedText("   |   ", 3,"green") << endl;
   }
   if (UserName.size() > 0 ){
      ss << fm.FormatedText("User:" + UserName, 2,"blue") << endl;
      ss << fm.FormatedText("   |   ", 3,"green") << endl;
      ss << fm.FormatedText("<a href=\"/?logout\" >logout</a>", 2,"green") << endl;
      //ss << "<a href=\"/?logout\" >logout</a>" << endl;
   }
   else
      ss << fm.FormatedText("Not logged in", 2,"blue") << endl;

   ss << "</body> </html>";

   return ss.str();
}

string WebPages::Head(int RefreshRate){
    std::stringstream ss;
    std::stringstream st;

    htmlformatter fm;
    string s;

    ss << fm.TagOpen(tag_head);
    ss << fm.htmlformat("Cigorn Gateway",tag_title);
    ss << fm.TagOpenClose(tag_meta, "name=\"description\" content=\"Cigorn Gateway\"") << endl;
    ss << fm.TagOpenClose(tag_meta, " http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"") << endl;
    ss << fm.TagOpenClose(tag_meta, "http-equiv=\"Pragma\" content=\"no-cache\"") << endl;
    ss << fm.TagOpenClose(tag_meta, "HTTP-equiv=\"Cache-Control\" content=\"no-cache\"") << endl;
    if (RefreshRate > 0){
        st << "META HTTP-EQUIV=\"refresh\" CONTENT=\"" << RefreshRate << "\"";
        ss << fm.TagOpen(tag_meta,st.str()) << endl;
    }


    ss << fm.TagClose(tag_head) << endl;
    return ss.str();
}

