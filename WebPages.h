/* 
 * File:   WebPages.h
 * Author: john
 *
 * Created on December 31, 2010, 7:53 AM
 */

#ifndef WEBPAGES_H
#define	WEBPAGES_H

#include <string>

// Highlight colors
#define Color_Link      "#00207F"  // green
#define Color_Text      "#000000"  // black
#define Color_Heading   "#0B7C7C"
#define Color_Connected     "blue"
#define Color_Disconnected  "black"
#define Color_Parameters    "blue"

#define MaxSocketsPerPage    100
#define MaxWdPerPage         100

// Form tags
#define FORM_security     "cigorn_security_check"


using namespace std;

class WebPages {
public:
    WebPages();
    WebPages(const WebPages& orig);
    virtual ~WebPages();

    string HomePage(bool);
    string RadioPage(string);
    string LoginPage(void);
    string StatsPage(void);
    string CigornInfoPage(void);
    string SerialPage(void);
    string ElogPage(void);
    string ConfigPage(void);
    string DevSetupPage(void);
    string DevicesPage(void);
    string RoutesPage(void);
    string PortsPage(void);
    string MemoryPage(void);
    string NetworkPage(string);
    string WdPage(string);
    string WnatPage(string);
    string SocketsPage(string);
    static string AddHeader(string);
    string Err404page(string);
    string TemplatePage(string, string, int) ;
    string TemplatePage(string, string) ;
    string TestPage1(void);
    string Head(int);
    string BuildNetHTML(int, int);
    string BuildWdHTML(int, int);
    string BuildRadioHTML(int);
    string BuildWnatHTML(int);
    string BuildSocketsHTML(int, int);
    string BuildSocketsHTML(void);
    string BuildDevicesHTML(void);
    string BuildSerialHTML(void);
    string BuildDevSetupHTML(void);
    string BuildCigornInfoHTML(void);
    string BuildConfigHTML(void);
    string BuildRouteListHTML(void);
    string BuildElogHTML(void);
    string BuildPortsHTML(void);
    string BuildProcHTML(string, int, string,string,string,string);


    string UserName;
    string ClientIP;
    bool   ShowIP;

private:

};

#endif	/* WEBPAGES_H */

