/* 
 * File:   EventTimer.cpp
 * Author: john
 * 
 * Created on June 3, 2011, 3:22 AM
 */

#include "EventTimer.h"

#include <string>
#include <sstream>
#include "functions.h"
#include "ascii.h"
#include "stdio.h"

EventTimer::EventTimer() {
    // Return the number of seconds with 4 digits of precision.
    struct timeval now;

    gettimeofday(&now, NULL);
    starttime = now.tv_usec;             // number of uSeconds
    starttime = starttime / 1000000 + now.tv_sec;

    gettimeofday(&now, NULL);
    nowtime = now.tv_usec;             // number of uSeconds
    nowtime = nowtime / 1000000 + now.tv_sec;
}


EventTimer::EventTimer(const EventTimer& orig) {
}

EventTimer::~EventTimer() {
}

void EventTimer::start( void)  {
    // Return the number of seconds with 4 digits of precision.
    struct timeval now;

    gettimeofday(&now, NULL);
    starttime = now.tv_usec;             // number of uSeconds
    starttime = starttime / 1000000 + now.tv_sec;

    gettimeofday(&now, NULL);
    nowtime = now.tv_usec;             // number of uSeconds
    nowtime = nowtime / 1000000 + now.tv_sec;

}
double EventTimer::timeinterval( void)  {
    // Return the number of seconds with 4 digits of precision.
    struct timeval now;

    gettimeofday(&now, NULL);
    nowtime = now.tv_usec;             // number of uSeconds
    nowtime = nowtime / 1000000 + now.tv_sec;

    return (nowtime - starttime);

}




