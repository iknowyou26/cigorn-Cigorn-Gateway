
// Various network related subroutines
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <arpa/inet.h>    // Utilities for networking

#include "network.h"

using namespace std;

static const char *l_pszConfigFile = "/etc/sysconfig/network/ifcfg-";

//==============================================================================
// function: SetNetworkConfig
// description: updates the network configuration file
//==============================================================================
int SetNetworkConfig(char *ifname, in_addr_t ip, in_addr_t mask,in_addr_t  gw){
char szTemp[256];
in_addr_t bc,nw;

mask = ValidateMask(mask);
nw = IP2Network(ip,mask);
bc = IP2Broadcast(ip,mask);

sprintf(szTemp,"%s%s",l_pszConfigFile,ifname);
printf("open -> %s\n",szTemp);
FILE *file = fopen(szTemp,"wr");
if ( file == NULL ){
    perror("SetNetworkConfig");
    return -1;
}

fprintf(file,"IPADDR=\"%s\"\n",inet_ntoa(inet_makeaddr(ip,0)));
fprintf(file,"NETMASK=\"%s\"\n",inet_ntoa(inet_makeaddr(mask,0)));
fprintf(file,"NETWORK=\"%s\"\n",inet_ntoa(inet_makeaddr(nw,0)));
fprintf(file,"BROADCAST=\"%s\"\n",inet_ntoa(inet_makeaddr(bc,0)));
if ( gw != 0 )
    fprintf(file,"GATEWAY=\"%s\"\n",inet_ntoa(inet_makeaddr(gw,0)));
fclose(file);
system("/etc/init.d/network restart");

}



//==============================================================================
// function: IP2Broadcast
// description: Takes a subnet mask and IP returns a subnet broadcast address
//==============================================================================
in_addr_t IP2Broadcast(in_addr_t IP, in_addr_t Mask)
{
in_addr_t bcIP;
in_addr_t bit;
in_addr_t notbit;
int i;

bit = 0x80000000;
notbit = 0xFFFFFFFF;
bcIP = 0;
for ( i=0; i<31; i++)
{
if ((bit & Mask) == 0)
break;
bcIP |= (IP & bit);
notbit &= ~bit;
bit = bit >> 1;
}
bcIP != notbit;
//printf("bcIP = %s\n",inet_ntoa(inet_makeaddr(bcIP,0)));
return bcIP;
}

//==============================================================================
// function: IP2Network
// description: Takes a subnet mask and IP returns a subnet address
//==============================================================================
in_addr_t IP2Network(in_addr_t IP, in_addr_t Mask)
{
in_addr_t netIP;
in_addr_t bit;
int i;

bit = 0x80000000;
netIP = 0;
for ( i=0; i<31; i++)
{
if ((bit & Mask) == 0)
break;
netIP |= (IP & bit);
bit = bit >> 1;
}
// printf("Network = %s\n",inet_ntoa(inet_makeaddr(netIP,0)));
return netIP;
}

//==============================================================================
// function: ValidateMask
// description: Takes a subnet mask and returns a valid version
//==============================================================================
in_addr_t ValidateMask(in_addr_t Mask)
{
in_addr_t newMask;
in_addr_t bit;
int i;

bit = 0x80000000;
newMask = 0;
for ( i=0; i<31; i++)
{
if ((bit & Mask) == 0)
break;
newMask |= bit;
bit = bit >> 1;
}
// printf("Mask = %s\n",inet_ntoa(inet_makeaddr(Mask,0)));
return Mask;
}


//==============================================================================
// function: GetLocalIP
// description: retrieve current network ip and mask
//==============================================================================
string GetIP(const char *ifname, in_addr_t *ip, in_addr_t *mask)
{
    struct ifreq ifr;
    string ipaddstr="";
    in_addr i;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock <0)
        return "";

    // Get the interface IP address
    strcpy( ifr.ifr_name, ifname );
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl( sock, SIOCGIFADDR, &ifr ) < 0){
        // Failed to bind error.  
        return "";
    }else{
        *ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        shutdown(sock, SHUT_RDWR);
        i.s_addr = *ip;
        return inet_ntoa(i);
    }

}

