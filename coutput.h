/* 
 * File:   coutput.h
 * Author: cigorn
 *
 * Created on November 25, 2011, 9:54 AM
 */

#ifndef COUTPUT_H
#define	COUTPUT_H

#include <iostream>		// 8-02 Added since std::ostream needed it

enum LogPriority {
    kLogEmerg,   // system is unusable
    kLogAlert,   // action must be taken immediately
    kLogCrit,    // critical conditions
    kLogErr,     // error conditions
    kLogWarning, // warning conditions
    kLogNotice,  // normal, but significant, condition
    kLogInfo,    // informational message
    kLogDebug    // debug-level message
};

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);


class coutput : public std::basic_streambuf<char, std::char_traits<char> > {
public:
    coutput();
    coutput(const coutput& orig);
    virtual ~coutput();


private:
    friend std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
    std::string buffer_;
    int facility_;
    int priority_;
    char ident_[50];


};

#endif	/* COUTPUT_H */

