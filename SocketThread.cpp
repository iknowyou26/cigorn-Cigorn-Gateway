#include <stdio.h>
#include <stdlib.h>
#include "platform/Platform.h"

#include <string>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <fcntl.h>

#undef __USE_MISC
#include <net/if.h>
#endif

#include "dataparser.h"
#include "Cigorn.h"     // Our application-specific constants and headers
#include "dataparser.h"
#include "TCPsocket.h"
#include "CommThread.h"
#include "microsleeper.h"
#include "SocketThread.h"
#include "network.h"
#include "functions.h"

using namespace std;
bool SocketThreadRunning = true;     // Set false at end of thread.
IPaddList ipaddresses;               // a list of all of the ethernet interfaces and their IP addresses.
bool RunSocketThread = true;
bool deconstructSockets = false;

microsleeper  MySleeper(SOCKSLEEP, 2);   // does microsecond sleeping. Stay awake 2 loops if we are busy, or sleep X uS if not.

void *threadTCPserver(  void *ptr )
  {
    int sockindex;
    int newport = -1;
    int defaultport = -1;
    bool busy = false;                // set true anytime a routine in the loop does anything
    int outsocketindex = -1;
    string s;
    stringstream ss;

    BinaryEntry TopEntry;

    time_t now_time = time(NULL);
    time_t last_time = time(NULL);

    ss << saySSTHREADSTART  << endl;
    MyCLI.Display(&ss);
    ss.str("");

     struct sockaddr_in ;
     struct addrinfo;
     int i;

     // Wait untill the communications thread is all initialized and running before
     // starting the socket communications.
     while (CommThreadInitialized = false){
         sleep(0.05);
     }

     pthread_mutex_lock(&addlistlock);       // the addlist structure lock so we can update it in another thread.
     GetMyIPaddressList(ipaddresses);
     pthread_mutex_unlock(&addlistlock);     // the addlist structure lock so we can update it in another thread.

     ss << "Ethernet interface count: " << ipaddresses.size() << endl;
     MyCLI.Display(&ss);
     ss.str("");

     pthread_mutex_lock(&addlistlock);     // the addlist structure lock so we can update it in another thread.
     IPaddList::iterator it;
     Me.IPadd = "";

     for (it=ipaddresses.begin(); it!= ipaddresses.end(); it++){
        if (Me.IPadd.size() > 0)
             Me.IPadd = Me.IPadd + ",";
        Me.IPadd =  Me.IPadd + it->second.interface + " " + it->second.ipaddress;
        ss << "Interface: " << FixedRight(it->second.interface, 5) << "   " <<it->second.ipaddress << endl;
        MyCLI.OutputText(ss.str());
        ss.str("");
     }
     pthread_mutex_unlock(&addlistlock);     // the addlist structure lock so we can update it in another thread.

     while (!ShutDownApplication){

         while (RunSocketThread){
             if (deconstructSockets) {
                 // May run multiple times through the socketloop
                for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++) {
                    tcpsockets[sockindex].DisconnectSocket(); // Disconnect socket
                    tcpsockets[sockindex].clear();            // Reset all values
                }
                //CoutM2(ss) << "All SOCKETS cleared. " << endl;
                MyCLI.OutputText(ss.str());
                deconstructSockets = false;
             }

             busy = false;
             SocketThreadRunning = true;
             // Run sockets that are bound to some I/O channel
             for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
                   if (tcpsockets[sockindex].myDevDesIndex >= 0){
                      // This socket is bound to an interface, so go execute the code
                      if (tcpsockets[sockindex].myIP4.size() < 1)
                         tcpsockets[sockindex].myIP4 = GetInterfaceIPaddress(tcpsockets[sockindex].interface);

                      if (Me.IsActive == false){
                          // We are off-line
                          tcpsockets[sockindex].DisconnectSocket();  // do not connect to anyone
                      }else{
                          // Execute the socket object if we are an active gateway
                          if (tcpsockets[sockindex].tcp_socket() > 0)
                              busy = true;
                      }

                      if (tcpsockets[sockindex].sockconsoleout.str().size() > 0){
                          // There is a string message to be sent out the debug interface
                          // cout << " Debugger--" << tcpsockets[sockindex].skout.str();
                          MyCLI.OutputText(tcpsockets[sockindex].sockconsoleout.str());
                      }
                   }
                   tcpsockets[sockindex].sockconsoleout.str("");   // avoid memory leak. Always flush it
             }

             // See if there is data from the outbound data queue to send out a socket.
             if (qETHout.size() > 0){
                 // cout << "Eth data" << endl;
                 pthread_mutex_lock(&qlock);
                 TopEntry = qETHout.front();           // get the q object with the data we need to send out
                 try
                 {
                   qETHout.pop();                      // take one object off the top. Throws exception some times.  Timein needs range limit?
                 }
                 catch( std::bad_alloc)
                 {
                   elog.store("Error 634. qETHout error.");
                   TopEntry.SrcDevDesIndex = -1;   // bad entry
                   TopEntry.DstDevDesIndex = -1;
                   TopEntry.bcount = 0;
                 }

                 pthread_mutex_unlock(&qlock);   // pthread_mutex_unlock(&addlistlock)
                 busy = true;
                 // See if there is WNAT port translation.  -1 if no WNAT so we use the first DevDes we find to send this out.
                 WNAT.PortTranslate(TopEntry, newport, defaultport);

                 // The socket index we'll use to send this message out
                 outsocketindex = -1;
                 //CoutM2(ss) << "Moving message to TCP queue " << endl;
                 if ((TopEntry.bcount < MAXBUFSIZE) && (TopEntry.bcount > 0) && (TopEntry.DstDevDesIndex >= 0)){

                     // Try to find the socket to use for this message.  We need to loop throug all
                     // sockets because many sockets can be assigned to one device designator
                     for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){

                          // Is this socket the one to use as this destination?
                          if ((TopEntry.DstDevDesIndex == tcpsockets[sockindex].myDevDesIndex)){
                              // This is the designator we need to send out the data to. Is this the right PORT?
                              if (newport == -1){
                                  // No WNAT.  Just send it to the port defined in the route table.
                                  outsocketindex = sockindex;  // no WNAT.  This is the socket we should use
                                  break;
                              }else{
                                  // WNAT is used. Is this socket the right port to use?
                                  if (newport == tcpsockets[sockindex].portnum){
                                        // This socket should be used to send out this message
                                        // cout << "WNAT. Send data to " << tcpsockets[sockindex].myDevDesIndex << " " << newport << "/" << defaultport<< endl;
                                        if (tcpsockets[sockindex].connected){
                                            // This socket is connected, so OK to send the data to it
                                            outsocketindex = sockindex;
                                        }else{
                                            // Failed to deliver to primary port.  Send it to the default port
                                            outsocketindex = GetSocketIndex(defaultport);
                                        }
                                        break;  // jump out of the for..
                                  }
                              }
                          }
                     }// for socket...


                     if ((outsocketindex >= 0) && (outsocketindex < MAXSOCKETS)){
                        // we found the socket to send this message out
                        pthread_mutex_lock(&tcpsockets[outsocketindex].qlock);        // pthread_mutex_unlock(&addlistlock)
                        try{
                            if (tcpsockets[outsocketindex].MsgQout.size() >= maxQcount){
                                CoutM2(ss) << "Q overflow in " << tcpsockets[outsocketindex].description << endl;
                                try {
                                    tcpsockets[outsocketindex].MsgQout.pop(); // remove the message. It is too old
                                 }
                                    catch (exception& e) {
                                 }
                            }
                            if (tcpsockets[outsocketindex].MsgQout.size() < maxQcount ){
                                tcpsockets[outsocketindex].MsgQout.push(TopEntry);            // put the message into the outbound q for this socket
                                // cout << "Sent data to port:" << tcpsockets[outsocketindex].portnum << endl;
                            }
                        }
                        catch (bad_alloc& ba)
                              {
                                ss << "MsgQout.push bad_alloc caught: " << ba.what() << " Q size="
                                   << tcpsockets[outsocketindex].MsgQout.size() << endl;
                                elog.store(ss.str());
                                if (tcpsockets[outsocketindex].MsgQout.size() > 0)
                                    tcpsockets[outsocketindex].MsgQout.pop();  // we better shrink the queue.  It was out of memory
                        }
                        pthread_mutex_unlock(&tcpsockets[outsocketindex].qlock);      // pthread_mutex_unlock(&addlistlock)
                     }else
                        FailedSockOut++;    // count the homeless messages
                  }
             }//if (qETHout.size() > 0){

             now_time = time(NULL);
             // Run these things once per second...
             if (last_time != now_time) {
                 last_time = time(NULL);
                 // Handle the STATUS display connected to the RS232 serial port
                 busy = true;
             }

             // If the app is shutting down, then we should shut down
             if (ShutDownApplication)
                RunSocketThread = false;   // if app is shutting down, stop this thread.


             MySleeper.DozeOff(busy);

        }// while socket thread should run...

        elog.store("Socket thread closing all sockets.");
        cout <<  "Stopping all Sockets." << endl;
        // Close sockets that are bound to some I/O channel
        for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
           if (tcpsockets[sockindex].myDevDesIndex >= 0){
              // This socket is bound to an interface, so clost the socket and free the fd
              if (tcpsockets[sockindex].sockfd != -1)
                  tcpsockets[sockindex].DisconnectSocket();  // disconnect if it is used.
              else
                  tcpsockets[sockindex].freesocket();
               while (tcpsockets[sockindex].MsgQout.size() > 0)
                  tcpsockets[sockindex].MsgQout.pop();  // clear the queue out
           }
        }
        SocketThreadRunning = false;  // we are not running.  ALl sockets dead.

        i=0;
        // wait for a while if someone wants us to stop communications.
        if ((StopCommunications) && (i<10) ){
            // Someone told us to stop for a while and then we'll restart
            sleep(1);  // wait here for a bit
            i++;
            CommunicationsRunning = false;
        }


    } // while application should run

    elog.store("Socket thread shutting down.");
    ss << "Shutting down TCP thread." << endl;
    MyCLI.OutputText(ss.str());
    ss.str("");

     // Close sockets that are bound to some I/O channel
     for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
       if (tcpsockets[sockindex].myDevDesIndex >= 0){
          // This socket is bound to an interface, so clost the socket and free the fd
          tcpsockets[sockindex].DisconnectSocket();
       }
     }
     SocketThreadRunning = false;
     ss << "Shut down TCP thread." << endl;
     MyCLI.OutputText(ss.str());
     ss.str("");

     //     exit(1);   // done with the thread.  Stop now.

}

