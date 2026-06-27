/* 
 * File:   health.h
 * Author: john
 *
 * Created on August 31, 2010, 8:43 PM
 */

#ifndef _HEALTH_H
#define	_HEALTH_H

#include "TCPsocket.h"


class health {
public:
    health();
    health(const health& orig);
    virtual ~health();
    bool LogDirectory;
    tcpnet MySocket;  // the socket object to communicate to the smtp mail server with


void  ConfigureMonitor(int);
bool  ProcessMonitor(void);

private:

};

#endif	/* _HEALTH_H */

