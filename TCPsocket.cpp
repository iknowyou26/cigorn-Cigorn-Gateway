

/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "dataparser.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>    // Utilities for networking
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <fcntl.h>
#include "procinfo.h"
#include "SocketThread.h"
#include "network.h"

#undef __USE_MISC
#include <net/if.h>

#include "errno.h"  // added by Chad 4-11

#include "Cigorn.h"     // Our application-specific constants and headers
#include "dataparser.h"
#include "TCPsocket.h"
#include "CommThread.h"
#include "microsleeper.h"

using namespace std;

int getaddrinfo(const char *nodename, const char *servname,
                const struct addrinfo *hints, struct addrinfo **res);

void freeaddrinfo(struct addrinfo *ai);
const char *gai_strerror(int ecode);
int get_iface_list(struct ifconf *);

int ethcount = 0;

// tcpio class constructor
tcpnet::tcpnet() {
    clear();
}

tcpnet::tcpnet(const tcpnet& orig) {
}

tcpnet::~tcpnet() {


}

void tcpnet::clear(void) {
    sockfd = -1;
    connected = false;
    myDevDesIndex = -1;        // indicates not bound to any device designator yet.
    myDevType = -1;
    newsockfd = -1;
    portnum = -1;
    sockfd = -1;
    txcount = 0;
    bytes_in = 0;
    bytes_out = 0;
    index = -1;
    interface = "";
    init_tries = 0;
    hostaddress = "";
    hostport = 0;
    connects = 0;
    myIP4 = "";
    ConnectedToIP = "";
    description = "";
    hostIPaddress = "";
    rxRawCount = 0;
    rxRaw[0] = NUL;
    time_last_in = time(NULL);
    time_last_out = time(NULL);
    time_last_activity = time(NULL);
    client_tries = 0;
    session_bytes_out = 0;
    pthread_mutex_t txbufflock;     // Lock for the tx data buffer
    echoinput = false;
    NLtoCRLF = false;               // by default, no translation.
    enable_keepalive = false;       // be deafault, don't send keep alives.
    ForceClientDisconnect = false;
}

