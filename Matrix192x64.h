/* 
 * File:   MatrixDisplay.h
 * Author: john
 *
 * Created on August 14, 2010, 12:03 AM
 */

#ifndef _MATRIXDISPLAY_H
#define	_MATRIXDISPLAY_H

#include "serialhandler.h"

#define DPL_HEIGHT      8       // 8 lines
#define DPL_WIDTH       32      // 32 per line
#define PDL_PIXEL_W     192     // 196 pixels wide
#define DPL_PIXEL_H     64      // 65 pixels wide

// Function codes for the display
#define MD_FUNCTION  0xFE           // Matrix display function code
#define MD_HOME      0x48
#define MD_XY        0x47
#define MD_CLEAR_DSP 0x58
#define MD_GPIO_OFF  0x56
#define MD_GPIO_ON   0x57
#define MD_FLOW_OFF  0x3B
#define MD_SETBAUD   0x39
#define MD_SETBRITE  0x99
#define MD_B19200    0x67     // setting for 19200 in the display

#define LEDYELLOW       0x00
#define LEDGREEN        0x01
#define LEDRED          0x02
#define LEDOFF          0x03

#define DISPLAYBAUD     19200
#define DISPLAYSETTING  "N81"

using namespace Communications;

class MatrixDisplay {
public:

    MatrixDisplay(void);
    MatrixDisplay(rs232*);
    MatrixDisplay(const MatrixDisplay& orig);
    virtual ~MatrixDisplay();
    void CursorHome();
    void ClearDisplay();
    void CursorXY(char, char);
    void setPort(rs232*);
    void Initialize(void);
    void setLED(int, int);
    char MapButton(char);
    void PlaceLong(long,int,int,int);
    void PlaceString(std::string,int,int,int);
    void PlaceString(std::string);
    void PlaceLongLJ(long, int , int , int);
    void PlaceDouble(double , int , int , int );

private:
    rs232* serial_port;
    char   gpiomem;
};

#endif	/* _MATRIXDISPLAY_H */

