/* 
 * File:   procinfo.h
 * Author: john
 * Read the process info from the linux proc file to get machine information
 * Created on January 10, 2011, 8:39 PM
 */

#ifndef PROCINFO_H
#define	PROCINFO_H

#include <map>
#include <string>

#define MAX_PROC_LINES 1000

using namespace std;

struct PortConn{
    string MyIP4;
    int MyPort;
    string RemAddress;
    int RemPort;
};

struct Fields{
    string Field1;
    string Field2;
    string Field3;
    string Field4;
};

class procinfo {
public:
    procinfo();
    procinfo(const procinfo& orig);
    virtual ~procinfo();
    void ReadProcFile(string);
    void ReadMyPorts(void);
    string GetKeepAliveSettings(void);

    int count;
    map <int, PortConn> MyPortInfo;
    map <int, PortConn> ::iterator itr;

    map <int, Fields> MyFields;
    map <int, Fields> ::iterator mitr;
    
private:

};

#endif	/* PROCINFO_H */