//  **************************************************************************************
// Service the SOCKET
// CALL very often.  Sockets are non-blocking, so this routine must be constantly called.
// Return -1 if we do nothing, or a positive number if we sent or received data.
//  **************************************************************************************
int tcpnet::tcp_socket() {

     struct adr_clnt; // AF_INET
     socklen_t slt;
     int n=0;
     int rdflags = MSG_DONTWAIT;  // Ensure all reads are nonblocking
     struct sockaddr_in adr;      // AF_INET
     int retval = -1;   // -1 not connected, 0=no data, > 0 number of bytes txed or rxed
     double tmout = 0;

     if (ForceClientDisconnect){
         // something wants us to disconnect this socket from client
         DisconnectSocket();
         ForceClientDisconnect = false;   // done doing this.
         return 0;
     }

    // See if the socket is initialized OK and got a File Descriptor
    if (sockfd < 0 ){
        // Socket is not initialized, so first go initialize it
        initialize_socket();
        if (sockfd < 0){
           //cout << "Failed to open port " << portnum << " DD:" << description << endl;
        }
        return -1;   //
    }

    // Safety check to verify that the socket is still nonblocking. Should
    // never happen
    int socketFlags = fcntl(sockfd, F_GETFL);
    if (socketFlags < 0 || !(socketFlags & O_NONBLOCK)){
        // Non-blocking is not set. Bail
        DisconnectSocket();
        return -1;
    }

    switch (protocol){
        case pServer:
            // See if this socket is communicating with another host
            if (newsockfd == -1){
                // we are waiting to connect the socket to some other host who is a TCP client
                ConnectSocket();

                return -1;
            }
            // We have a socket created and are ready to exchange data.
            // See if the remote host connects to it and sends data, or if we need to send data to the remote host
            memset(rxbuff, NUL, MAXBUFSIZE);   // Clear out the RX data  buffer
            n = recv(newsockfd, rxbuff, MAXBUFSIZE, rdflags );   // non-blocking read of incomming data
            break;
        case pDGRAMTX:
            // We normally only initialize socket if there is data to send
            if (MsgQout.size() > 0){
                // We do have data to send, so try to call the server
                // cout << "Connect me to" << hostaddress << endl;
                ConnectSocket();   // setup the socket
                if (client_tries > MAXCONTRIES){
                    CoutM1(sockconsoleout) << "Trying to initialize #" << myDevDesIndex << "(" << description << ") to client. Re-initializing my port:" << portnum << endl;
                    initialize_socket();
                }
                MyCLI.Display(&sockconsoleout);
                n = -1;     // no receive data on a sending port. Set n to -1 to fall through RX logic.
            }else{
                MyCLI.Display(&sockconsoleout);
                return -1;  // nothing to do now.
            }
            break;
        case pDGRAMRX:
            // We normally only connect if there is data to send to the server
            if (sockfd == -1){
                // have not created a listneing socket.
                initialize_socket();
                return -1;
            }
            memset(rxbuff, NUL, MAXBUFSIZE);   // Clear out the RX data  buffer
            // n = recv(sockfd, rxbuff, MAXBUFSIZE, rdflags );   // non-blocking read of incomming data
            slt = sizeof(adr);
            n = recvfrom(sockfd, rxbuff, sizeof rxbuff, 0,  (struct sockaddr *)&adr, &slt);   // non-blocking read of incomming data
            if (n < 0){
                return -1;
            }else
                CoutM2(sockconsoleout) << description << " UDP receive " << n << " bytes" << endl;
            break;
        case pClient:
            // See if this socket is communicating with another host
            if (newsockfd == -1){
                // we are not connected now, should we?
                if (clienttimeout < 1){
                    // We do not time connections, so always try to connect to the server
                    if (client_tries > MAXCONTRIES){
                        CoutM2(sockconsoleout) << "Tried to connect to client " << client_tries << " times to client. Re-initializing " << description << endl;
                        initialize_socket();
                    }
                    ConnectSocket();
                }else{
                    // We normally only connect if there is data to send to the server
                    if (MsgQout.size() > 0){
                        // We do have data to send, so try to call the server
                        // cout << "Connect me to" << hostaddress << endl;
                        if (client_tries > MAXCONTRIES){
                            CoutM1(sockconsoleout) << "Trying to connect #" << myDevDesIndex << "(" << description << ") to client. Re-initializing my port:" << portnum << endl;
                            initialize_socket();
                        }
                        ConnectSocket();
                    }
                }
                MyCLI.Display(&sockconsoleout);
                return -1;
            }else{
                // We are conencted to a server right now.
                if (clienttimeout > 0){
                    // See if we timed out and should disconnect from the server
                    tmout = difftime(time(NULL), time_last_activity);
                    if ((tmout > clienttimeout)){
                        // Time to disconnect. Nothing happening...
                        CoutM1(sockconsoleout) << "Connection timed out. Port:" << portnum << " Disconnecting." << endl;
                        DisconnectClient();
                        MyCLI.Display(&sockconsoleout);
                        return -1;
                    }

                }
            }
            // We have a socket created and are ready to exchange data.
            // See if the remote host connects to it and sends data, or if we need to send data to the remote host
            memset(rxbuff, NUL, MAXBUFSIZE);   // Clear out the RX data  buffer
            n = recv(newsockfd, rxbuff, MAXBUFSIZE, rdflags );   // non-blocking read of incomming data
            break;
        default:
            MyCLI.Display(&sockconsoleout);
            return -1;
            break;
    }


     // Did the remote host disconnect from us?
     if (n == 0){
         // Come here when the remote disconnects or we sensed a connection loss because
         // keep-alive messages are not responded to.
         CoutM1(sockconsoleout) << "DeviceDesignator:" << description << " disconnected from " << ConnectedToIP << endl;
         CoutM2(sockconsoleout) << " Disconnected from " << ConnectedToIP << " sockets:" << sockfd << "|" << newsockfd << " my port:" << portnum << endl;
         DisconnectClient();
         MyCLI.Display(&sockconsoleout);
         return -1;
     }
     
     // We are connected. See if this is the first time through this routine
     if (connected == false){
         switch (protocol){
            case pServer:
                // We just connected to another computer, so note this.
                connected = true;
                // cout << "HERE " << connects << endl;
                CoutM1(sockconsoleout) << "TCP socket connected " << description << " on " << myIP4 << ":" << portnum
                     << " to host " << ConnectedToIP << ":" << RemotePort  << " Count=" << connects << endl;
                connects++;
                break;
            case pClient:
                // We just connected to another computer, so note this.
                connected = true;
                // cout << "HERE " << connects << endl;
                CoutM1(sockconsoleout) << "TCP socket connected " << description << " on " << myIP4 << ":" << portnum
                     << " to host " << ConnectedToIP << ":" << RemotePort  << " Count=" << connects << endl;
                connects++;
                break;
           case pDGRAMTX:
                // We just connected to another computer, so note this.
                connected = true;
                // cout << "HERE " << connects << endl;
                CoutM1(sockconsoleout) << "TCP send by " << description << " on " << myIP4 << ":" << portnum
                     << " to host " << ConnectedToIP << ":" << RemotePort  << " Count=" << connects << " fd=" << sockfd << endl;
                connects++;
                break;
           case pDGRAMRX:
                // We just connected to another computer, so note this.
                connected = true;
                // cout << "HERE " << connects << endl;
                CoutM1(sockconsoleout) << "TCP receive by " << description << " on " << myIP4 << ":" << portnum
                     << " to host " << hostIPaddress << ":" << hostport   << " Count=" << connects << " fd=" << sockfd << endl;
                connects++;
                break;
        }

    }

    MyCLI.Display(&sockconsoleout);   // output the string to the Command-Line user and then erase it.

    if (n > 0){
        // We got n valid data bytes in

        if (localecho)
           sockconsoleout << "DataIn[" << portnum << "," << n << "]" << rxbuff << endl;
        bytes_in += n;
        time_last_in = time(NULL);
        time_last_activity = time(NULL);

        if(!isInputPaused()){
            // Store the rx data into the raw buffer in cse we are piping directly to another interface
            if ((rxRawCount < 0) || (rxRawCount >= MAXBUFSIZE))
                rxRawCount = 0;   // range check
            if (n < (MAXBUFSIZE - rxRawCount))
            memcpy(&rxRaw[rxRawCount], rxbuff, n);      // store the raw into our buffer

            if (echoinput){
               if (NLtoCRLF && (rxbuff[0] == CR)) {
                   // add a NL to a CR if we are supposed to translate
                   sendbytes("\n");
               }
               sendbytes(rxbuff, n);   // echo them back out the socket if we are supposed to echo them back out
            }
            // cout <<"NN" << n << endl;
            msg_in = msg_in + MyParser.parse(rxbuff, n, myDevDesIndex, myDevType);   // parse the data
        }else{
            // Do nothing, just discard the data
        }
        
        connected = true;
        retval = n;
    }
     

     // We are connected, so take messages out of the q and put them in the data buffer for transmission
     pthread_mutex_lock(&qlock);  // Lock the q while we manipulate it
     if (MsgQout.size() > 0 && !isOutputPaused()){
         // There is a message for us to send out the socket
         // CoutM2(skout) << " Q size= " << MsgQout.size() << endl;
         time_last_out = time(NULL);        // remember the time we last sent a message out
         time_last_activity = time(NULL);
         BinaryEntry TopEntry;
         TopEntry = MsgQout.front();  // get a reference to the top entry
         if ((TopEntry.bcount < (MAXBUFSIZE - txcount)) && (TopEntry.bcount > 0)){
             try {
                    MsgQout.pop();   // remove the entry from the q. Throws exception sometimes.
                    //cout << DisplayHexData(TopEntry.data, TopEntry.bcount) << endl;
                    //cout << "Dx:" << TopEntry.data << endl;
                    if (session_bytes_out == 0){
                        // The first time we are sending anything on this socket
                        destID = TopEntry.dstID;  // remember the ID of the thing we first sent data to.
                        srcID = TopEntry.srcID;   // remember the ID of the thing we first sent data to.
                        CoutM1(sockconsoleout) << "First message on EthDevDes:" << description << endl;
                    }
                    sendbytes(TopEntry.data, TopEntry.bcount);  // put the message into the data q to be sent out the socket
                    session_bytes_out = session_bytes_out + TopEntry.bcount;
	     }
	        catch (exception& e) {
	     }
         }
     }
     pthread_mutex_unlock(&qlock); // unlock it

     // Integrety check
     if (txcount > MAXBUFSIZE){
         // Very bad. FLush the buffer
         pthread_mutex_lock(&txbufflock);
         txcount = 0;
         rxbuff[0] = NUL;
         pthread_mutex_unlock(&txbufflock);
     }

     // See if it is time to send some data out
     if (txcount > 0){
        // There is data to send
        pthread_mutex_lock(&txbufflock);
        // cout << description <<  " Dx:" <<  txcount << endl;
        txbuff[txcount] = NUL;  // null terminate it
        if (localecho)
           sockconsoleout << "DataOut[" << portnum << "," << txcount << "]" << txbuff << endl;

        switch (protocol){
            case pServer:
                n = write(newsockfd, txbuff, txcount);   // might not write all the data  TODO
                break;
            case pClient:
                n = write(newsockfd, txbuff, txcount);   // might not write all the data  TODO
                break;
           case pDGRAMTX:
                slt = sizeof(CliAddr);
                n = sendto(sockfd, txbuff, txcount, 0, (struct sockaddr *)&CliAddr, slt);   // might not write all the data  TODO
                if (n<0){
                    CoutM2(sockconsoleout) << description <<  " UDP send " <<  txcount <<  "bytes Failed. errno:" << errno << endl;
                }else{
                    CoutM2(sockconsoleout) << description <<  " UDP send " <<  n << " bytes" << endl;
                }
                break;
           case pDGRAMRX:
                n = write(sockfd, txbuff, txcount);   // might not write all the data  TODO
                break;
            default:
                n = write(newsockfd, txbuff, txcount);   // might not write all the data  TODO
        }

        //CoutM2(skout) << " Data sent to " << ConnectedToIP << " on:" << description << " txcount=" << txcount << "  n=" << n << endl;

        if (n < 0){
           // problem with the connection
           DisconnectSocket();
           txcount = 0;        // tx buffer is empty
        }
        else{
            txbuff[0] = NUL;
            txcount = 0;        // we write them all so buffer is empty
            connected = true;
            bytes_out += n;     // count the number of bytes we send out the socket
            retval = n;
        }
        pthread_mutex_unlock(&txbufflock);
     }
     MyCLI.Display(&sockconsoleout);   // output the string to the Command-Line user and then erase it.

    return retval;
}

