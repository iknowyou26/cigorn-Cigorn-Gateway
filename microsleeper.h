/* 
 * File:   microsleeper.h
 * Author: john
 *
 * Created on November 3, 2010, 6:32 AM
 */

#ifndef _MICROSLEEPER_H
#define	_MICROSLEEPER_H

#define  DEFAULT_MAXL  10
#include "time.h"

class microsleeper {
public:
    microsleeper(int, int);   // the timne in uS and the number of loops to stay awake when busy
    microsleeper(const microsleeper& orig);
    virtual ~microsleeper();
    void DozeOff(bool bz);
    int getLoopSpeed();

    bool busy;
    int MAXL;       // the max value for maxloops
    int sleeptime;
    time_t last_time;
    int call_counter;
    
private:
    int LoopSpeed;
    int busyloops;
    int maxloops;
};

#endif	/* _MICROSLEEPER_H */

