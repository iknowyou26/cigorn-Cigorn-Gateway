/* 
 * File:   BinaryEntry.cpp
 * Author: john
 * 
 * Created on December 25, 2010, 6:35 AM
 */

#include "BinaryEntry.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include "Cigorn.h"     // Our application-specific constants and headers
#include "TCPsocket.h"
#include "CommThread.h"
#include "dataparser.h"
#include "ascii.h"
#include "WMX.h"
#include "xmlFunctions.h"
#include "xmlDefs.h"

using namespace std;

BinaryEntry::BinaryEntry() {
  data[0] = 0;            // the data in the message
  bcount = 0;             // the number of bytes in this message
  format = -1;            // the format the parser thinks this message is in
  srcID = -1;             // the ID of the source for this message
  dstID = -1;             // the ID where this message is destined to be sent to
  timein  = 0;            // Time when this was created.  */
  SrcDevDesIndex = -1;       // the devicedesignator that originated this entry
  DstDevDesIndex = -1;
  PortIn = -1;            // The port this message came in on
}

// Copy constructor. Must have to use this class in a queue or vector
BinaryEntry::BinaryEntry(const BinaryEntry& orig) {

  int i;
  int c = orig.bcount + 1;
  if (c>=MAX_DATA)
      c = MAX_DATA;

  for (i=0; i < c; i++)
    data[i] = orig.data[i];        // the data in the message

  bcount = orig.bcount;            // the number of bytes in this message
  format = orig.format;            // the format the parser thinks this message is in
  srcID = orig.srcID;              // the ID of the source for this message
  dstID = orig.dstID;              // the ID where this message is destined to be sent to
  timein  = orig.timein;
  PortIn = orig.PortIn;
  SrcDevDesIndex = orig.SrcDevDesIndex;
  DstDevDesIndex = orig.DstDevDesIndex;

}

BinaryEntry::~BinaryEntry() {

}