// Initialized the socket, and try to bind it.
// prot = 0(serer)  1(client)
bool tcpnet::initialize_socket(){
     char ch[255];
     struct addrinfo hints;
     int yes=1;
     int no=0;
     int GetResult = -1;
     int myadd;
     stringstream ss;
     struct addrinfo *servinfo, *p;
     int optval;
     socklen_t optlen;

     // Reset the statistics for this socket
     if (newsockfd >= 0){
        shutdown(newsockfd, SHUT_RDWR);
        if (close(newsockfd) != 0){
            // Failed to close socket
            // cout << "Failed to close client connection socket " << << " fd:" << newsockfd << endl;
        };
     }
     if (sockfd >= 0){
        shutdown(sockfd, SHUT_RDWR);
        if (close(sockfd) != 0){
            // Failed to close socket
            // cout << "Failed to close socket " << sockfd << endl;
        };
     }
     connected = false;
     newsockfd = -1;
     sockfd = -1;
     ConnectedToIP = "";                                   // The IP address and port of the remote machine we talk to
     RemotePort = -1;
     session_bytes_out = 0;

     MyParser.ParsingPort = portnum;                       // let the parser know our port number
     MyParser.initialize();                                // restart the parsing logic
     client_tries = 0;

     // tcpsockets[tcpindex].portnum = SERVER_PORT_BASE + tcpsockets[tcpindex].index;

     connect_time = TimeNow();   // decimal seconds, mS resolution
     time_last_activity = time(NULL);

     // myadd = getIPv4(interface);

     // make a socket address hint for the interface we want to use to connect this socket.
     struct sockaddr_in serv;
     memset(&serv,0,sizeof serv);
     serv.sin_family = AF_INET;
     serv.sin_port = htons(portnum);
     serv.sin_addr.s_addr = inet_addr(myIP4.c_str());   //INADDR_ANY;//inet_addr(IP);

     memset(txbuff, NUL, MAXBUFSIZE);                   // erase out data buffer
     init_tries++;

     memset(&hints, 0, sizeof hints);

      switch (protocol){
        case pServer:
            hints.ai_family = AF_INET;        // use IPv4
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            break;
        case pClient:
             portnum = GetFreePortNum(CLIENT_PORT_BASE, index);
             hints.ai_family = AF_INET;        // use IPv4
             hints.ai_protocol = IPPROTO_TCP;
             hints.ai_socktype = SOCK_STREAM;
             hints.ai_flags = AI_PASSIVE;
             break;
        case pDGRAMTX:
             portnum = GetFreePortNum(CLIENT_PORT_BASE, index);
             hints.ai_family = AF_INET;        // use IPv4
             hints.ai_protocol = IPPROTO_IP;
             hints.ai_socktype = SOCK_DGRAM;
             hints.ai_flags = AI_PASSIVE;
             break;
        case pDGRAMRX:
             hints.ai_family = AF_INET;        // use IPv4
             hints.ai_protocol = IPPROTO_IP;
             hints.ai_socktype = SOCK_DGRAM;
             hints.ai_flags = AI_PASSIVE;
             break;
         default:
            hints.ai_family = AF_INET;        // use IPv4
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            break;
       }

    if (portnum  <= 0)
        return false;   // can't do anything without a valid port number

    // OPtional IP address hints to get the right ethernet interface card
    //hints.ai_addrlen = sizeof(serv);
    //hints.ai_addr = (sockaddr*)&serv;


    to_cstring(ch, portnum, 8);           // Convert the localport number to a c string

    // getaddrinfo() returns a list of address structures.   Try each address until we successfully bind(2).
    // If socket() or bind()) fails, we close the socket and try the next address.

    init_time = TimeNow();  // the time we built this socket
    connect_time = 0;       // last time we attempted to connect

    GetResult = getaddrinfo(NULL, ch, &hints, &servinfo);    // Get the IP address info for this machine
    if (GetResult != 0){
        // CoutM2(skout) << "Cant init socket for " << description << endl;
        if(servinfo != NULL){
            freeaddrinfo(servinfo);
        }
        return false;   // failed to find any interfaces to use
    }

    // The IP address of the interface we want to use
    myadd = inet_addr(myIP4.c_str());

    pthread_mutex_lock(&socklock);              // lock the sockets while we manipulate them

    char s[INET6_ADDRSTRLEN];

    // loop through all the interface results and bind to the first we can use for this socket
    for(p = servinfo; p != NULL; p = p->ai_next) {

           // See if this is the IP address of the interface we want to use.
           sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
           if (sockfd == -1) {
              // cout << "Can't create socket for port " << portnum << endl;
               continue;
           }

           // Chad 4-11
           int n = fcntl(sockfd,F_GETFL,0);

           if (fcntl(sockfd, F_SETFL, n | O_NONBLOCK) == -1){  // Chad 4-11
                 // We failed to create a non-blocking socket. Clear and
                 // try next servinfo
                 shutdown(sockfd, SHUT_RDWR);
                 close(sockfd);
                 sockfd = -1;
                 //CoutM2(skout) << "Cant init socket for " << description << endl;
                 continue;                     // we must be non-blocking. fail.
           }

           if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                CoutM2(sockconsoleout) << "Cant get socket options for "  <<  description << " on my port " << portnum << endl;
                MyCLI.Display(&sockconsoleout);
                break;
           }else{
                // Seems the socket is OK and can be configured.  
                // Set the linger option.  no=dump all data when closing.
                setsockopt(newsockfd, SOL_SOCKET, SO_LINGER, &no, sizeof(int));
                if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                   if (close(sockfd) != 0){
                      // Failed to close socket
                      CoutM2(sockconsoleout) << "Failed to close socket on port" << portnum << " fd:" << newsockfd << endl;
                      MyCLI.Display(&sockconsoleout);
                   };
                   if (init_tries < 3){
                        ss << "Error: Can't bind socket:#" << index  << "  Port:" << portnum << " on " <<  myIP4;
                        elog.store(ss.str());  // log the error that we could not initialize this socket
                   }
                   continue;
                }
                else{
                    // Success.We have a port to use for this socket.
                    break;
                }
          }
        } // for...

       pthread_mutex_unlock(&socklock);  // unlock the socket

       if (p == NULL)  {
            // Failed
            sockfd = -1;   // failed
            freeaddrinfo(servinfo); // free the linked list
            if (init_tries <=1 ){
               CoutM2(sockconsoleout) << "Failed to open socket for port:" << portnum << " on " <<  interface << endl;
               ss.str("");
               ss << "Error: Can't bind socket:#" << index  << "  Port:" << portnum << " on " <<  myIP4;
               elog.store(ss.str());  // log the error that we could not initialize this socket
            }
            return false;
        }
       MyCLI.Display(&sockconsoleout);

       // Following line throws exception some times
       freeaddrinfo(servinfo); // free the linked list

       // verify we have an IP address, if it is a URL, look it up.
       // CoutM2(sockconsoleout) << "Initializing: " << description << endl;
       MyCLI.Display(&sockconsoleout);

       switch (protocol){
            case pServer:
                //coutm1 << "Initializing TCP Server " << description << " "<< myIP4 << ":" << intToString(portnum) << "  FD:" << sockfd << endl;
                CoutM2(sockconsoleout) << "Initializing TCP Server " << description << " "<< myIP4 << ":" << intToString(portnum) << "  FD:" << sockfd << endl;
                break;
            case pClient:
                // A client shoudl validate the IP address we are trying to connect to
                hostaddress = trim(hostaddress);
                hostIPaddress = CheckURLorIP(hostaddress);
                if (enable_keepalive)
                    optval = 1;   // TRUE
                else
                    optval = 0;   // FALSE   no keep alive messages
                optlen = sizeof(optval);
                /// enable keep alive messages
                if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0){
                    CoutM2(sockconsoleout) << "Failed to set socket option. Port: " << portnum  << endl;
                    close(sockfd);
                    return false;
                }
                MyCLI.Display(&sockconsoleout);
                CoutM2(sockconsoleout) << "Initialized TCP client " << description << " on my " << myIP4 << ":" << intToString(portnum )  << " to " << hostaddress << ":" << hostport;
                if (enable_keepalive){
                    CoutM2(sockconsoleout) << " Keepalive = TRUE" << endl;
                }
                else{
                    CoutM2(sockconsoleout) << " Keepalive = FALSE" << endl;
                }
                MyCLI.Display(&sockconsoleout);
                break;
           case pDGRAMTX:
                // A client should validate the IP address we are trying to connect to
                hostaddress = trim(hostaddress);
                hostIPaddress = CheckURLorIP(hostaddress);
                CoutM2(sockconsoleout) << "Initialized UDP sender " << description << " on my " << myIP4 << ":" << intToString(portnum )
                              << " to " << hostaddress << ":" << hostport << endl;
                MyCLI.Display(&sockconsoleout);
                break;
           case pDGRAMRX:
                // A client should validate the IP address we are trying to connect to
                hostaddress = trim(hostaddress);
                hostIPaddress = CheckURLorIP(hostaddress);
                CoutM2(sockconsoleout) << "Initialized UDP listener " << description << " on my " << myIP4 << ":" << intToString(portnum) << endl;
                MyCLI.Display(&sockconsoleout);
                break;
        }

       //  Send any debug messages out to the CLI
       //  if (skout.str().size() > 0)
       MyCLI.Display(&sockconsoleout);

       return true;
}


