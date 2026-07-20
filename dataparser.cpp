#include "platform/PlatformConstants.h"
/* 
 * File:   dataparser.cpp
 * Author: john
 * 
 * Created on August 17, 2010, 10:28 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "Cigorn.h"     // Our application-specific constants and headers
#include "TCPsocket.h"
#include "CommThread.h"
#include "dataparser.h"
#include "ascii.h"
#include "WMX.h"
#include "xmlFunctions.h"
#include "xmlDefs.h"

using namespace std;

// Constructor for the class
dataparser::dataparser() {
    rget = 0;
    rput = 0;  // zero out the buffer
    NMEAparsing = false;
    WMXparsing = false;
    tapState = WAIT_FOR_CONNECT;
    EndOfLineChar = CR;
}

dataparser::dataparser(const dataparser& orig) {


}

dataparser::~dataparser() {
}

void  dataparser::initialize(void){
    rget = 0;
    rput = 0;  // zero out the buffer
    NMEAparsing = false;
    WMXparsing = false;
    XMLparsing = false;
    EndOfLineChar = CR;
    rawdata[0]=NUL;

}

// Called whenever some new data came in via some interface who created an instance of this parser
// This instance of the parser will buffer it, and look for the various protocols
int dataparser::parse(char *buff, int count, int devdes, int devicetype) {
    // Some data came in, so lets parse it into the proper buffers.
    int i;
    int messages= 0;

    if ((devdes < 0) || (devdes > MAXDEVDES))
        return 0; // invalid devices

    this->devindex = devdes;
    this->devicetype = devicetype;
    
    bool parseFlaggedMoreData = false;
    int loops = 0;
    
    switch (devicetype){
        case dNONE:
            // no device configured on this interface. Discard the data.
            rget = 0;
            rput = 0;
            count = 0;
            rawdata[rput] = NULL;
            break;
        case dArcGIS:
            // no device configured on this interface. Discard the data.
            rget = 0;
            rput = 0;
            count = 0;
            rawdata[rput] = NULL;
            break;
        case dDataModem:
            // Loop through all the generic protocols and see if there are messages
            for (i=0; i < protocols.size(); i++){
                protocols[i].AddNewData(buff, count);  // The generic protocol parsers keeps its own buffer
                protocols[i].LookForMessages();
            }
            // Now check the custom protocol parsers
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            cleanraw();                  // get rid of any leading charators that should not be there.
            messages = messages + look4NMEA();
            messages = messages + look4WMX();          // Look to see if there is a WMX message in the data buffer
            if ((NMEAparsing == false) && (WMXparsing == false)){
                // Neither parser sees a message so purge the buffer
                if (rget < rput)
                    rget++;   // skip this byte. It is not part of any protocol we know of.
            }
            break;
        case dWMXmodem:
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            parseFlaggedMoreData = false;
            loops = 0;
            while(rget < rput && !parseFlaggedMoreData){
                if(loops > 20){
                    cleanraw();
                    break;
                }
                
                realignRaw();                  // get rid of any leading charators that should not be there.
                int newMessages = 0;
                
                newMessages = look4NMEA();
                messages += newMessages;
                parseFlaggedMoreData = NMEAparsing;
                if(newMessages > 0 || parseFlaggedMoreData){
                    // Start loop over, we found something
                    continue;
                }
                
                newMessages = look4WMX();
                messages += newMessages;
                parseFlaggedMoreData = WMXparsing;
                if(newMessages > 0 || parseFlaggedMoreData){
                    // Start loop over, we found something
                    continue;
                }
                
                if(rget < rput){
                    // Nothing found if we got to this point. Eat one character
                    // always, then eat any invalid characters
                    rget++;
                    cleanraw();
                }
            }
            break;
        case dAVLPC :
            // Loop through all the generic protocols and see if there are messages
            for (i=0; i < protocols.size(); i++){
                protocols[i].AddNewData(buff, count);  // The generic protocol parsers keeps its own buffer
                protocols[i].LookForMessages();
            }
            // Now check the custom protocol parsers
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            cleanraw();                  // get rid of any leading charators that should not be there.
            messages = messages + look4NMEA();         // See if there is a NMEA message in the buffer
            messages = messages + look4WMX();          // Look to see if there is a WMX message in the data buffer
            if ((NMEAparsing == false) && (WMXparsing == false)){
                if (rget < rput)
                    rget++;   // skip this byte. It is not part of any protocol we know of.
            }
            break;
        case dCigorn :
            // Talking to another gateway. It will be XML encoded.
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            cleanrawXML();               // get rid of any leading charators that should not be there.
            if (XMLparsing == false){
                if (rget < rput)
                    rget++;   // skip this byte. It is not part of any protocol we know of.
            }
            messages = messages + look4XML();          // Look to see if there is a WMX message in the data buffer
            break;
        case dMAILserver:
            // Talking to a mail server. It will be ascii.
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            messages = messages + 1;
            break;
        case dWEBserver:
            // Talking to a web browser
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            messages = messages + 1;
            break;
        case dTerminal:
            // Talking to a terminal
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            look4ASCII();          // copy the ASCII data over to the  qMSGin queue
            messages = messages + 1;
            break;
        case dCLI:                      // Command-line interface to human
            // Talking to a terminal
            addnew(buff, count);         // put the new bytes into the raw buffer. It appends it to the end of the buffer
            messages = messages + 1;
            break;
        case dTAP:
            // Paging commands
            addnew(buff,count);
            messages = messages + look4WMX();          // Look to see if there is a WMX message in the data buffer
            if(!WMXparsing){
                // No WMX. Pass to the TAP parser
                parseTAP();
            }
            realignRaw();
    }
    return messages;
}

int dataparser::parseTAP(){
    while(rget < rput){
    switch(tapState){
        default:
        case WAIT_FOR_CONNECT:
        case CONNECTED:
                if(rput - rget >= 5 && StringCompare(&rawdata[rget], "\x1BPG1\r", 5)){ // <ESC>PG1<CR>
                    // Respond with <CR><ACK><CR><ESC>[p<CR>
                    sendASCII(devindex, devindex, "\r\x06\r\x1B[p\r", 7);
                    
                    // Remove
                    rget += 5;
                    
                    // Next state
                    tapState = CONNECTED;
                    break;
                }else if(rput - rget >= 2 && StringCompare(&rawdata[rget], "\x04\r", 2)){
                    // <EOT><CR>, disconnect. Ignore it
                
                    // Flush buffer
                    rget += 2;
                }else if(rawdata[rget] == '\r'){
                    // "Check for connection"- respond with ID=<CR>
                    sendASCII(devindex, devindex, "ID=\r", 4);
                    
                    // Next char
                    rget++;
                }else if(rawdata[rget] == '\x1B' && rput - rget < 5){
                    // Potentially have a connect string soon. Stop parsing
                    return 0;
                }else if(rawdata[rget] == '\x02'){ // <STX>
                    // Find mesage terminator
                    int crCount = 0;
                    for(int i = rget; i < rput; i++){
                        if(rawdata[i] == '\r'){
                            crCount++;
                        }
                    }
                    
                    if(crCount >= 3){
                        // This looks TAP-ish. We're going to commit to pulling this out of the buffer
                        int cr[3];
                        int currentCr = 0;
                        int bufferIndex = rget;
                        string idString, messageString, checksumString;
                        int checksumSent, checksumCalced;
                        stringstream intParse;
                        
                        while(bufferIndex < rput && currentCr < 3){
                            if(rawdata[bufferIndex] == '\r'){
                                // Change to a null character for c string assignment
                                rawdata[bufferIndex] = '\0';
                                
                                // Record location
                                cr[currentCr] = bufferIndex;
                                currentCr++;
                            }
                            bufferIndex++;
                        }
                        
                        if(rawdata[cr[1] + 1] != '\x03'){ // Second <CR> must be followed by <ETX>
                            // Malformed or bad message
                            // Respond with <CR><NAK><CR>
                            sendASCII(devindex, devindex, "\r\x15\r", 3);
                        }else{
                            idString.assign(&rawdata[rget+1]); // First block
                            messageString.assign(&rawdata[cr[0]+1]); // Second block
                            checksumString.assign(&rawdata[cr[1]+2]); // Third block (skip the ETX)
                            
                            // Checksum - sent as three printable characters with the 4 LSB the actual value
                            checksumSent = 0;
                            checksumSent += ((((int) checksumString[0]) << 8) & 0x0F00);
                            checksumSent += ((((int) checksumString[1]) << 4) & 0x00F0);
                            checksumSent += ((((int) checksumString[2])     ) & 0x000F);
                            
                            checksumCalced = 0;
                            for(int i = rget; i <= cr[1] + 1; i++){ // Up to and including the ETX after the second CR
                                if(rawdata[i] == '\0'){
                                    // We replaced the CRs with null
                                    checksumCalced += (int) ('\r' & 0x07F); // 7 bits only
                                }else{
                                    checksumCalced += (int) (rawdata[i] & 0x07F);
                                }
                            }
                            
                            checksumCalced &= 0x0FFF; // 12 bit limit
                            if(checksumCalced != checksumSent){
                                // Respond with <CR><NAK><CR>
                                sendASCII(devindex, devindex, "\r\x15\r", 3);
                            }else{
                                // Check if pager exists
                                int pagerID = 0, capCode = 0;
                                intParse << idString;
                                intParse >> pagerID;
                                
                                PagerTableEntry pager = Pagers.GetPager(pagerID);
                                capCode = pager.capCode;
                                
                                if(capCode != PagerTable::RESULT_NO_PAGER){
                                    // Respond with <CR><ACK><CR>
                                    sendASCII(devindex, devindex, "\r\x06\r", 3);
                                    
                                    // Pull the single TAP message and push it in to the message queue.
                                    BinaryEntry NewEntry;
                                    NewEntry.srcID = 0;
                                    NewEntry.dstID = pagerID; // We'll continue to use the pager ID
                                    NewEntry.format = fmtPageASCII;
                                    NewEntry.timein = TimeNow();            
                                    NewEntry.SrcDevDesIndex = devindex;
                                    NewEntry.PortIn = ParsingPort;
                                    
                                    // ID is in the destination. Just the ASCII message goes in the data
                                    memcpy(NewEntry.data, messageString.c_str(), messageString.length());
                                    NewEntry.bcount = messageString.length();

                                    qlock.lock();
                                    try{
                                        qMSGin.push(NewEntry);
                                    }catch (exception& e){}
                                    qlock.unlock();
                                }else{
                                    // Pager not in database
                                    // Respond with <CR><NAK><CR>
                                    sendASCII(devindex, devindex, "\r\x15\r", 3);
                                }
                            }
                        }
                        
                        rget = cr[2] + 1; // We processed everything up to the last CR
                    }else if(rput - rget > 50){
                        // Flush, something's wrong
                        rget = rput;
                        return 0; // Bail out of parsing
                    }else{
                        return 0; // Bail out, we'll try again when more data comes in
                    }
                }else{
                    // No useful characters at the head of the queue
                    // Check for potential WMX
                    int wmxMessage = look4WMX();
                    if(WMXparsing){
                        // WMX has laid claim to the characters in the buffer.
                        // We need to stop and let it take over
                        return 0;
                    }
                    
                    if(wmxMessage == 0){
                        // No characters were processed
                        rget++;
                    }
                }
            break;
    }
    }
}

// Look for WMX messages in the buffer, and then it if we find some
// Return the number of NMEA messages we parsed. devindex is the device designator this message came in on.
int dataparser::look4WMX(){
    int count = 0;
    int eot_pos = 0;
    
    // Assume we're not parsing. If there is a WMX message coming, the
    // code below will set this to true
    WMXparsing = false;

    if (rget == rput){
        return 0;  // no data
    }

    while (rawdata[rget] == SOH && rget < rput){
        eot_pos = findchar(EOT);    // find the position of an EOT
        int fs1_pos = findchar(FS1);
        int dle_pos = findchar(DLE);
        int etx_pos = findchar(ETX);
        
        if((eot_pos > rget && eot_pos < rput) &&
           (fs1_pos > rget && fs1_pos < rput) &&
           (dle_pos > rget && dle_pos < rput) &&
           (etx_pos > rget && etx_pos < rput)){
            
            BinaryEntry NewEntry;

            NewEntry.srcID = NULL_ID;
            NewEntry.dstID = NULL_ID;
            NewEntry.timein = TimeNow();
            
            NewEntry.bcount = eot_pos - rget + 1;             // store the byte count
            NewEntry.format = fmtWMX;
            ExtractData(NewEntry.data, NewEntry.bcount );    // take the WMX message out of the rawdata buffer
            if (NewEntry.bcount < WXM_MAX_BYTES){
                // This may be a valid WMX. Lets try to parse it
                WMX wmx(NewEntry.data, NewEntry.bcount);     // create a WMX message structure and parse the data
                if (wmx.valid){
                    NewEntry.srcID = wmx.source;
                    NewEntry.dstID = wmx.destination;
                    //  cout << wmx.Display() << endl;
                }
            }
            NewEntry.SrcDevDesIndex = devindex;
            NewEntry.PortIn = ParsingPort;
            wmxcount_in++;
            //NMEAinput.device = devindex;    // The device index into the devices structure array.
            qlock.lock();
            try
              {
                //cout << "New WMX: " << NewEntry.srcID << endl;
                qMSGin.push(NewEntry);
              }
            catch (exception& e)
              {
                stringstream ss;
                //ss << "Error 846: " << e.what();
                elog.store(ss.str());
              }
            qlock.unlock();
            count++;
            rget = eot_pos + 1;
            realignRaw();
         }else if(rput - rget == 1){
             // There's on an SOH so far. Keep parsing
             WMXparsing = true;
             break;
         }else if(rput - rget >= 2){
             // Start looking for reasons to discard this as potential WMX
             if(!(rawdata[rget + 1] & 0x40)){
                 // Bit 6 of the control field must be 1
                 WMXparsing = false;
             }else{
                 WMXparsing = true;
             }
             
             if(WMXparsing && rput - rget >= 7){
                 for(int i = 2; i <= 6; i++){
                     char inputChar = rawdata[rget+i];
                     if(inputChar == FS1){
                         // Stop if we hit FS1
                         break;
                     }
                     
                     if(!(inputChar >= '0' && inputChar <= '9') && !(inputChar >= 'A' && inputChar <= 'F')){
                         WMXparsing = false;
                         break;
                     }
                 }
             }
             
             // Regardless, we don't have a message, so break out of parsing
             break;
         }else if(rput - rget > WXM_MAX_BYTES){
             WMXparsing = false;
             break;
        }else{
            // Paranoid case that should never happen
            WMXparsing = false;
            break;
        }
    }
    
    return count;  // return how many NMEA messages we found
}


// Look for and XML document in the buffer, and then remove it if we find some
// data in rawdata must be left justified starting at byte 0.
int dataparser::look4XML(){
    int count = 0;
    int eot_pos = 0;
    int i;
    string root;
    string close;
    int nq = rput - rget;   // bytes in the queue


    if (rget == rput){
        XMLparsing = false;                // set true when a parser sees a message that may be for it.
        return 0;  // no data
    }

    if (rput < 10)
        return 0;   // no XML messages in such a small space

    if ((StringCompare(rawdata, "<?xml", 5) == true) && (nq > (MAX_PARSE_BUFF * .9))){
        // the buffer filled up.  OK to throw out this message
        XMLparsing = false;
        return 0;
    }

    while (StringCompare(rawdata, "<?xml", 5) == true){
        // we have the makings of an XML document in our buffer
        XMLparsing = true;
        root = findroot(rawdata, rput, 2);  // find the root element
        if (root.size() > 1){
            BinaryEntry NewEntry;  // This is the message strucure we use to put things into a message queue

            NewEntry.srcID = NULL_ID;
            NewEntry.dstID = NULL_ID;
            NewEntry.timein = TimeNow();

            close = closetag(root);          // make a tag that represents the closing tag for the root element
            eot_pos = FindString(rawdata, close, rput);  // find the location of the closing root tag.
            if (eot_pos > 0)
                eot_pos = eot_pos + close.size();   // the byte number after the last char of the closing tag.
            // cout << root << " " << close << eot_pos << endl;
            // There is a root element
            if (eot_pos > 0){
                // The basics of a XML frame are there so we assume it is one.
                NewEntry.bcount = eot_pos;                       // store the byte count
                NewEntry.format = fmtXML;                        // It is XML formatted.
                xmlcount_in++;
                ExtractData(NewEntry.data, NewEntry.bcount );    // take the WMX message out of the rawdata buffer
                NewEntry.SrcDevDesIndex = devindex;
                NewEntry.srcID = NULL_ID;
                NewEntry.dstID = NULL_ID;
                NewEntry.PortIn = ParsingPort;
                // find out what type of XML message this one is
                // if (root == maketag(xmlCIGORN)){
                // All Cigorn inter-site messages must have a root tag with the word Cigorn in it
                if (root.find("Cigorn") > 0){
                    // This XML message is from another CIGORN gateway
                    NewEntry.format = fmtCigorn;
                }
                qlock.lock();
                try
                  {
                    qMSGin.push(NewEntry);
                  }
                catch (exception& e)
                  {
                    stringstream ss;
                    ss << "Error 846: " << e.what();
                    elog.store(ss.str());
                  }
                qlock.unlock();
                count++;
            }
           // Now remove reminents of the old message that we took the data from
           cleanrawXML();
         }
        else{
            // No CR yet. Is there an errror or missing CR??
            break;
        }
    }
    return count;  // return how many NMEA messages we found

}

// return how many bytes are inthe parser
int dataparser::RawRXcount(){

    int nq = rput - rget;   // bytes in the queue
    if (nq>=0){
        return nq;
    }else
        return 0;

}

// clear out the parsing buffer
void dataparser::ClearRXbuffer(void){

    rput = 0;
    rget = 0;   // bytes in the queue

}

// Look for NMEA messages in the buffer, and then it if we find some
// Return the number of NMEA messages we parsed.
// Return 0 if the next message in the buffer looks like a NMEA, but it not yet complete
int dataparser::look4NMEA(){
    int count = 0;
    int cr_pos = 0;
    int msg_size;
    int nq = rput - rget;   // bytes in the queue
    bool parsing = false;

    if (rget == rput){
        NMEAparsing = false;
        return 0;  // no data
    }

    if(rawdata[0] != NMEA_START){
        NMEAparsing = false;
        return 0;  // this is not a NMEA type message start char
    }

    while (rawdata[0] == NMEA_START){
        cr_pos = findchar(CR);  // find the position of a cr
        msg_size = cr_pos + 1;
        NMEAparsing = true;    // flag we are parsing now
        if (cr_pos > 0){
            // There is a CR, so lets extract this NMEA messasge
            BinaryEntry NMEAinput;
            BinaryEntry debug;

            NMEAinput.srcID = NULL_ID;
            NMEAinput.dstID = NULL_ID;
            NMEAinput.timein = TimeNow();
            NMEAinput.bcount = ExtractData(NMEAinput.data, msg_size);    // take the NMEA message out of the rawdata buffer
            NMEAinput.SrcDevDesIndex = devindex;                               // The device index into the devices structure array.
            NMEAinput.PortIn = ParsingPort;
            if (NMEAinput.bcount > 0){
                //  cout << "NMEA:" << NMEAinput.data << endl;
                SetFormat(NMEAinput);           // determine the message format and IDs for this input
                // cout << "NMEA F:" << NMEAinput.srcID << " D" << NMEAinput.dstID << " @:" << NMEAinput.timein << " " << NMEAinput.data << endl;
                qlock.lock();
                try
                  {
                    qMSGin.push(NMEAinput);       // thows exception sometimes
                  }
                    catch (exception& e)
                  {
                    stringstream ss;
                    ss << "Error 844: " << e.what();
                    elog.store(ss.str());
                  }

                debug = qMSGin.front();
                // cout << qMSGin.size() << "Debug F:" << debug.srcID << " D" << debug.dstID << " @:" << debug.timein << endl;
                qlock.unlock();
                count++;
            }
           // Now remove reminents of the old message that we took the data from
           cleanraw();
        }
        else{
            // No CR yet. Is there an errror or missing CR??
            if (nq > NMEA_MAXLEN){
                return 0;              // protocol violation. too many bytes for this type of message
                NMEAparsing = false;    // flag we are not parsing now
                }
            break;
        }
    }

    return count;  // return how many NMEA messages we found

}

// Look for ASCII messages in the buffer, and then it if we find some
// Return 1.  Pass all ascii or binary data through.
int dataparser::look4ASCII(){
    int count = 0;
 
    if (rget == rput){
        return 0;  // no data
    }
    count = rput - rget;  // number of bytes we have

    BinaryEntry ASCIIinput;

    ASCIIinput.format = fmtASCII;
    ASCIIinput.srcID = DEFAULT_ID;
    ASCIIinput.dstID = NULL_ID;
    ASCIIinput.timein = TimeNow();
    ASCIIinput.bcount = ExtractData(ASCIIinput.data, count);    // take the ASCII data out of the rawdata buffer
    ASCIIinput.SrcDevDesIndex = devindex;                          // The device index into the devices structure array.
    ASCIIinput.PortIn = ParsingPort;
    qlock.lock();
    if (ASCIIinput.bcount > 0){
        //   cout << "ASCII #" << ASCIIinput.bcount << "  " << ASCIIinput.data << endl;
        count++;
        try
          {
            qMSGin.push(ASCIIinput);       // thows exception sometimes
          }
        catch (exception& e)
          {
            stringstream ss;
            ss << "Error 845: " << e.what();
            elog.store(ss.str());
          }

    }
   qlock.unlock();
   // Now remove reminents of the old message that we took the data from
   rget = 0;  // flush the buffer
   rput = 0;
   rawdata[rput] = NUL;

   return 1;  // return how many messages we found

}

// Look for a CR terminated string. return it if we find one
bool dataparser::look4command(string *s){
    int count = 0;
    int NLposition = -1;
    int i;
    *s = "";

    if (rget == rput){
        return false;  // no data
    }
    count = rput - rget;  // number of bytes we have in the input buffer

    if ((count > 0) && (count < MAX_PARSE_BUFF)){
        // See if there is a newline in the buffer
        NLposition = findchar(NL);
        // If it is a char or UNIX terminal, then CR comes in.  If Linux, then it is NL.
        if (NLposition >= 0){
            EndOfLineChar = NL;
        }else{
            NLposition = findchar(CR);
            if (NLposition >= 0){
                EndOfLineChar = CR;
           }
        }
        if ((NLposition >= 0) && (NLposition < MAX_PARSE_BUFF)){
            // there is a CR
            for (i = 0; i < NLposition; i++){
             //if (rawdata[i] > CR)
              *s = *s + rawdata[i];  // done copy over NL or CR charactors
            }
            //*s = trim(*s);
            rget = 0;                // flush the buffer
            rput = 0;
            rawdata[rput] = NUL;
            return true;
        }
    }else{
        // bad error. Buffer overflow.


    }

   return false;  //

}


// look at the data and figure out the type of message format
// Set the value in the format part of the structure.
int dataparser::SetFormat(BinaryEntry& be){
    int p;
    
    be.format = fmtINVALID;  // default vale
    
    // See if this looks like a NMEA formatted message
    p = CharPos(be.data, '*', be.bcount);  // the checksum delimiter position in the buffer.
    if ((be.bcount > 6) && (be.data[0] == '$') && (p > 7)){
        // This is some type of NMEAish format
        be.format = fmtNMEA;            // default to this if no other format is found
        be.dstID = NULL_ID;
        be.srcID = NULL_ID;
        if (StringCompare("$PRAVE",be.data, be.bcount)){
            // This is a PRAVE message.  Get the IDs.
            be.format = fmtPRAVE;
            // cout << "PRAVE In" << endl;
            // Get the destination ID from the message
            if (GetSubInt(be.data, 3, be.dstID) == false)
              be.dstID = NULL_ID;       // no ID so can't route it.
            // Get the source ID
            if (GetSubInt(be.data, 2, be.srcID) == false)
              be.srcID = NULL_ID;       // no ID so can't route it.
        }
        return be.format;
    }

   // look for WMX messages
   p = CharPos(be.data, WMX_EOT, be.bcount);  // the checksum delimiter position in the buffer.
   if ((be.bcount > 6) && (be.data[0] == WMX_SOH) && (CharPos(be.data, WMX_EOT, be.bcount) > 20)
           && (CharPos(be.data, WMX_EOT, be.bcount) > 20) && (CharPos(be.data, FS1, be.bcount) > 1) ){
        // This is WMX formatted
        be.format = fmtWMX;               // default to this if no other format is found
        return be.format;
    }

   return be.format;
    
    
}

// Remove n data bytes out of the rawdata and put them in the buff
int dataparser::ExtractData(char *buff, int n){
    int i = 0;
    
    while ((i < MAX_DATA) && (rget < rput) && (n > 0) && (rget < MAX_PARSE_BUFF)){
          *buff = rawdata[rget];
          buff++;
          rget++;
          i++;
          n--;
    }
    
    return i;     // actual number of bytes we removed.
};

// Find the position of a char in the raw buffer
// Return -1 if not found.
// 0-based!
int dataparser::findchar(char c){
    int i = rget;
    
    while (i < rput){
        if (rawdata[i] == c){
          // Found it.  
          return i;  
        }
        i++;
    }
    return -1;   // not found. 
   
}


// Add some new data to our raw buffer.
bool dataparser::addnew(char *buff,int count){
 
    while (count > 0){
       if ((rput < 0)  || (rput >= (MAX_PARSE_BUFF-1))){
           // bad error
           rput = 0;
           rget = 0;
           return false;
       }
       rawdata[rput] = *buff;
       rput++;
       buff++;
       count--;
    }
    rawdata[rput] = NUL;
 
    return true;

}

void dataparser::cleanraw(void){
    int i;
    int count;
    // Check for pointer errors.
    if (rget >= rput){
        // Error with pointers. Something is messed up, so restart the buffer.
        rget = 0;
        rput = 0;
        count = 0;
        rawdata[rput] = NUL;
        return;
    }

    // Take off the any leading non-start bytes.
    // Every protocol we support must have its header char in this list.
    switch (devicetype){
    default:
        cout << "Serious error! Undefined device is calling dataparser::cleanraw()! Data will be lost!";
    case dNONE:
    case dArcGIS:
        // Discard the data
        rget = 0;
        rput = 0;
        count = 0;
        rawdata[rput] = NULL;
        break;
    case dDataModem:
    case dWMXmodem:
    case dAVLPC :
        // NMEA or WMX
        while ((rget < rput) && (rget< MAX_PARSE_BUFF) && (rawdata[rget] != NMEA_START) && (rawdata[rget] != WMX_SOH) ){
                rget++;
        }
        break;
    case dCigorn :
        // Uses XML, and should never call this
        cout << "Error: Cigorn device is calling dataparser::cleanraw()";
        break;
    case dMAILserver:
        // Text protocol that should never call this
        cout << "Error: Mailserver device is calling dataparser::cleanraw()";
        break;
    case dWEBserver:
        // Text protocol that should never call this
        cout << "Error: Web server device is calling dataparser::cleanraw()";
        break;
    case dTerminal:
        // Terminal. Anything is valid
        cout << "Error: Terminal device is calling dataparser::cleanraw()";
        break;
    case dCLI:                      // Command-line interface to human
        // Terminal. Anything is valid
        cout << "Error: Terminal device is calling dataparser::cleanraw()";
        break;
    case dTAP:
        // WMX parser will call this, but the TAP parser takes care of
        // the raw buffer. So just don't do anything
        break;
    }
    
    // Check for an empty buffer
    count = rput - rget;
    if ((count<= 0) || (rget == rput))
    {  // No valid chars in the buffer, so reset it back to point to the beginning of the buffer.
        rget = 0;
        rput = 0;
        count = 0;
        rawdata[rput]=NUL;
    }

    // Left justify the data back to the beginning of the buffer at byte zero
    realignRaw();
}

/**
 * Realigns the raw buffer so that it starts at 0. This should be called
 * after all parsing is done to avoid overruns
 */