int GetSocketCount(){
    
}

// Find the socket index of the socket assigned to device TCP port p
int GetSocketIndex(int p){

    int sockindex = -1;
    if (p<0)
        return -1;

    for (sockindex = 0; sockindex < MAXSOCKETS; sockindex++){
        if (tcpsockets[sockindex].portnum == p){
            return sockindex;
        }
    }
    return -1;

}

string AddressListString(void){
    IPaddList::iterator it;
    string s = "";

    for (it=ipaddresses.begin(); it!= ipaddresses.end(); it++){
        if (s.size() > 0)
             s = s + ",";
        s = s + it->second.ipaddress;
     }

    return s;
}



bool InterfaceExists(string s){
    IPaddList::iterator it;

    for (it=ipaddresses.begin(); it!= ipaddresses.end(); it++){
        // cout << s << "> > >" << it->second.interface << endl;
        if (s == it->second.interface) {
            return true;
        }
     }

    return false;
}


// Look up the IP address for this interface in the table we previously loaded.
string GetInterfaceIPaddress(string intf){
    int i;

    if (ipaddresses.size() <= 0)
        return "";

    for (i=0; i< ipaddresses.size(); i++){
        if (intf == ipaddresses[i].interface){
            return ipaddresses[i].ipaddress;
        }
    }
    return "";

}