#include <errno.h>
#include <math.h>


// Try to connect to a remote host if we are a client,
// Check for a client connection if we are a server
bool tcpnet::ConnectSocket(void){

    double ts;
    int clilen;
    int linger_onoff;
    int err=0;
    socklen_t slt;
    struct sockaddr_in serv_addr;
    int yes=1;
    static int AttemptCount = 0;

    // we are waiting to connect the socket to some other host
    ts = TimeNow() - connect_time;   // see how long its been since last time we tried to connect to a remote host.

    AttemptCount++;

    switch (protocol){
        case pDGRAMTX:
            // we don't connect. Just send UDP datagram.
            AttemptCount = 0;
            // Configure the IP address of the host we send the UDP packets to
            memset( (char *) &CliAddr, NUL, sizeof(CliAddr));  // Initalize the address structure
            CliAddr.sin_family = AF_INET;
            CliAddr.sin_port = htons(hostport);  // Configure the port number to call on the remote machine.
            CliAddr.sin_addr.s_addr = inet_addr(hostIPaddress.c_str());  // case the address of the remote machine, but convert to C string first.
            ConnectedToIP = inet_ntoa(CliAddr.sin_addr);
            RemotePort = ntohs(CliAddr.sin_port);
            break;
        case pDGRAMRX:
            // we don't connect.
            AttemptCount = 0;
            break;
        case pServer:
            //listen(sockfd, 5);
            if (listen(sockfd, 5) == -1){
                // socket closed or error with port.
                clilen = 0;
                //sockfd = 0;
                DisconnectSocket();
                if (AttemptCount <=1)
                    CoutM2(sockconsoleout) << "Listen failed on " << interface << " port:" << portnum << endl;
                return false;
            };

            // myIP4 = inet_ntoa(((struct sockaddr_in *)&p->ai_addr)->sin_addr);     // unclear if this works???
            // found a client who is trying to connect to us.
            clilen = sizeof(CliAddr);
            slt = clilen;
            newsockfd = accept(sockfd, (struct sockaddr *) &CliAddr, &slt);  // creates a new socket to listen on
            client_tries = 0;   //we don't count client tries in the server mode'

            if (newsockfd >= 0){
                // Set the close() operation to no-linger.  Causes unsent-data to be lost.
                linger_onoff = 0;
                setsockopt(newsockfd, SOL_SOCKET, SO_LINGER, (char *)&linger_onoff, sizeof(linger_onoff));  // chad   not needed

                if (setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                    if (AttemptCount <=1)
                        CoutM2(sockconsoleout) << "Failed to set reuse client socket on my port: " << portnum << endl;
                };

                // Set the new socket to non-blocking
                int n = fcntl(newsockfd,F_GETFL,0);
                if (fcntl(newsockfd, F_SETFL, n | O_NONBLOCK) == -1){
                    // error. Cannot open non-blocking
                    DisconnectSocket();
                    if (AttemptCount <=1)
                        CoutM2(sockconsoleout) << "Error.  Cannot set server socket " << description << " to Non-Blocking on my port " << portnum << "  fd:"  << sockfd << endl;
                    return false;
                }

                // Now get my own IP address
                struct sockaddr_in my;   // will hold my address info for this computer's IP address
                socklen_t socklen = sizeof(struct sockaddr_storage);
                memset(&MyAdd,0,sizeof(&MyAdd));
                if (getsockname(newsockfd,(struct sockaddr *)&my,&socklen) >= 0)
                    myIP4 = inet_ntoa(my.sin_addr);
                else
                    myIP4 = "?";

                memset(&CliAddr,0,sizeof(&CliAddr));
                if (getpeername(newsockfd,(struct sockaddr *)&CliAddr, &socklen) >= 0){
                    ConnectedToIP = inet_ntoa(CliAddr.sin_addr);
                    RemotePort = ntohs(CliAddr.sin_port);
                }
                else{
                    ConnectedToIP = "??";
                    RemotePort = -1;
                }
                session_bytes_out = 0;
                CoutM2(sockconsoleout) << "TCP Server " << ConnectedToIP<< ":" << RemotePort << " on my port " << portnum << "  fd:"  << sockfd << endl;
                AttemptCount = 0;   // we connected, so restart the counter

           }else{
                // Nobody tried to connect to us.
                pthread_mutex_lock(&txbufflock);
                txcount = 0;
                pthread_mutex_unlock(&txbufflock);
           }
           break;
        case pClient:
           if ((ts > TRYTIME) && (sockfd >=0)) { //timeout changed in 2.3.2.  Sockfd test added in 3.0
                //cout << "Try. " << description << endl;
                connect_time = TimeNow();           // time we last tried to connect.
                time_last_activity = time(NULL);

                // Try to connect to the remote server
                // Congigure the socket to call the remote server
                client_tries++;
                memset( (char *) &serv_addr, NUL, sizeof(serv_addr));  // Initalize the address structure
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(hostport);  // Configure the port number to call on the remote machine.
                serv_addr.sin_addr.s_addr = inet_addr(hostIPaddress.c_str());  // case the address of the remote machine, but convert to C string first.


                newsockfd = -1;  // added in 3.0
                // Try to connect to the remote server
                err = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
                if (err >= 0){
                   newsockfd = sockfd;  // We connected t
                   // Now get my own IP address
                   socklen_t socklen = sizeof(struct sockaddr_storage);
                   memset(&MyAdd,0,sizeof(&MyAdd));
                   if (getsockname(sockfd,(struct sockaddr *)&MyAdd, &socklen) >= 0)
                       myIP4 = inet_ntoa(MyAdd.sin_addr);
                   else
                       myIP4 = "??";
                   memset(&CliAddr,0,sizeof(&CliAddr));
                   if (getpeername(newsockfd,(struct sockaddr *)&CliAddr, &socklen) >= 0){
                        ConnectedToIP = inet_ntoa(CliAddr.sin_addr);
                        RemotePort = ntohs(CliAddr.sin_port);
                    }
                    else{
                        ConnectedToIP = "??";
                        RemotePort = -1;
                    }
                   client_tries = 0;  // we connected
                   session_bytes_out = 0;
                   CoutM1(sockconsoleout) << "TCP Client " << description << " connected to:" << ConnectedToIP<< ":" << RemotePort << " on my port " << portnum;
                   CoutM2(sockconsoleout) << " Client fd:"  << sockfd;
                   CoutM1(sockconsoleout) << endl;
                   MyCLI.Display(&sockconsoleout);
                   AttemptCount = 0;   // we connected, so restart the counter
                }else{
                    // Could not connect to the server
                    CoutM2(sockconsoleout) << description << " Tried to connect to " << hostIPaddress << ":" 
                             << hostport << " on my port:" << portnum << " Err:"<< intToString(errno) << "(" << strerror(errno) << ")" << endl;
                    // added in 3.0
                    //.DisconnectSocket();   // make sure we are disconnected before we retry to connect
                    if (errno == EBADF){
                        // Added in 3.0
                        // Bad file descriptor.  Socket opened and no one was listening.
                        sockfd = -1;
                        newsockfd = -1;  //bad socket
                    }
                    MyCLI.Display(&sockconsoleout);
                }
                break;
           }
    }
    return true;
}



