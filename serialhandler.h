/* 
 * File:   serialport.h
 * Author: john
 *
 * Created on July 29, 2010, 7:59 PM
 */

#ifndef _SERIALPORT_H
#define	_SERIALPORT_H

#include <string>
#include <queue>

#include "dataparser.h"
#include "BinaryEntry.h"


#define  MAXSERIALBUFFSZ   5000

namespace Communications {
    enum BCLState {
        WaitForCD,
        CDHigh
    };
    
    class rs232
    {
        public:
            rs232();
            rs232(int, int, int, char);
            int index;              // The 0-based index of this object in the RS232[] array of objects
            int myDevIndex;         // The devicedesignator index of the device[] this rs232 object will is bound to and servicing
            int myDevType;          // The type of device that we talk to
            int baudrate;
            int databits;
            int stopbits;
            char parity;
            char flowcontrol;
            bool bcl;
            bool invertData;
            bool echo;              // true if we want the serial port echo feature on.
            long bytes_in;
            long bytes_out;
            long msg_in;               // count the messages in/out
            long msg_out;
            time_t time_last_msg;      // The time we got the last message in from this port
            std::string devicename;
            std::string fullname;
            int handle;
            bool localecho;              // True to echo chars to the local console display
            bool cts_in;
            bool dsr_in;
            pthread_mutex_t qlock;       // Lock for the TTY message queue
            deque<BinaryEntry> MsgQout;  // Messages that are queued up to be sent out this TTY port
            double timeNextPageAllowed;
            double busyChannelStartTime;
            BCLState bclState;
            
            bool ForceReset;
            bool ShouldConnect;
            
            bool OpenComPort();
            bool ReOpen();
            bool Configure(int,string);
            int SendString(std::string);
            int SendBytes(char*, int);
            bool GetChar(char* );
            int GetChars(void);               // read the data, and put it in the buffer.  return the size of the buffer. 
            bool CTSin(void);                 // the value of the CTS input pin. TRUE is OK to send.
            bool DSRin(void);                 // the value of the CTS input pin. TRUE is OK to send.
            bool CDin(void);                  // CD pin status
            void setIncomingFlowStatus(bool allowInboundFlow);
            int queuedBytes(void);
            double ActivityTime(void);

            bool isInputPaused();
            bool isOutputPaused();
            
            void pauseInputUntil(double unpauseTime);
            void pauseOutputUntil(double unpauseTime);
            
            void unpauseInput();
            void unpauseOutput();
            
            dataparser MyParser;              // a data parser for the raw data that comes in
            
        private:
            double timeOutputUnpaused;
            double timeInputUnpaused;
    };
}

#endif	/* _SERIALPORT_H */

