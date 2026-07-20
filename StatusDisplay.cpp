/* 
 * File:   StatusDisplay.cpp
 * Author: Ryan Le
 * 
 * Created on July17, 2026, 8:14 AM
 */
#include "platform/thread/PlatformMutex.h"
#include "platform/thread/PlatformLockGuard.h"
#include "StatusDisplay.h"
#include "serialhandler.h"
#include "Cigorn.h"
#include "Matrix192x64.h"
#include "CommThread.h"
#include "Console.h"
#include "CommandLine.h"
#include "SiteManagement.h"
#include "SocketThread.h"

using namespace Communications;

// Constructor for the status display object.
StatusDisplay::StatusDisplay(void) {
   disabled = true;
   display_line = 0;
}

StatusDisplay::StatusDisplay(const StatusDisplay& orig) {

}

StatusDisplay::~StatusDisplay() {
}


void StatusDisplay::RedrawAll(void){
    if (disabled) return;

    MyLCD.CursorHome();
    MyLCD.PlaceString(APP_TITLE);
    ShowTime();

}

void StatusDisplay::Refresh(){

    Refresh(false);  // don't for a refresh

}
// Refresh the fields in the display that need updating. Assume the
// screen has been drawn with all the proper fixed fields.
void StatusDisplay::Refresh(bool forceRefresh){
    static int lastscreen = 0;
    static int lastline = 0;
    // Put the time onto the display.  
    if (disabled) return;

    ShowTime();

    if (lastscreen != screen_num)
        forceRefresh = true;

    if (lastline  !=  display_line)
        forceRefresh = true;
    
    // Show the system status via the LEDs
    MyLCD.setLED(SYSTEMLED, LEDGREEN);
    
    // Show LAN status
    if (GetSocketStatus().count_connected > 0 )
       MyLCD.setLED(NETLED, LEDGREEN);
    else
       MyLCD.setLED(NETLED, LEDYELLOW);

    switch (screen_num){
        case SCREEN_HOME:
            HomeScreen(forceRefresh);
            break;
        case SCREEN_STATUS:
            StatusScreen(forceRefresh);
            break;
        case SCREEN_NET:
            NetScreen(forceRefresh);
            break;
        case SCREEN_RADIO:
            RadioScreen(forceRefresh);
            break;

    }
    lastscreen = screen_num;  // remember the last screen we put up
    lastline = display_line;
}

void StatusDisplay::ShutDownScreen (int redraw){

    if (disabled) return;

    if (redraw)
        MyLCD.ClearDisplay();

    MyLCD.CursorHome();
    MyLCD.PlaceString(APP_TITLE);
    ShowTime();

    MyLCD.PlaceString("Version;",0,2, 0);  // Line 2
    MyLCD.PlaceString(VersionString(REV_MAJOR, REV_MINOR, REV_BUILD));

    MyLCD.PlaceString("Shutting down system",0,3, 0);  // Line 2
    MyLCD.PlaceString(VersionString(REV_MAJOR, REV_MINOR, REV_BUILD));

}


// The first screen shown upon power up
void StatusDisplay::HomeScreen (int redraw){
        int i;
        int x=0;
        string s = "";

        if (disabled) return;


        if (redraw) MyLCD.ClearDisplay();
        if (redraw) MyLCD.CursorHome();
        if (redraw) MyLCD.PlaceString(APP_TITLE);

        ShowTime();
        
        if (redraw) MyLCD.PlaceString("Version:",0,2, 0);  // Line 2
        if (redraw) MyLCD.PlaceString(VersionString(REV_MAJOR, REV_MINOR, REV_BUILD));

        if (redraw) MyLCD.PlaceString("Name:",0,3, 0);  // Line 2
        if (redraw) {
            MyLCD.PlaceString(Me.MyName);
            if (Me.IsChief){
                MyLCD.PlaceString(" ");
                MyLCD.PlaceString(sayMASTER);
            }
        }
//routecount
        MyLCD.PlaceString("Routed:",0 ,5 , 0);  // Line 4
        MyLCD.PlaceString(longToString(routecount),8,5, 0);  // Line 4

        i = DPL_WIDTH/2;
        MyLCD.PlaceString("Site Msg:",i, 5, 0);  // Line 4
        MyLCD.PlaceString(longToString(Cigorncount_in),(i+9),5, 0);  // Line 4

// Connected Sockets
        MyLCD.PlaceString("Sockets:",0 ,6 , 0);  // Line 5
        MyLCD.PlaceString(intToString(OurDevices.getSocketCount()),9,6, 0);  // Line 4

        i = DPL_WIDTH/2;
        s = intToString(OurDevices.getChannelCount()) + "/" + intToString(OurDevices.getActiveChannelCount());
        MyLCD.PlaceString("Radios:",i , 6, 0);  // Line 4
        MyLCD.PlaceString(s, (i+9),6, 0);  // Line 4


        i = DPL_WIDTH/2;
        if (display_line >= ipaddresses.size())
            display_line = ipaddresses.size()-1;
        if (display_line < 0)
            display_line = 0;

        IPaddList::iterator it;
        IPaddList al;  // make a copy because it is manipulated in a background task


    {
    cigorn::PlatformLockGuard lock(addlistlock);

    for (it = ipaddresses.begin(); it != ipaddresses.end(); ++it) {
        if (x == display_line) {
            MyLCD.PlaceString(it->second.interface, 1, 7, 0);  // Line 7
            MyLCD.PlaceString(it->second.ipaddress, i, 7, 0);   // Line 7
        }
        x++;
    }
}

 
    }