// Force a disconnection of the socket
bool tcpnet::DisconnectSocket(void){

    if (newsockfd >= 0){
        shutdown(newsockfd, SHUT_RDWR);
        usleep(CLOSEDELAY);
        if (close(newsockfd) != 0){
            CoutM2(sockconsoleout) << "Error disconnecting client." << endl;
        };
    }
    if (sockfd >= 0){
        usleep(CLOSEDELAY);
        shutdown(sockfd, SHUT_RDWR);
        if (close(sockfd) != 0){
            CoutM2(sockconsoleout) << "Error disconnecting the socket:" << sockfd << endl;
        };
    }
    sockfd = -1;
    newsockfd = -1;
    connected = false;
    ConnectedToIP = "";                           // The IP address and port of the remote machine we talk to
    RemotePort = -1;
    client_tries = 0;
    session_bytes_out = 0;
    return true;

}

// Force a disconnection to the client
bool tcpnet::DisconnectClient(void){

    switch (protocol){
        case pServer:
            if (newsockfd >= 0){
                shutdown(newsockfd, SHUT_RDWR);
                if (close(newsockfd) != 0){
                    CoutM2(sockconsoleout) << "Error disconnecting client." << endl;
                };
                CoutM2(sockconsoleout) << "Closing fd:" << newsockfd << endl;
            }
            newsockfd = -1;
            break;
        case pClient:
            if (sockfd >= 0){
                shutdown(sockfd, SHUT_RDWR);
                if (close(sockfd) != 0){
                    CoutM2(sockconsoleout) << "Error closing socket." << endl;
                };
            }
            sockfd = -1;
            break;
        case pDGRAMTX:

            break;
        case pDGRAMRX:

            break;
    }// switch

    connected = false;
    ConnectedToIP = "";                           // The IP address and port of the remote machine we talk to
    RemotePort = -1;
    client_tries = 0;
    return true;

}


