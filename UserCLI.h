/* 
 * File:   cli.h
 * Author: ss
 *
 * Created on September 15, 2011, 6:54 PM
 */

#ifndef UserCLI_H
#define	UserCLI_H

#include "health.h"
#include "ascii.h"
#include <stdio.h>
#include <iostream>
#include "functions.h"
#include "DeviceList.h"
#include "htmlformatter.h"

#define MAX_CLI_OUT_SIZE   200000
#define MAX_CLI_TRANSFER   1000

class UserCLI {
public:
    UserCLI();
    UserCLI(const UserCLI& orig);
    virtual ~UserCLI();

    bool LogDirectory;
    tcpnet MySocket;  // the socket object to communicate to the smtp mail server with

    void  Configure(int);
    bool  InputOutput(void);
    bool  GetNewCommand(string *);
    bool  OutputText(string);
    bool DisplayString(string*);
    bool Display(stringstream* );

private:

    stringstream ssOutBuffer;

};

#endif	/* UserCLI_H */