void dataparser::realignRaw(void){
    if(rget == 0){
        // Already aligned
        return;
    }
    
    int count = rput - rget;
    if(count > MAX_PARSE_BUFF){
        count = MAX_PARSE_BUFF;
    }
    if(count >= 0){
        memmove(rawdata, &(rawdata[rget]), count);
    }else{
        cout << "Negative count in data parser. Flushing buffer.";
        rget = 0;
        rput = 0;
        return;
    }
    rget = 0;
    rput = count;
}

// Make sure the front of the buffer has a valid XML document in it. Remove
// non-XML data
void dataparser::cleanrawXML(void){
    int i;
    int count;
    // Check for pointer errors.
    if (rget >= rput){
        // Error with pointers. Something is messed up, so restart the buffer.
        rget = 0;
        rput = 0;
        count = 0;
        rawdata[rput] = NUL;
        return;
    }

    // Take off the any leading non- '<' bytes
    while ((rget < rput) && (rget< MAX_PARSE_BUFF) && (rawdata[rget] != '<')){
       // The first charactor in the buffer should be a <
       rget++;  // go past this char. It should not be there there.
    }

    // Check for an empty buffer
    count = rput - rget;
    if ((count<= 0) || (rget == rput))
    {  // No valid chars in the buffer, so reset it back to point to the beginning of the buffer.
        rget = 0;
        rput = 0;
        count = 0;
        rawdata[rput] = NUL;
        return;
    }

    // Left justify the data back to the beginning of the buffer at byte zero
    realignRaw();

    // Now see if a valid XML beginning sequence is in the buffer.
    if (count > 5){
        // First 5 charactors are always   <?xml
        if (StringCompare(rawdata, "<?xml", 5) == false){
            // error in the data buffer.  move the get past the first <
            rget++;
            // Take off the any leading non- '<' bytes
            while ((rget < rput) && (rget< MAX_PARSE_BUFF) && (rawdata[rget] != '<')){
               // The first charactor in the buffer should be a <
               rget++;  // go past this first < char. It should not be there.
            }
        }
    }
}