// Load a list with the IP addresses and interface names on this maching
// Bad file descriptor leak. Only call this at boot-up

void GetMyIPaddressList(IPaddList& al){

   static struct ifreq myEthIF[MAX_ETH];
   struct ifconf ifconfig;
   int  nifaces, i=0;
   struct if_nameindex *head;
   struct if_nameindex *nm;
   vector<int> AddressToDelete;
   in_addr_t ip=0;
   in_addr_t mask=0;

   al.clear();                  // initialize/clear the MAP

   nm = if_nameindex();  // function shall return an array of if_nameindex structures

   if (nm != NULL){
       // Loop through all the interfaces, and get their names
       i = 0;

          while (nm[i].if_index != 0 || nm[i].if_name != 0)
            {
              al[i].interface = nm[i].if_name;
              al[i].ipaddress = GetIP(al[i].interface.c_str(), &ip, &mask);
              i++;
            }

       
       if_freenameindex(nm);     // free the memory
       nm = NULL;                /* prevent use after free  */
   }

   return;
   
}

int get_iface_list(struct ifconf *ifconf)
{
   int sock, rval;

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 0){
     perror("socket");
     return (-1);
   }

   if((rval = ioctl(sock, SIOCGIFCONF , (char*) ifconf  )) < 0 )
     perror("ioctl(SIOGIFCONF)");

   close(sock);

   return rval;
}