//==============================================================================
// function: set_default_gw
// description: sets routing table's default gateway to the sock peer addr
//==============================================================================
static int set_default_gw( int sockfd, in_addr_t gip )
{
struct sockaddr_in *dst, *gw, *mask;
struct rtentry route;

memset(&route,0,sizeof(struct rtentry));

dst = (struct sockaddr_in *)(&(route.rt_dst));
gw = (struct sockaddr_in *)(&(route.rt_gateway));
mask = (struct sockaddr_in *)(&(route.rt_genmask));

/* Make sure we're talking about IP here */
dst->sin_family = AF_INET;
gw->sin_family = AF_INET;
mask->sin_family = AF_INET;

/* Set up the data for removing the default route */
dst->sin_addr.s_addr = 0;
gw->sin_addr.s_addr = 0;
mask->sin_addr.s_addr = 0;
route.rt_flags = RTF_UP | RTF_GATEWAY;

/* Remove the default route */
ioctl(sockfd,SIOCDELRT,&route);

/* Set up the data for adding the default route */
dst->sin_addr.s_addr = 0;
gw->sin_addr.s_addr = gip;
mask->sin_addr.s_addr = 0;
route.rt_metric = 1;
route.rt_flags = RTF_UP | RTF_GATEWAY;

/* Remove this route if it already exists */
ioctl(sockfd,SIOCDELRT,&route);

/* Add the default route */
if( ioctl(sockfd,SIOCADDRT,&route) == -1 ) {
   cout << "Adding default route: %d" << endl;
   return -1;
}

//fprintf( stdout,"Added default route successfully." );
return 0;
}

//==============================================================================
// function: UpdateIP
// description: updates the networks parameters
//==============================================================================
void UpdateIP(const char *ifname, in_addr_t ip, in_addr_t mask, in_addr_t gip){
struct ifreq ifr;
char *myip;
int sock = socket(AF_INET,SOCK_DGRAM,0);
in_addr_t IP;
struct rtentry rt;
struct sockaddr_in sa;
memset(&sa, 0, sizeof(sa));
memset(&rt, 0, sizeof(rt));

// Get the interface IP address
strcpy( ifr.ifr_name, ifname );
ifr.ifr_addr.sa_family = AF_INET;

if (ioctl( sock, SIOCGIFADDR, &ifr ) < 0)
perror( "SIOCGIFADDR 2" );
myip = inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr );

IP = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = ip;
if (ioctl( sock, SIOCSIFADDR, &ifr ) < 0)
perror( "SIOCSIFADDR" );

set_default_gw(sock, gip);
shutdown(sock,SHUT_RDWR);
}

//==============================================================================
// Function: GetMacAddress
// Retrieve current network MAC address
//==============================================================================
int GetMacAddress(char *ifname, char *addr){
    struct ifreq ifr;
    int sock = socket(AF_INET,SOCK_DGRAM,0);

    // Get the interface IP address
    strcpy( ifr.ifr_name, ifname );
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl( sock, SIOCGIFHWADDR, &ifr ) < 0){
        perror( "SIOCGHWADDR 1" );
        return -1;
    }

    memcpy(addr,ifr.ifr_hwaddr.sa_data,6);
    //uint8_t *hwaddr = (uint8_t*)addr;
    //printf("The hardware address (SIOCGIFHWADDR) of %s is type %d "
    // "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x.\n", ifname,
    // ifr.ifr_hwaddr.sa_family, hwaddr[0], hwaddr[1],
    // hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    shutdown(sock,SHUT_RDWR);
    return 0;
}

//==============================================================================
// function: SetMacAddress
// description: Set network address
//==============================================================================
int SetMacAddress(char *ifname, char *addr)
{
struct ifreq ifr;
int sock = socket(AF_INET,SOCK_DGRAM,0);

// Set the interface IP address
strcpy( ifr.ifr_name, ifname );
ifr.ifr_addr.sa_family = AF_UNIX;
memcpy(ifr.ifr_hwaddr.sa_data,addr,6);

if (ioctl( sock, SIOCSIFHWADDR, &ifr ) < 0)
{
perror( "SIOCSIFHWADDR" );
return -1;
}
shutdown(sock,SHUT_RDWR);
return 0;
}






