
#include <string.h>
#include <string>
#include "serialhandler.h"
#include <fcntl.h>
#include "Cigorn.h"     // Our application-specific constants and headers
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <termio.h>
#include <sys/fcntl.h>
#include "DeviceList.h"
#include <linux/serial.h>

using namespace std;

// rs232 Class routines. Use to send data to and get data from the RS232 ports.

namespace Communications {
    
    // Initialize the class, default parameters. 38400, 8 data, 1, stop, No parity
    rs232::rs232()
        {
        int i;
            index = 0;              // unknown for now
            myDevIndex = -1;
            myDevType = dNONE;
            baudrate = 38400;
            bytes_in = 0;
            bytes_out = 0;
            stopbits = 1;
            databits = 8;
            parity = 'n';
            flowcontrol = 'N';
            bcl = false;
            invertData = false;
            handle = -1;         // The Linux file handle for this port
            MyParser.rget = 0;
            MyParser.rput = 0;
            MyParser.rawdata[0]=NUL;
            MyParser.ParsingPort = -1;  // unknown right now
            localecho = false;
            cts_in = false;
            echo = true;         // the serial port char echo (loopback) setting.
            devicename = "";
            fullname="";
            msg_in = 0;
            msg_out = 0;
            time_last_msg = time(NULL);
            timeNextPageAllowed = 0;
            busyChannelStartTime = 0;
            bclState = WaitForCD;
            timeOutputUnpaused = 0;
            timeInputUnpaused = 0;
            ShouldConnect = false;
            ForceReset = false;
        };
    
    // Initialize the class, given baud rate, paritys, databits, stops
    rs232::rs232(int br, int d, int ns, char p)
        {
            baudrate = br;
            bytes_in = 0;
            bytes_out = 0;
            stopbits = ns;
            databits = d;
            parity = p;
            flowcontrol = 'N';
            bcl = false;
            invertData = false;
            handle = -1;
            MyParser.rget = 0;
            MyParser.rput = 0;
            MyParser.rawdata[0]=NUL;
            MyParser.DefaultDstID = DEFAULT_ID;
            MyParser.DefaultSrcID = DEFAULT_ID;
            localecho = false;
            cts_in = false;
            msg_in = 0;
            msg_out = 0;
            time_last_msg = time(NULL);
            timeNextPageAllowed = 0;
            busyChannelStartTime = 0;
            bclState = WaitForCD;
            timeOutputUnpaused = 0;
            timeInputUnpaused = 0;
            ShouldConnect = false;
            ForceReset = false;
    };

 // Initialize tty port settings (PDS format for setings )
 bool   rs232::Configure(int br, string s)
        {
            int d;
            int ns;
            char p;

            if (s.size() < 3)
                return false;      // improper format

            d = s.at(1) - '0';      // the data bits
            ns = s.at(2) - '0';
            p = toupper(s.at(0));
            if (((d>=7) && (d<= 8) && (ns >= 1) && (ns <=2)) && ((p=='E') || (p=='O') || (p=='M') || (p=='N') || (p=='S'))){
                 stopbits = ns;
                 databits = d;
                 parity = p;
                 return true;
            }
            return false;
        };

bool rs232::ReOpen() {
    if(handle > 0){
        // Technically, handle 0 should be allowed but we haven't tested
        // all code paths here. Handle 0 is probably stdin
        close(handle);
    }
    
    return OpenComPort();
}    

