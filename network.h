/* 
 * File:   network.h
 * Author: John Sonnenberg
 *
 * Created on January 18, 2011, 9:05 PM
 */

#ifndef NETWORK_H
#define	NETWORK_H

// Prototype the functions
int SetNetworkConfig(char *, in_addr_t , in_addr_t ,in_addr_t );
in_addr_t IP2Broadcast(in_addr_t, in_addr_t);
in_addr_t IP2Network(in_addr_t , in_addr_t );
in_addr_t ValidateMask(in_addr_t);
std::string GetIP(const char *, in_addr_t *, in_addr_t *);
static int set_default_gw( int , in_addr_t);
void UpdateIP(const char *, in_addr_t, in_addr_t, in_addr_t);
int GetMacAddress(char *, char *);
int SetMacAddress(char *, char *);


#endif	/* NETWORK_H */

