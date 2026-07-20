/* 
 * File:   Translator.cpp
 * Author: john
 * 
 * Created on September 30, 2010, 10:57 PM
 */

#include "Translator.h"

#include "Router.h"
#include <iostream>
#include <string>
#include <string.h>   // Required by strcpy()
#include <stdlib.h>   // Required by malloc()
#include <ios>
#include <limits>
#include <vector>
#include <locale>

#include "Cigorn.h"     // Our application-specific constants
#include "CommThread.h"
#include "Translator.h"

using namespace std;

int translatorSequence = 0;

// Translate the protocol if needed.
// The DstDevDesIndex of the destination interface must be set before calling this.
int ProtocolTranslate(BinaryEntry& be, BinaryEntry& beOut){

    int devicetype;
    WMX MyWMX;

    // The device type this message is destined for
    devicetype = OurDevices.devicetypes[be.DstDevDesIndex];
    switch (devicetype){
        case dNONE:
            beOut = be;
            break;
        case dDataModem:
            // Loop through all the generic protocols and see if there are messages
            beOut = be;
            break;
        case dAVLPC :
            // Loop through all the generic protocols and see if there are messages
            beOut = be;
            break;
        case dCigorn :
            // Talking to another gateway. It will be XML encoded.
            beOut = be;
            break;
        case dMAILserver:
            // Talking to a mail server. It will be ascii.
            beOut = be;
            break;
        case dWEBserver:
            // Talking to a web browser
            beOut = be;
            break;
        case dTerminal:
            // Talking to an ASCII terminal or some other ASCII device
            //                    WMX wmx(NewEntry.data, NewEntry.bcount);     // create a WMX message structure and parse the data
            if (be.format ==  fmtWMX ){
                // It is WMX wrapped. Unwrapp it
                WMX wmx(be.data, be.bcount);     // create a WMX message structure and parse the data
                if (wmx.size < be.MAXDATA){
                    // OK to change the format to ASCII.
                    memcpy(be.data, wmx.data, wmx.dsize);      // store the data bytes int the binary entry
                    be.bcount = wmx.dsize;
                    be.format = fmtASCII;
                }
             }
            beOut = be;
            break;
        case dWMXmodem:
        case dTAP:
            // WMX-enabled connections. ASCII can be wrapped and sent
            beOut = be;
            if (be.format ==  fmtASCII ){
                // Build up a WMX frame in the output message
                // beOut.bcount = MyWMX.BuildWMX(be.data ,be.bcount, be.srcID, be.dstID , WMX_FR_TXD, beOut.data ,beOut.MAXDATA );
                // create a WMX frame for this message
                // Source ID defaults to the base ID that will send this
                MyWMX.seqnum = translatorSequence; // Ensure the message has a unique sequence
                translatorSequence++;
                if(translatorSequence > WMX_MAX_SEQUENCE){
                    translatorSequence = 0;
                }
                if (MyWMX.BuildWMX(be.data, be.bcount, 0, be.dstID , WMX_FR_TXD)){
                    // WMX constructed OK
                    if ((MyWMX.size < beOut.MAXDATA) && (MyWMX.size > 0) ) {
                        memcpy ( beOut.data, MyWMX.frame, MyWMX.size );
                        beOut.format = fmtWMX;  // set the format flag
                        beOut.bcount = MyWMX.size;
                    }
                }
            }
            break;
        case dArcGIS:
            beOut = be;
            break;
        default:
            beOut = be;
            break;
    }


    return 0;

}

