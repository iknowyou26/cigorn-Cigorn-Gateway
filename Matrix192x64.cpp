/* 
 * File:   MatrixDisplay.cpp
 * Author: john
 * 
 * Created on August 14, 2010, 12:03 AM
 */

#include "serialhandler.h"
#include "Cigorn.h"     // Our application-specific constants
#include "Matrix192x64.h"
#include "StatusDisplay.h"
#include <sstream>


MatrixDisplay::MatrixDisplay(void) {
    
}

MatrixDisplay::MatrixDisplay(rs232* r) {
    serial_port = r;
    gpiomem = 0xFF;    // all LEDs off

    // Turn Flow control off
    char s[10];
    s[0] = MD_FUNCTION;
    s[1] = MD_FLOW_OFF;
    serial_port->SendBytes(s,2);

}

MatrixDisplay::MatrixDisplay(const MatrixDisplay& orig) {
}

MatrixDisplay::~MatrixDisplay() {
}

void MatrixDisplay::setPort(rs232 * p){
     serial_port = p;  // remember the port we should be using
     serial_port->Configure(DISPLAYBAUD, DISPLAYSETTING);
     serial_port->OpenComPort();
};

// Initialize the display
void MatrixDisplay::Initialize() {
    char s[10];
    // Set the baud rate 19200.  This is doen to make sure it is saved in flash
    s[0] = MD_FUNCTION;
    s[1] = MD_SETBAUD;
    s[2] = MD_B19200;
    serial_port->SendBytes(s,3);

    // Turn Flow control off
    s[0] = MD_FUNCTION;
    s[1] = MD_FLOW_OFF;
    serial_port->SendBytes(s,2);
}

// Output a string to the RS232 serial port.
// Put it at location X,Y on the display, max length l.
void  MatrixDisplay::PlaceString(std::string S, int x, int y, int l)
 {

     if (l > 0){
        // User wants this to be a fixed field width.
         if (S.length()> l)
             S = S.substr(S.length()-l, l);         // string version is too long, trim leading chars off
         else
         { // Left pad with spaces
             while (S.length() < l){
                 S = " " + S;
             }
         }
         x = x + l - S.size();          // right justify
    }
    
    CursorXY(x,y);                 // put the cursor at x,y
    serial_port->SendString(S);

 }



// Output a long integer to the RS232 serial port.
// Put it at current cursor location, Left justified,
void  MatrixDisplay::PlaceLongLJ(long num, int x, int y, int l)
 {
    std::string S;
    std::stringstream ss;
    ss << num;
    ss >> S;

    if (l > 0){
        // User wants this to be a fixed field width.
         if (S.length()> l)
             S = S.substr(S.length() - l, l);         // string version is too long, trim it
         else
         { // right pad with spaces
             while (S.length() < l){
                 S = S + " ";
             }
         }
    }

    CursorXY(x,y);                 // put the cursor at x,y
    serial_port->SendString(S);

}

// Output a long integer to the RS232 serial port.
// Put it at current cursor location, RIGHT justified,
void  MatrixDisplay::PlaceLong(long num, int x, int y, int l)
 {
    std::string S;
    std::stringstream ss;
    ss << num;
    ss >> S;

    if (l > 0){
        // User wants this to be a fixed field width.
         if (S.length()> l)
             S = S.substr(S.length()-l, l);         // string version is too long, trim it
         else
         { // Left pad with spaces
             while (S.length() < l){
                 S = " " + S;
             }
         }
    }

     x = x + l - S.size();          // right justify
     CursorXY(x,y);                 // put the cursor at x,y

     serial_port->SendString(S);
 }

// Output a long integer to the RS232 serial port.
// Put it at current cursor location, RIGHT justified,
void  MatrixDisplay::PlaceDouble(double num, int x, int y, int l)
 {
    std::string S;
    std::stringstream ss;
    ss << num;
    ss >> S;

    if (l > 0){
        // User wants this to be a fixed field width.
         if (S.length()> l)
             S = S.substr(S.length()-l, l);         // string version is too long, trim it
         else
         { // Left pad with spaces
             while (S.length() < l){
                 S = " " + S;
             }
         }
    }

     x = x + l - S.size();          // right justify
     CursorXY(x,y);                 // put the cursor at x,y

     serial_port->SendString(S);
 }

void  MatrixDisplay::PlaceString(std::string S)
 {
     serial_port->SendString(S);

 }

// Put the cursor at the HOME location (upper left)
void MatrixDisplay::CursorHome(void) {

    char s[10];
    s[0] = MD_FUNCTION;
    s[1] = MD_HOME;
    serial_port->SendBytes(s,2);
}

// Put the cursor at the HOME location (upper left)
void MatrixDisplay::CursorXY(char x, char y) {

    if ((x> DPL_WIDTH) || (y > DPL_HEIGHT) || ( x<0 ) || ( y < 0 ))
        return;// can't do this. 

    char s[10];
    s[0] = MD_FUNCTION;
    s[1] = MD_XY;
    s[2] = x;
    s[3] = y;
    s[4] = NUL;
    serial_port->SendBytes(s, 4);
}


// Put the cursor at the HOME location (upper left)
void MatrixDisplay::ClearDisplay(void) {

    char s[10];
    s[0] = MD_FUNCTION;
    s[1] = MD_CLEAR_DSP;
    serial_port->SendBytes(s,2);
    //s[1] = MD_SETBRITE;
    //s[2] = 0xff;  // brightness level, 0-ff
}

// Set the color of LEN n to color
void MatrixDisplay::setLED(int n, int Color){

    char s[10];
    char p1, p2;
    char func;

    p1=0; p2=0;

    switch (Color)
    {
      case LEDYELLOW:
          p1=0;
          p2=0;
          break;
      case LEDGREEN:
          p1=1;
          p2=0;
          break;
      case LEDRED:
          p1=0;
          p2=1;
          break;
      case LEDOFF:
          p1=1;
          p2=1;
          break;
    }


    if (p1 == 0){
        func = MD_GPIO_OFF;  // the bit needs to be on
    }
    else{
        func = MD_GPIO_ON; // the bit needs to be off
    }
    s[0] = MD_FUNCTION;
    s[1] = func;  // set or clear teh GPIO function
    s[2] = n*2+1;   // the GPIO bit we are manipulating
    serial_port->SendBytes(s, 3);

    if (p2 == 0){
        func = MD_GPIO_OFF;  // the bit needs to be on
    }
    else{
        func = MD_GPIO_ON; // the bit needs to be off
    }
    s[0] = MD_FUNCTION;
    s[1] = func;    // set or clear teh GPIO function
    s[2] = n*2+2;   // the GPIO bit we are manipulating
    serial_port->SendBytes(s, 3);

  }

// Map the ASCII char for a button to the function of the button
// Return 0 if we cannot map it.
char MatrixDisplay::MapButton(char c){

    // The generic button definitions
   switch (c)
        {
          case 0x42:
              return BUT_UP;
          case 0x43:
              return BUT_RIGHT;
          case 0x44:
              return BUT_LEFT;
          case 0x45:
              return BUT_ENTER ;
          case 0x41:
              return BUT_TOP_LEFT;
          case 0x47:
              return BUT_BOTTOM_LEFT;
          case 0x48:
              return BUT_DOWN;

        }
   return 0;

}