/**
 * getIPv4()
 *
 * This function takes a network identifier such as "eth0" or "eth0:0" and
 * a pointer to a buffer of at least 16 bytes and then stores the IP of that
 * device gets stored in that buffer.
 * it return 0 on success or -1 on failure.
 */
int getIPv4(const char * dev, char * ipv4) {
    struct ifreq ifc;
    int res;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0)
        return -1;
    strcpy(ifc.ifr_name, dev);
    res = ioctl(sockfd, SIOCGIFADDR, &ifc);   // get interface address.
    close(sockfd);
    if(res < 0)
        return -1;
    strcpy(ipv4, inet_ntoa(((struct sockaddr_in*)&ifc.ifr_addr)->sin_addr));
    return 0;
}

/**
 * getIPv4()
 *
 * This function takes a network identifier such as "eth0" or "eth0:0" and
 * a pointer to a buffer of at least 16 bytes and then stores the IP of that
 * device gets stored in that buffer.
 * it return 0 on success or -1 on failure.
 */
int getIPv4(string intf) {
    struct ifreq ifc;
    char dev[32];
    int res;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    to_cstring(dev, intf, 32);

    if(sockfd < 0)
        return -1;
    strcpy(ifc.ifr_name, dev);
    res = ioctl(sockfd, SIOCGIFADDR, &ifc);   // get interface address.
    close(sockfd);
    if(res < 0)
        return -1;

    return res;
}





bool ResetSocket(int ResetPort){

    // Global Statistics
    string s;
    bool OnlyConnected = true;
    bool retval = false;
    int i;
    stringstream ssout;

    for (i=0; i < MAXSOCKETS; i++){
       if (tcpsockets[i].portnum == ResetPort) {
          tcpsockets[i].ForceClientDisconnect = true;
          retval = true;
       }
    }
    return retval;

};

// Reset all sockets associated with this device sedignator
bool ResetSocket(string ResetDevDes){
    int i;
    bool retval = false;

    ResetDevDes = StringToUpper(ResetDevDes);
    for (i=0; i < MAXSOCKETS; i++){
       if (WildCardMatch(ResetDevDes, StringToUpper(tcpsockets[i].description))){
          tcpsockets[i].ForceClientDisconnect = true;
          retval = true;
       }
    }
    return retval;
};

/**
 * Gets the current measured TCP loop speed, in Hz
 * @return TCP loop's speed, in Hz
 */
int getTCPLoopSpeed(){
    return MySleeper.getLoopSpeed();
}