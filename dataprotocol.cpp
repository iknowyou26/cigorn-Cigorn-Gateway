/* 
 * File:   dataprotocol.cpp
 * Author: john
 * 
 * Created on September 21, 2010, 10:47 PM
 */

#include "dataprotocol.h"
#include "ascii.h"
#include <stdio.h>
#include <iostream>
#include <map>
#include "ourstructures.h"
#include "functions.h"
#include "BinaryEntry.h"

using namespace std;

dataprotocol::dataprotocol() {
    count = 0;
    rawdata[0] = NUL;
    header = "";
    name = "";
}

dataprotocol::dataprotocol(const dataprotocol& orig) {
}

dataprotocol::~dataprotocol() {
}

int dataprotocol::LookForMessages(){
    int cr_pos = 0;

    if (count == 0)
        return 0;  // no data

    // Loop here while there is a message in the buffer
    while (rawdata[0] == SOH_ch){
        cr_pos = FindChar(EOT_ch);  // find the position of the end of the message
        // cout << "C>>" << cr_pos << "," << count << " " << rawdata <<  endl;

        if (cr_pos > 0){
            // There is a CR, so lets extract this NMEA messasge
            ExtractMessage();    // take the message out of the rawdata buffer
            
         }
        else{
            // No CR yet. Is there an errror or missing CR??
            break;
        }
    }
    return count;  // return how many NMEA messages we found

}
// Remove one NMEA message out of the buffer
bool dataprotocol::ExtractMessage(void){
    int i = 0;
    int j;
    bool done = false;
    BinaryEntry NewMessage;
    NewMessage.bcount = 0;
    NewMessage.data[0] = NUL;
    NewMessage.timein = TimeNow();  // remember when we created this entry

    // Find the end of the message
    while ((i < MAX_PROTO_BUFF) && (i < count) && (i < MaxLen) && (!done) ){
        if (rawdata[i] == EOT_ch)
            done = true;
        i++;
    }

    // Copy the data to a new message buffer
    for (j = 0; j < i; j++) {
            NewMessage.data[j] = rawdata[j];
            if (j >= MAX_PROTO_BUFF )
                break;  // safety
        }
    NewMessage.data[j] = NUL;
    // cout << NewMessage.data << endl;

    // Now remove the old message that we took the first byte out from
    // i should now point to the char after the EOT char.
    // Left-justify the buffer so SOH is in rawdata[0]
    j = 0;
    if (i > 0){
        // We need to shift the buffer down to put first byte in 0 location
        while ((i < count) && ( j < MAX_PROTO_BUFF)){
            rawdata[j++] = rawdata[i++];
        }
    }

    // clean up the beginning of the buffer
    CleanRaw();

    count = j;              // the new number of bytes in the buffer
    rawdata[count] = NUL;   // why not
   // cout << "Cnt= " << count << " bytes. Buff=" << rawdata << endl;
    return true;
};



// Find the position of a char in a NUL terminated string.
// Return -1 if not found.
// 0-based!
int dataprotocol::FindChar(char c){
    int i = 0;

    while ((rawdata[i] != NUL) && (i < count)){
        if (rawdata[i] == c){
          // Found it.
          return i;
        }
        i++;
    }
    return -1;   // not found.

}

int dataprotocol::AddNewData(char *buff, int c){

    //cout << "Adding " << buff << endl;

    // clean up the beginning of the buffer
    CleanRaw();

    // now put the new data into the buffer
   // cout << "Insert " << c <<  " at " << count << endl;
    while ((count < MAX_PROTO_BUFF) && (c > 0)){
        rawdata[count] = *buff;   // store a new bytes
        buff++;                   // go to the next byte
        count++;
        c--;
    }

    rawdata[count] = NUL;  // always Nul terminate in case someone prints the buffer
    return count;
}

void dataprotocol::CleanRaw(void){
    int i;
    int j;

    // Check for pointer errors.
    if (count >= MAX_PROTO_BUFF){
        // Error with pointers. Something is messed up, so restart the buffer.
        count = 0;
        rawdata[0] = NUL;
        return;
    }

    // take off the any leading non-start bytes
    i = 0;
    while ((i< count) && (i< MAX_PROTO_BUFF) && (rawdata[i] != SOH_ch)){
            // The first charactor in the buffer should be a start byte for some type of message
       i++; // go past this char. It should not be there there.
    }


    // Check for an empty buffer
    if ((count <= 0) || (i >= count) || (i>= MAX_PROTO_BUFF) )
    {  // No valid chars in the buffer, so reset it back to point to the beginning of the buffer.
       count = 0;
       rawdata[0] = NUL;
       return;
    }

    // Left justify the data back to the beginning of the buffer at byte zero
    j = 0;
    if (i > 0){
        // i should be pointing to the SOH byte.
        // copy the raw data down to byte zero
        while ((i < count) && ( i < MAX_PROTO_BUFF)){
            rawdata[j++] = rawdata[i++];  // move a byte
        }
        count = j;
        rawdata[j] = NUL;
    }


}