void StatusDisplay::StatusScreen(int redraw){
    int Line = 1;
    int mid;
    double d;
    string s;
    int prec = 2;

    if (disabled) return;

    mid = DPL_WIDTH/2 +1;

    if (redraw)
        MyLCD.ClearDisplay();

    MyLCD.CursorXY(1, 1);         // Line 2
    MyLCD.PlaceString(SC_STATUS_TITLE );

    Line = 3;
    d = (double)getCommLoopSpeed()/1000;
    if (d > 9) prec = 1;
    if (d > 99) prec = 0;
    MyLCD.PlaceString("Com OS:",1,Line ,0);  //
    MyLCD.PlaceString(doubleToString(d, prec),9,Line ,4);  // Place the number here
    MyLCD.PlaceString("kHz",13,Line ,0);  //
    d = (double)getTCPLoopSpeed()/1000;
    if (d > 9) prec = 1;
    if (d > 99) prec = 0;
    MyLCD.PlaceString("Net:",mid,Line ,0);  //
    MyLCD.PlaceString(doubleToString(d, prec),24,Line ,4);  // Place the number here
    MyLCD.PlaceString("kHz",mid+10,Line ,0);  //

    Line++;
    MyLCD.PlaceString("NMEA:",1,Line ,0);      // Number of NMEA messages in
    MyLCD.PlaceLong(nmeacount,8,Line ,8);      // Place the number here
    MyLCD.PlaceString("PRAV:",mid,Line ,0);    // Number of NMEA messages in
    MyLCD.PlaceLong(pravecount,mid+6,Line ,8); // Place the number here

    Line++;
    MyLCD.PlaceString("Cig:",0, Line, 0);  // Line 4
    MyLCD.PlaceLong(Cigorncount_in, 8, Line , 8);  // Line 4
    MyLCD.PlaceString("XML:", mid, Line, 0);    // Line 4
    MyLCD.PlaceLong(xmlcount_in, mid+5, Line , 8);  // Line 4

    Line++;
    MyLCD.PlaceString("tty Q:",1,Line ,0);       // Number messages in the tty queue
    MyLCD.PlaceLong(qTTYout.size(),10,Line ,8);  // Place the number here
    MyLCD.PlaceString("WMX:", mid, Line, 0);    // Line 4
    MyLCD.PlaceLong(wmxcount_in, mid+5, Line , 8);  // Line 4
    
    Line++;
    Line++;
   // MyLCD.PlaceString("eth Q:", 1, Line, 0);     // Number of messages in thr socket queue
   // MyLCD.PlaceLong(qETHout.size(), mid+5, Line ,6);  // Place the number here

}


void StatusDisplay::StatsScreen(int redraw){
    int Line = 1;

    if (disabled) return;  // teh status display is disable from use

    if (redraw)
        MyLCD.ClearDisplay();
    MyLCD.CursorXY(1, 1);         // Line 2
    MyLCD.PlaceString(SC_STATS_SCREEN );

    Line = 3;
    MyLCD.PlaceString("Com OS:",1,Line ,0);  //
    MyLCD.PlaceLong(getCommLoopSpeed()/1000,9,Line ,4);  // Place the number here
    MyLCD.PlaceString("kHz",13,Line ,0);  //


}

void StatusDisplay::NetScreen(int redraw){

  int x = 0;
  int LinesDisplayed = 0;
  int Line;
  int SockIndex;
  int mid;

  if (disabled) return;  // teh status display is disable from use

  Line = 1;

  mid = DPL_WIDTH/2 +3;

  if (redraw) MyLCD.ClearDisplay();

  // Put the title on this screen
  MyLCD.PlaceString(SC_NET_TITLE, 1, Line, 0 );

  Line++;
  Line++;
  MyLCD.PlaceString("Sockets:",1,Line ,0);      // Number of sockets created
  MyLCD.PlaceLong(OurDevices.getSocketCount(),mid,Line ,8);      // Place the number here

  Line++;
  MyLCD.PlaceString("Connected Sockets:",1,Line ,0);    // Number of NMEA messages in
  MyLCD.PlaceLong(OurDevices.getConnectedSocketCount(),mid,Line ,8);      // Place the number here

  Line++;
  MyLCD.PlaceString("Bytes in:",1,Line ,0);    // Number of NMEA messages in
  MyLCD.PlaceLong(OurDevices.getEthBytesIn(),mid,Line ,8);      // Place the number here

  Line++;
  MyLCD.PlaceString("Bytes out:",1,Line ,0);    // Number of NMEA messages in
  MyLCD.PlaceLong(OurDevices.getEthBytesOut(),mid,Line ,8);      // Place the number here
  

}