/**
 * Sends an ASCII message, usually into a static route or back to the source
 * @param srcDev Device designator the parser is attached to
 * @param destDev Destination device designator, if known. Set to -1 if unknown
 * @param message ASCII message to be sent
 * @param length Length in bytes of the message
 * @return number of messages sent (either 0 or 1)
 */
int dataparser::sendASCII(int srcDev, int destDev, char* message, int length){
    BinaryEntry messageOut;

    messageOut.format = fmtASCII;
    messageOut.srcID = NULL_ID;
    messageOut.dstID = NULL_ID;
    messageOut.timein = TimeNow();
    messageOut.bcount = length;  
    CopyCstr(messageOut.data, message, length);
    messageOut.SrcDevDesIndex = srcDev;                          // The device index into the devices structure array.
    messageOut.DstDevDesIndex = destDev;                          // The device index into the devices structure array.
    messageOut.PortIn = ParsingPort;
    if (messageOut.bcount > 0){
        qlock.lock();
        try
          {
            qMSGin.push(messageOut);       // thows exception sometimes
          }
        catch (exception& e)
          {
            stringstream ss;
            ss << "Error 845: " << e.what();
            elog.store(ss.str());
          }
        qlock.unlock();
        return 1; // 1 message sent
    }else{
        return 0; // 0 messages sent
    }
}