// Load count bytes into the tx buffer to send out the socket when time comes
bool tcpnet::sendbytes(char* ch, int count){

    int i = 0;

    // cout << count << " bytes to " << myDevIndex << endl;

    pthread_mutex_lock(&txbufflock);  // lock the buffer while we load it
    while ((count > 0) && (txcount < MAXBUFSIZE) && (txcount >= 0)){
       txbuff[txcount] = ch[i];
       i++; count--; txcount++;
    }
    pthread_mutex_unlock(&txbufflock);
    msg_out++;  // count this message
    return true;

}

// Load string into the tx buffer to send out the socket when time comes
bool tcpnet::sendbytes(string s){

    int i = 0;
    int l = s.size();

    if (l == 0)
        return false;

    // cout << l << " bytes to " << endl;

    pthread_mutex_lock(&txbufflock);  // lock the buffer while we load it
    while ((l > 0) && (txcount < MAXBUFSIZE) && (txcount >= 0)){
       txbuff[txcount] = s[i];
       i++; 
       l--;
       txcount++;
    }
    pthread_mutex_unlock(&txbufflock);
    msg_out++;  // count this message
    return true;

}


// Load text string into the tx buffer to send out the socket when time comes
// Do CRLF translation if necessary
bool tcpnet::sendtext(string s){

    int i = 0;
    int l = s.size();

    if (l == 0)
        return false;

    pthread_mutex_lock(&txbufflock);  // lock the buffer while we load it
    while ((l > 0) && (txcount < (MAXBUFSIZE - 2)) && (txcount >= 0)){
       if (NLtoCRLF && (s[i] == NL)) {
           // We are supposed to translate CR into CR LF combo.
           txbuff[txcount++] = CR;
           txbuff[txcount++] = NL;
       }else{
           txbuff[txcount++] = s[i];
       }
       i++; l--;
    }
    pthread_mutex_unlock(&txbufflock);
    msg_out++;  // count this message
    return true;

}