    // Tries to open RS232 device ttySx.
    // Return -1 if fails. Return device number if it does not fail.
    // The baud rate must previously been set before calling this.
bool rs232::OpenComPort(void)
    {
        ShouldConnect = true;
    
        struct termios options;          // The rs232 port option structure used in SerialPort.h
        if (devicename.size()== 0)
            return false;  // no device name, so quit. 
        bytes_in = 0;
        bytes_out = 0;

        fullname = "/dev/" + devicename;
        
        // Open all the COM ports used on this device
        handle = open(fullname.c_str() , O_RDWR | O_NOCTTY | O_NDELAY);
        
        if (handle == -1) {
            // Error. Cannot open the port
            return false;
	} else {
            // Opened OK.
            fcntl(handle, F_SETFL, 0); //Set the file status flags part of the descriptor's flags to 0
	}
       if (handle >= 0){
            tcgetattr(handle, &options);
            switch (baudrate)
            {
                case 300:
                   cfsetispeed(&options, B300);
                   cfsetospeed(&options, B300);
                   break;
                case 1200:
                   cfsetispeed(&options, B1200);
                   cfsetospeed(&options, B1200);
                   break;
                case 2400:
                   cfsetispeed(&options, B2400);
                   cfsetospeed(&options, B2400);
                   break;
                case 4800:
                   cfsetispeed(&options, B4800);
                   cfsetospeed(&options, B4800);
                   break;
                case 9600:
                   cfsetispeed(&options, B9600);
                   cfsetospeed(&options, B9600);
                   break;
                case 19200:
                   cfsetispeed(&options, B19200);
                   cfsetospeed(&options, B19200);
                   break;
                case 38400:
                   cfsetispeed(&options, B38400);
                   cfsetospeed(&options, B38400);
                   break;
                case 57600:
                   cfsetispeed(&options, B57600);
                   cfsetospeed(&options, B57600);
                   break;
                case 115200:
                   cfsetispeed(&options, B115200);
                   cfsetospeed(&options, B115200);
                   break;
                case 230400:
                   cfsetispeed(&options, B230400);
                   cfsetospeed(&options, B230400);
                   break;
            }
            // Apply the settings
            /*
            CRTSCTS : output hardware flow control (only used if the cable has
                      all necessary lines. See sect. 7 of Serial-HOWTO)
            CS8     : 8n1 (8bit,no parity,1 stopbit)
            CLOCAL  : local connection, no modem contol
            CREAD   : enable receiving characters
            */

            // Enable the receiver and set local mode...
	    options.c_cflag |= (CLOCAL | CREAD);
            options.c_cflag &= ~CSIZE;    // clear the size bits

            int optflag = CS8;  // default to 8 data bits
            switch (databits){
                case 5:
                    optflag = CS5;
                    break;
                case 6:
                    optflag = CS6;
                    break;
                case 7:
                    optflag = CS7;
                    break;
                case 8:
                    optflag = CS8;
                    break;
            }
            options.c_cflag |= optflag;

            if (stopbits == 2)
                options.c_cflag |= CSTOPB;  // 2 stop bits
            else
               options.c_cflag &= ~CSTOPB;  // 1 stop bit

            // Now configure parity
    	    options.c_cflag &= ~PARENB;   // default to no parity
            parity = toupper(parity);
            switch (parity){
                case 'N':
           	    options.c_cflag &= ~PARENB;
                    break;
                case 'E':
                    options.c_cflag |= PARENB;   // enable parity
                    options.c_cflag &= ~PARODD;  // not odd (Even)
                    break;
                case 'O':
                    options.c_cflag |= PARENB;  // enable parity
                    options.c_cflag |= PARODD;  // odd parity
                    break;
                case 'M':
                    // mark parity is simulated by two stop bits
                    options.c_cflag &= ~PARENB;
                    options.c_cflag |= CSTOPB;
                    break;
                case 'S':
                    // space parity is setup the same as no parity
                    options.c_cflag &= ~PARENB;
                    break;
            }

            switch (flowcontrol){
                case 'H':
                    options.c_cflag |= CRTSCTS;
                    break;
                default:
                    options.c_cflag &= ~CRTSCTS;
                    break;
            }
            
            // Store the options
            // options.c_lflag = ICANON;       // disable all echo functionality, and don't send signals to calling program
            options.c_lflag = 0;            /* set input flag non-canonical, no processing, no echo */
//            options.c_lflag = ICANON;       // disable all echo functionality, and don't send signals to calling program
//            options.c_lflag &= ~ECHO;       /* disable echo */
            options.c_iflag = IGNPAR;       /* ignore parity errors */
            options.c_oflag = 0;            /* set output flag non-canonical, no processing */
            options.c_cc[VTIME] = 0;        /* no time delay */
            options.c_cc[VMIN] = 0;         /* no char delay */

            // Save the new attribute. Now.
            if (tcsetattr(handle, TCSANOW, &options) < 0)
                // Error handler. Cant set attributes.
                return -1;

            fcntl(handle, F_SETFL, FNDELAY);   // set the port to non-blocking reads
  
       }

       // Erase the data buffer
       MyParser.rget = 0;
       MyParser.rput = 0;
       MyParser.rawdata[0]=NUL;
       MyParser.ParsingPort = GetIntVal(devicename);   // Set to the ttyX serial port number
       bytes_in = 0;
       bytes_out = 0;

       if (handle >= 0)
          return true;
       else
          return false;

    };

// read the status of the CTS input pin
bool rs232::CTSin(void){

    int bits;

    if (handle < 0){
        cts_in = false;
        return false;
    }
    
    if(ioctl(handle, TIOCMGET, &bits) < 0){
        // Some error. Reset the port
        ForceReset = true;
    }

    if ((bits & TIOCM_CTS) > 0)
        cts_in = true;
    else
        cts_in = false;

    return cts_in;
}
// read the status of the CTS input pin
bool rs232::DSRin(void){

    int bits;

    if (handle < 0){
        dsr_in = false;
        return false;
    }

    if(ioctl(handle, TIOCMGET, &bits) < 0){
        // Some error. Reset the port
        ForceReset = true;
    }

    if ((bits & TIOCM_DSR) >= 0) {	// 08-02	Change from '> 0'. Allowed Cigorn on MiniBox to detect serial ports
        dsr_in = true;	
    }
    else {    
	dsr_in = false;
    }

    return dsr_in;
}

/**
 * Read the status of the CD pin
 * @return True if CD is high
 */
bool rs232::CDin(void){
    int bits;

    if (handle < 0){
        return false;
    }

    if(ioctl(handle, TIOCMGET, &bits) < 0){
        // Some error. Reset the port
        ForceReset = true;
    }

    if ((bits & TIOCM_CD) != 0)
        return true;
    else
        return false;
}

void rs232::setIncomingFlowStatus(bool allowInboundFlow){
    int rtsBit = TIOCM_RTS;
    int result;
    if(allowInboundFlow){
        result = ioctl(handle, TIOCMBIS, &rtsBit);
    }else{
        result = ioctl(handle, TIOCMBIC, &rtsBit);
    }
    
    if(result < 0){
        // Some error. Reset the port
        ForceReset = true;
    }
}

/**
 * Gets the number of bytes in the serial ports output queue at the OS level.
 * @return Queued byte count
 */
int rs232::queuedBytes(void){
    int queueLength;
    
    if(ioctl(handle, TIOCOUTQ, &queueLength) < 0){
        // Some error. Reset the port
        ForceReset = true;
    }
    
    if(queueLength < 0){
        return 0;
    }
}

// Output a string to the RS232 serial port
int rs232::SendString(std::string S)
 {
     int bytes_written;
     int i;

     if (handle == -1)
         return -1;  // This port is not open.

     for (i=0; i< S.size(); i++)
         S[i] = S[i] & 0x7f;   // not control chars
     
     // Write the string to the serial port
     bytes_written = write(handle, S.c_str(), S.size());
     bytes_out += bytes_written;                       // count the bytes we send out
 
     msg_out++;  // count this message

     // cout << S;
     return bytes_written;

 }

// Send count number of bytes from the *bytes array.
int rs232::SendBytes(char *bytes, int count){

     int bytes_written = 0;
     int pointer = 0;
     //int y;
     if (handle == -1)
         return -1;  // This port is not open.

     if (count > MAXSERIALBUFFSZ)
         return -1;    // too much to send at one time
     
     // Write the data bytes to the serial port
     while (pointer < count){
        //y = bytes[pointer];
        int bytesOut = write(handle, &bytes[pointer], 1);  // one byte at a time to the serial port
        if(bytesOut != 1){
            cout << "Failed to write to serial. Data has been lost!";
        }
        bytes_written += bytesOut;
        pointer++;
        bytes_out++;
       // cout << bytes[pointer]; // count the bytes we send out
        //cout << IntToHex(y, 2) << " ";
     }
     msg_out++;  // count this message

     //cout << endl;
     return bytes_written;
}

