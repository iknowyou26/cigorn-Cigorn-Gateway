/* 
 * File:   StatusDisplay.h
 * Handle the local statud display. Usually an LCD panel.
 * Author: john sonnenberg
 *
 * Created on August 14, 2010, 8:14 AM
 */

#ifndef _STATUSDISPLAY_H
#define	_STATUSDISPLAY_H

#include "StatusDisplay.h"
#include "serialhandler.h"
#include "Matrix192x64.h"


#define  SYSTEMLED   0
#define  RADIOLED    1
#define  NETLED      2

// The generic button definitions
#define BUT_UP          0x42   // up arrow
#define BUT_RIGHT       0x43   // right arrow
#define BUT_LEFT        0x44   // up arrow
#define BUT_ENTER       0x45   // Enter/center
#define BUT_DOWN        0x48   // Down arrow
#define BUT_TOP_LEFT    0x41   // down arrow
#define BUT_BOTTOM_LEFT 0x47   // down arrow

using namespace Communications;

#define SCREEN_HOME       0
#define SCREEN_STATUS     1
#define SCREEN_NET        2
#define SCREEN_RADIO      3


#define MAX_SCREENS  3

#define REDRAWALL         1
#define UPDATEFIELDS       0

#define SC_HOME_TITLE    "Home Screen"
#define SC_STATUS_TITLE  "System Status"
#define SC_NET_TITLE     "Network Status"
#define SC_RADIO_TITLE   "Wireless Status"
#define SC_STATS_SCREEN  "System Statistics"

class StatusDisplay {
public:
    StatusDisplay(void);
    StatusDisplay(const StatusDisplay& orig);
    virtual ~StatusDisplay();
    void RedrawAll(void);
    void Refresh(void);
    void Refresh(bool);
    void Initialize(rs232*);
    void ShowTime(void);
    int CheckKeyboard(void);
    void HomeScreen(int );
    void StatusScreen(int);
    void NetScreen(int );
    void RadioScreen(int);
    void ShutDownScreen(int);
    void StatsScreen(int);
    // Public variables
    int screen_num;                  // which screen are we showing
    bool disabled;
    int display_line;                // which line of information are we showing. Used to scroll up/down through a list

private:
    rs232* pStatTTY;                  // which serial port we will use
    MatrixDisplay MyLCD;             // Class to talk to an LCD

};

#endif	/* _STATUSDISPLAY_H */