string tcpnet::Protocol(void){

    switch (protocol){
        case pServer:
            return "Server";
        case pClient:
            return "Client";
        case pDGRAMTX:
            return "UDP Sender";
        case pDGRAMRX:
            return "UDP Listener";
    }
    return "";
}

// return the number of hours since last message
double tcpnet::ActivityTime(void){
    double d;
    int i;

    if (!connected)
        return -1;

    if (bytes_in == 0)
        return -1;

    time_t time_here = time(NULL);          // The time we got the last message in from this port

    d = difftime(time_here, time_last_in)/(60 * 60);

    return d;
}

string tcpnet::CheckURLorIP(string hostaddress ){

        string IPaddString = "";
        // hostIPaddress
        struct sockaddr_in sa;
        char str[INET_ADDRSTRLEN];

        if (hostaddress.size() > 5){
            // This is a client socket and we need to resolve the host IP
            if (inet_pton(AF_INET, hostaddress.c_str(), &(sa.sin_addr))== NULL){
                // Not a valid IP address. Maybe a URL?
                // cout << "Looking up " << hostaddress << endl;
                // Following code was added to resolve URLs to IP addresses
                struct addrinfo hints, *res, *p;
                int status;
                char ipstr[INET6_ADDRSTRLEN];

                memset(&hints, 0, sizeof hints);
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                // Do a DNS look-up on the hostaddress URL. (Slow funtion!)
                if ((status = getaddrinfo(hostaddress.c_str(), NULL, &hints, &res)) != 0) {
                    // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                    return "";  // can't do anything with this URL
                }

                for(p = res;p != NULL; p = p->ai_next) {
                    void *addr;
                    char *ipver;

                    // get the pointer to the address itself,
                    // different fields in IPv4 and IPv6:
                    if (p->ai_family == AF_INET) { // IPv4
                        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                        addr = &(ipv4->sin_addr);
                        ipver = "IPv4";
                    } else { // IPv6
                        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                        addr = &(ipv6->sin6_addr);
                        ipver = "IPv6";
                    }
                    // convert the IP to a string and print it:
                    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
                    IPaddString = ToString(ipstr);
                    CoutM2(sockconsoleout) << "Found server " << hostaddress << " " << ipver << " address: " << ipstr << " port: " << hostport << endl;
                }
                freeaddrinfo(res); // free the linked list
            } // end of DNS lookup
            else{
                IPaddString = hostaddress;
            }


            // now lets see if the IP address is OK
            if (inet_pton(AF_INET, IPaddString.c_str(), &(sa.sin_addr))== NULL){
                CoutM2(sockconsoleout) << "Invalid IP or URL:" << hostaddress << endl;
                elog.store(string("Invalid IP or URL:") + hostaddress);
                return "";  // invalid so return nothing
            }
        }// If we are a client and needed to resolve a URL

        return IPaddString;

}

// Find a free port number to used for client sockets
int tcpnet::GetFreePortNum(int basenum, int indx){
    int i;
    int retval = -1;
    bool found = false;
    procinfo MyPortInfo;
    MyPortInfo.ReadMyPorts();  // get the port information
    
    
    for (i = 0; i< 1000; i++){
        retval = basenum + rand() % SERVER_PORT_RES;
        found = false;
        for (MyPortInfo.itr = MyPortInfo.MyPortInfo.begin(); MyPortInfo.itr != MyPortInfo.MyPortInfo.end(); MyPortInfo.itr++){
            if (retval == MyPortInfo.itr->second.MyPort){
                found = true;
                break;
            }
        }
        if (GetSocketIndex(retval) >= 0)
            found = true;
        if (!found) 
            return retval;  // found an unused port
    }
    return (basenum + indx);
}