    // Get a singe charactor from the buffer. Return false if none available
    bool rs232::GetChar(char* c){

       char buffer[255];  /* Input buffer */
       int  nbytes;       /* Number of bytes read */
 

       /* read characters into our string buffer until we get a CR or NL */
       nbytes = read(handle, &buffer[0], 1);

       /* nul terminate the string and see if we got an OK response */
       if (nbytes > 0){
            *c = buffer[0];
            if (localecho)
                cout << *c;
            time_last_msg = time(NULL);   // remember this time

            return true;
        }
       else
         return false;
    }

    // Poll the serial port, and load-up the buffer with data if there is some comming in.
   int rs232::GetChars(void){

       int  nbytes;       /* Number of bytes read */
       bool done = false;
       char buffer[260];
       int  bytesread = 0;

       while ((done == false) && (bytesread < 255)) {
           // read characters into our string buffer
           nbytes = read(handle, &buffer[bytesread], 1);
           if (nbytes > 0){
               bytes_in = bytes_in + nbytes;
               bytesread = bytesread + nbytes;
               time_last_msg = time(NULL);   // remember this time
           }else if(nbytes < 0){
               // Some read error. Re-init the port
               ForceReset = true;
               
               done = true;
           }else{
               done = true;
           }
       }
       if(bytesread == 255){
           cout << "Hit byte processing limit at " << devicename << endl;
       }

       if(isInputPaused()){
           // Discard the data and continue
           return bytesread;
       }
       
       if (bytesread > 0){
           //cout << "B:" << buffer;
           buffer[bytesread] = NUL; // make sure we are null terminated.
           if (localecho)
              cout << buffer;
           msg_in = msg_in + MyParser.parse(buffer, bytesread, myDevIndex, myDevType);   // parse the data
       }
       return  bytesread;
   }


// return the number of hours since last message
double rs232::ActivityTime(void){
    double d;
    int i;

    if (bytes_in == 0)
        return -1;

    time_t time_here = time(NULL);          // The time we got the last message in from this port

    d = difftime(time_here, time_last_msg)/(60 * 60);

    return d;
}

    bool rs232::isInputPaused(){
        return TimeNow() < timeInputUnpaused;
    }

    bool rs232::isOutputPaused(){
        return TimeNow() < timeOutputUnpaused;
    }

    /**
     * Pauses input until the specified time. If input is already paused
     * for a greater amount of time than this would cause input to be paused
     * for, has no effect. If input is paused but would end before unpauseTime,
     * extends the pause duration to unpauseTime
     * @param unpauseTime Time to pause input for
     */
    void rs232::pauseInputUntil(double unpauseTime){
        if(unpauseTime > timeInputUnpaused){
            timeInputUnpaused = unpauseTime;
        }
    }
    
    /**
     * Same effect as @see pauseInputUntil(), for output.
     * @param unpauseTime Time to pause input for
     */
    void rs232::pauseOutputUntil(double unpauseTime){
        if(unpauseTime > timeOutputUnpaused){
            timeOutputUnpaused = unpauseTime;
        }
    }

    void rs232::unpauseInput(){
        timeInputUnpaused = 0;
    }

    void rs232::unpauseOutput(){
        timeOutputUnpaused = 0;
    }
}
