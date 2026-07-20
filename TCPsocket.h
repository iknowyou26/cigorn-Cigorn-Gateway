/* 
 * File:   TCPsocket.h
 * Author: Ryan le
 *
 * Created on july 15, 9:34 PM
 */

#ifndef _TCPSOCKET_H
#define	_TCPSOCKET_H
#include "dataparser.h"
#include "BinaryEntry.h"
#include <queue>
#include <sstream>
#include "platform/Platform.h"
#include "platform/thread/PlatformMutex.h"
#define MAXBUFSIZE  10000   // The largest amount of data we can pass to/from the socket in one chunk.
#define pServer     0       // TCP/IP Server
#define pClient     1       // TCP/IP Client
#define pDGRAMTX    2       // UDP sender of datagrams
#define pDGRAMRX    3       // UDP listener for datagrams


#define TRYTIME     5       // Time in decimal seconds to wait try to reconnect the socket if it fails the first time
#define MAXCONTRIES 5       // maximum number of times we try to connect to a client before restarting socket from scratch.
#define CLOSEDELAY  50000   // 50mS to close an open socket
using namespace std;


typedef map<int, int> PortMapping;       // map the port number to the device designator


// Prototype the functions


class tcpnet {
public:
    tcpnet();
    tcpnet(const tcpnet& orig);
    virtual ~tcpnet();
    void clear(void);
    int CreateSocket(string, int , std::string, std::string, int , bool, int );
    int tcp_socket();
    bool initialize_socket();
    bool sendbytes(char*, int);
    bool sendbytes(string);
    bool sendtext(string);
    bool ConnectSocket(void);
    bool DisconnectSocket(void);
    bool DisconnectClient(void);
    string Protocol(void);
    string CheckURLorIP(string);  // see if it is a valid URL or Ip address, and return the IP address string if it is.
    double ActivityTime(void);        // how many hours since last reception  -1 for never
    void freesocket(void);
    int GetFreePortNum(int, int);

    // Socket settings
    struct sockaddr_in  MyAdd;      // will hold my address info for this computer's IP address
    struct sockaddr_in  CliAddr;    // will hold the IP address info for the client we connect/send to.

    dataparser MyParser;            // a data parser for the raw data that comes in

    int  index;              // The 0-based index of this object in the tcpsockets array of objects
    int  myDevDesIndex;      // The index of the device designator device[] this socket will service
    int  myDevType;          // The index to the device hardare type (modem, radio, server,...) that this socket talks to
    int  portnum;            // the local port we use with this socket connection
    int  sockfd;             // The file descriptor for this socket
    int  newsockfd;
    int  clienttimeout;     // The maximum time to go without data before we disconnect.

    cigorn::PlatformMutex qlock;
    cigorn::PlatformMutex txbufflock;
    
    queue <BinaryEntry> MsgQout; // Messages that are queued up to be sent out this socket.

    char rxbuff[MAXBUFSIZE];     // The data buffer for the message we are currently processing.
    char txbuff[MAXBUFSIZE];     // the received bytes we bring in via this socket
    int  txcount;                // number of bytes in the txbuff that need to be sent out the socket

    string description;          // human readable description of what this socket is for

    char rxRaw[MAXBUFSIZE];     // the received bytes we bring in via this socket, unprocessed. Used to directly pipe data
    int  rxRawCount;            // number of bytes in the raw rx buffer

    // Socket statistics
    int init_tries;            // Number of times we try to create the socket
    int client_tries;          // number of times we tried to connect as a client
    int connects;              // number of times we connect to another machine.
    long bytes_out;
    long bytes_in;
    long msg_in;               // count the messages in/out
    long msg_out;              //
    bool connected;            // true when connected to an end point.
    long session_bytes_out;    // Number of bytes sent during this connection
    int  destID;               // The destination ID of the last message we send out this socket.
    int  srcID;                // The source ID of the last message we send out this socket.
    time_t time_last_in;       // The time we got the last message in from this socket
    time_t time_last_out;      // The time we last sent a message out this socket
    time_t time_last_activity; // The last time in/out or socket connected.
    int protocol;              // The index of the protocol to communicate with
    string interface;          // the interface to use for the connection.  NULL = any available
    bool enable_keepalive;     // true to enable the use of the socket keep-alive feature
    
    string myIP4;                     // The local IP address of the ethernet interface this socket is using
    string ConnectedToIP;                  // The IP address of the remote host we actually connected to.
    int RemotePort;                   // The port on the remote host we actually are talking to
    
    string hostaddress;               // The IP address or URL of the remote host we want to connect to.
    string hostIPaddress;             // The resolved IP address of the remote host we will try to connect to as a client or UDP sender
    int hostport;                     // The port on the remote host we want to talk to.
    bool localecho;
    static const int MaxBuffSize = MAXBUFSIZE;
    std::stringstream sockconsoleout;    // stream to hold socket debug info
    bool echoinput;
    bool NLtoCRLF;                       // set true to translate NL or LF to CRLF combo.  Default is false.
    bool ForceClientDisconnect;          // set true to force the socket to discoonnect from the remote host.
    
    bool isInputPaused();
    bool isOutputPaused();

    void pauseInputUntil(double unpauseTime);
    void pauseOutputUntil(double unpauseTime);
    void unpauseInput();
    void unpauseOutput();

private:
    double timeOutputUnpaused;
    double timeInputUnpaused;
    double init_time, connect_time;    // used to keep track of time between connection attempts

};

#endif	/* _TCPSOCKET_H */