void StatusDisplay::RadioScreen(int redraw){
    int Line;
    int mid = DPL_WIDTH/2 +3;
    
    if (disabled) return;  // the  status display is disable from use

    if (redraw)
        MyLCD.ClearDisplay();
    MyLCD.CursorXY(1, 1);         // Line 2
    MyLCD.PlaceString(SC_RADIO_TITLE );

    Line = 3;
    MyLCD.PlaceString("# Radio Channels:", 1, Line ,0);      // Number of sockets created
    MyLCD.PlaceLong(OurDevices.getChannelCount(), mid, Line ,8);      // Place the number here

    Line++;
    MyLCD.PlaceString("Active Channels:", 1, Line ,0);      // Number of sockets created
    MyLCD.PlaceLong(OurDevices.getActiveChannelCount(), mid, Line ,8);      // Place the number here

    Line++;
    MyLCD.PlaceString("Hours since RX:", 1, Line ,0);      // Number of sockets created
    MyLCD.PlaceString(doubleToString(OurDevices.HoursSinceTtyMsg(), 2), mid, Line ,8);      // Place the number here

      Line++;
    MyLCD.PlaceString("Bytes in:",1,Line ,0);    // Number of NMEA messages in
    MyLCD.PlaceLong(OurDevices.getTtyBytesIn(), mid, Line , 8);      // Place the number here

    Line++;
    MyLCD.PlaceString("Bytes out:",1,Line ,0);    // Number of NMEA messages in
    MyLCD.PlaceLong(OurDevices.getTtyBytesOut(), mid, Line , 8);      // Place the number here


    //MyLCD.PlaceString("Epoch time:",1,Line ,0);  //
    //MyLCD.PlaceDouble(SiteManager.MyTicker.EpochTime(SiteManager.EpochTime) , 12,Line,8 );  // Place the number here
    //MyLCD.PlaceString("Slot:",22,Line ,0);  //
    //MyLCD.PlaceLongLJ(SiteManager.MyTicker.SlotNum(SiteManager.EpochTime, SiteManager.slottime), 28,Line,5 );  // Place the number here
 
}

void StatusDisplay::Initialize(rs232* p){
    // Run once when we first start the program.
    pStatTTY = p;                    // remember the port to use
    MyLCD.setPort(pStatTTY);         // Tell the LCD driver which serial port to use.
    MyLCD.Initialize();              // Initialize the LCD
    MyLCD.ClearDisplay();
    screen_num  = 0;                 // home screen
    disabled = false;                // we are OK

    MyLCD.ClearDisplay();
    MyLCD.PlaceString("RaveonNet");
    MyLCD.setLED( 0, LEDOFF);
    MyLCD.setLED( 1, LEDOFF);
    MyLCD.setLED( 2, LEDOFF);

    StatusDisplay::HomeScreen(REDRAWALL);
   // cout << "Initialized Status Display " << pStatTTY->devicename << endl;
}

void StatusDisplay::ShowTime(void){
    if (disabled) return;  // the status display is disable from use

    MyLCD.CursorXY(DPL_WIDTH-7, 1);
    MyLCD.PlaceString(LocalTime());

}

// Check to see if the user pressed a button on the front pannel
int StatusDisplay::CheckKeyboard(void){
    char button;
    char c;

    if (disabled) return -1;  // the status display is disable from use

    if (pStatTTY->GetChar(&c) == true){
        // there is a char available
        button = MyLCD.MapButton(c);  // get the generic code for the button on this LCD panel.
        switch (button)
        { // Which key was pressed??
          case BUT_RIGHT :
              // Next screen
              screen_num++;          // go to the next screen
              display_line = 0;
              if (screen_num > MAX_SCREENS)
                  screen_num = 0;
              break;
          case BUT_UP :
              display_line++;
              break;
          case BUT_LEFT :
              screen_num--;          // go to the next screen
              display_line = 0;
              if (screen_num < 0)
                  screen_num = MAX_SCREENS;
              break;
          case BUT_ENTER :
               break;
          case BUT_DOWN :
              if (display_line > 0)
                 display_line--;
              break;
          case BUT_TOP_LEFT :
               screen_num = 0;
               display_line = 0;
              break;
          case BUT_BOTTOM_LEFT :
              display_line = 0;
              break;
        }
        return button;
    }

    return NUL;  // no button pressed

}
