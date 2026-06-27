/* 
 * File:   microsleeper.cpp
 * Author: john
 * 
 * Created on November 3, 2010, 6:32 AM
 */

#include "microsleeper.h"
#include "unistd.h"

// Create a micro sleeper.  st is the time in uS we'll sleep when not busy.
// m is the number of times we'll be called and not sleep if we are busy
microsleeper::microsleeper(int st, int m) {
    busy = false;
    busyloops = 0;
    maxloops = m;
    MAXL = DEFAULT_MAXL;      // limit number of loops we will doze off during

    if ((m > 0) && (m <= DEFAULT_MAXL))
        MAXL = m;
    else
        MAXL = 4;

    if (st > 0)
        sleeptime = st;
    else
        sleeptime = 100;  // devault to 100uS if invalid time.
    last_time = time(NULL);
    LoopSpeed = 0;
    call_counter = 0;

}

microsleeper::microsleeper(const microsleeper& orig) {
}

microsleeper::~microsleeper() {
}

void microsleeper::DozeOff(bool bz){
     // Now sleep a little if we were not terribly busy

     time_t now_time;

     // compute the rate at which this routine is called
     call_counter++;
     now_time = time(NULL);
     if (last_time != now_time){
         // new second
         last_time = now_time;
         LoopSpeed = call_counter;  // calculate the call rate, in Hz.
         call_counter = 0;
     }

     busy = bz;
     if (busy == true)
         busyloops = MAXL;    // run N loops no sleep if we were busy
     else{
         if (busyloops > 0)
             busyloops--;
     }

     if (busyloops == 0)
        usleep(sleeptime);  // Sleep because we are not busy
     else
        usleep(10);         // Sleep a tiny bit anyway

}

/**
 * Gets the speed the loop is running at. Ensures that if the loop has not run
 * in the last 10 seconds, a 0 is returned.
 * @return Loop speed in Hz. 0 if loop has not run in 10 seconds
 */
int microsleeper::getLoopSpeed(){
    time_t now_time = time(NULL);
    
    if(now_time - last_time > 10){
        // Loop has not run in 10 seconds
        return 0;
    }else{
        return LoopSpeed;
    }
}