/* 
 * File:   Ticker.h
 * Author: john
 *
 * Created on October 28, 2010, 10:13 PM
 */

#ifndef _TICKER_H
#define	_TICKER_H

#include <time.h>
#include "platform/Platform.h"

class Ticker {
public:
    Ticker();
    Ticker(double);
    Ticker(const Ticker& orig);
    virtual ~Ticker();
    double Elasped(void);
    double EpochTime(double);           // return the time into the current epoch
    int SlotNum(double, double);        // return the current slot number
    void ResetEpoch(double);            // reset the epoch counter to value given

    long int InitTime;
    timeval basetime;
    timeval epochstart;


private:

};

#endif	/* _TICKER_H */