// Free this socket object up for use by something else.
void tcpnet::freesocket(void) {
     sockfd = -1;
     connected = false;
     myDevDesIndex = -1;        // indicates not bound to any device designator yet.
     myDevType = -1;
     newsockfd = -1;
     portnum = -1;
     sockfd = -1;
     txcount = 0;
     bytes_in = 0;
     bytes_out = 0;
     index = -1;
     interface = "";
     init_tries = 0;
     hostaddress = "";
     hostport = 0;
     connects = 0;
     myIP4 = "";
     ConnectedToIP = "";
     description = "";
     hostIPaddress = "";
     rxRawCount = 0;
     rxRaw[0] = NUL;
     time_last_in = time(NULL);
     time_last_out = time(NULL);
     time_last_activity = time(NULL);
     client_tries = 0;
     session_bytes_out = 0;
     pthread_mutex_t txbufflock;     // Lock for the tx data buffer
     echoinput = false;
     NLtoCRLF = false;               // by default, no translation.
     enable_keepalive = false;       // be deafault, don't send keep alives.
     ForceClientDisconnect = false;
}


// {channel:int} {device type:string}  {interface:string]  {port:int} {protocol:string}
// Use to bind a device that use TCP/IP sockets to a TCP object.
// Return 0 for OK. Negative for fail.
int tcpnet::CreateSocket(string intf, int portno, std::string proto, std::string remoteserver, 
                         int ctimeout, bool keepalive, int devtype){
    int pr = -1;
    in_addr_t ip = 0;
    in_addr_t mask = 0;
    string s;


    // Determine the type of protocol we want to use, and set the designator integer
    if (StringToUpper(proto) == "CLIENT"){
        pr = pClient;
        remoteserver = trim(remoteserver);
        if (remoteserver.size() < 7){
            return -1; // failed. No server IP
        }
    }

    if (StringToUpper(proto) == "SERVER"){
        pr = pServer;
        remoteserver = "";    // no remote server when we are the server.
    }

    if (StringToUpper(proto) == "UDPTX"){
        pr = pDGRAMTX;
        remoteserver = trim(remoteserver);
    }
    if (StringToUpper(proto) == "UDPRX"){
        pr = pDGRAMRX;
        remoteserver = trim(remoteserver);
    }

    if (pr == -1)
        return -2;  // no valid protocol

    // See if thie interface exists
    intf = trim(intf);

    if (intf.size() < 1){
        interface="";        // invalid interface
        return -3;
    }

    myDevDesIndex = 0;      // must not be -1.
    myDevType = devtype;    // the type of device we are talking to
    index = -1;             // tell each object what its index is
    description = "";
    MyParser.DefaultSrcID = DEFAULT_ID; // Get the default port number
    MyParser.DefaultDstID = DEFAULT_ID; // Get the default port number
    clienttimeout = ctimeout;
    enable_keepalive = keepalive;
    protocol = pr;

    // Setup the ports we use
    switch (pr){
        case pServer:
            portnum = portno;  // Use the assigned port ot listen on
            hostport = 0;      // No host port yet. Not till we are connected
            break;
        case pClient:
            portnum = 0;            // Does not matter.  Is assigend randomly when we initialize
            hostport = portno;      // Port number on the remote host
            break;
        case pDGRAMTX:
            portnum = CLIENT_PORT_BASE;   // The default port we will communicate on
            hostport = portno;            // Port number on the remote host
            break;
        case pDGRAMRX:
            portnum = portno;   // The default port we will communicate on
            hostport = 0;      // No host port yet. Not till we get a packet
            break;
        }
    init_tries = 0;       // have not yet tried to initialize it.
    interface = trim(intf);     // the default ethernet interface.
    s = GetIP(interface.c_str(), &ip, &mask);
    //cout << "eth0dfsdf =" << s << endl;

    myIP4 = s;
    hostaddress = remoteserver;
    protocol = pr;

    //tcpsockets[tcpindex].description = OurDevices.getBoundDesignator(tcpindex, tcpsockets[tcpindex].interface);
    // Now go try to initialize this socket
    initialize_socket();

   // cout << "Connecting " << devdes << " to:" << tcpsockets[tcpindex].interface << ":" << portno << endl;
                  // the protocl to use.
    return 0;
    
}

bool tcpnet::isInputPaused(){
    return TimeNow() < timeInputUnpaused;
}

bool tcpnet::isOutputPaused(){
    return TimeNow() < timeOutputUnpaused;
}

/**
 * Pauses input until the specified time. If input is already paused
 * for a greater amount of time than this would cause input to be paused
 * for, has no effect. If input is paused but would end before unpauseTime,
 * extends the pause duration to unpauseTime
 * @param unpauseTime Time to pause input for
 */
void tcpnet::pauseInputUntil(double unpauseTime){
    if(unpauseTime > timeInputUnpaused){
        timeInputUnpaused = unpauseTime;
    }
}

/**
 * Same effect as @see pauseInputUntil(), for output.
 * @param unpauseTime Time to pause input for
 */
void tcpnet::pauseOutputUntil(double unpauseTime){
    if(unpauseTime > timeOutputUnpaused){
        timeOutputUnpaused = unpauseTime;
    }
}

void tcpnet::unpauseInput(){
    timeInputUnpaused = 0;
}

void tcpnet::unpauseOutput(){
    timeOutputUnpaused = 0;
}
