/* 
 * File:   Ticker.cpp
 * Author: john
 * 
 * Created on October 28, 2010, 10:13 PM
 */

#include "time.h"
#include "Ticker.h"
#include "platform/time/PlatformTime.h"

static void CigornGetTimeOfDay(timeval* value)
{
    const std::int64_t milliseconds = cigorn::UnixTimeMilliseconds();

    value->tv_sec =
        static_cast<decltype(value->tv_sec)>(milliseconds / 1000);

    value->tv_usec =
        static_cast<decltype(value->tv_usec)>((milliseconds % 1000) * 1000);
}

int   timeval_subtract (timeval *,timeval *, timeval *);
void  timeval_add(timeval* , double);

double TimeValToDbl(timeval);

Ticker::Ticker() {

    CigornGetTimeOfDay(&basetime);    // read the time that this program started
    CigornGetTimeOfDay(&epochstart);  // initialize the epoch to now untill we know better

}


Ticker::Ticker(const Ticker& orig) {
}

Ticker::~Ticker() {
}

// reset the epoch time to this many seconds as given by tm
void Ticker::ResetEpoch(double tm){

    CigornGetTimeOfDay(&epochstart);     // get the time now.
    timeval_add(&epochstart, tm);        // add the tm value to it we were told to set it to. 

}



double Ticker::Elasped(void){

    timeval nowtime;
    timeval deltat;

    CigornGetTimeOfDay(&nowtime);
    timeval_subtract (&deltat, &nowtime, &basetime);
    
    return TimeValToDbl(deltat);
  
}

// return the number of seconds into the epoch. mod is the epoch duration (TDMATIME)
double Ticker:: EpochTime(double mod){   // return the time into the current epoch

    double t;
    long l;
    long n;

    timeval nowtime;
    timeval deltat;

    CigornGetTimeOfDay(&nowtime);
    timeval_subtract(&deltat, &nowtime, &epochstart);

    t = TimeValToDbl(deltat);  // convert to double
    l = t;                     // get the integer portion
    n = l / mod;               // how many epochs?
    l = n * mod;               // how long were these epochs?
    t = t - l;                 // subtract, leaving the amount of time in this epoch

    return t;

}


// return the current slot number given the epoch time (mod) and the slot width (sw)
int Ticker::SlotNum(double mod, double sw){   // return the time into the current epoch

    double t;
    long l;
    long n;
    int i;

    timeval nowtime;
    timeval deltat;

    CigornGetTimeOfDay(&nowtime);
    timeval_subtract(&deltat, &nowtime, &epochstart);

    t = TimeValToDbl(deltat);  // convert to double
    l = t;                     // get the integer portion
    n = l / mod;               // how many epochs?
    l = n * mod;               // how long were these epochs?
    t = t - l;                 // subtract, leaving the amount of time in this epoch

    t = t / sw;                // how many slots have passed?
    i = t;                     // conver to int
    return i;
}


double TimeValToDbl(timeval tv){
    double d;
    d = tv.tv_sec;
    d = d + (double)tv.tv_usec/1000000;
    return d;
    
}

  // add the tm value to a time value
void timeval_add(timeval *tm, double d){
    int i = d;
    int j;

    tm->tv_sec = tm->tv_sec + i;           // add the seconds

    d = d - i;                           // get the fractional part
    j = (1000000 * d);                   // et the uS portion of the time to add

    j = j + tm->tv_usec;                  // add it
    if (j>= 1000000){
        // overflow in uS portion
        j = j - 1000000;
        tm->tv_sec++;
    }
    tm->tv_usec = j;                      // add the uS portion on

};


/* Subtract the `struct timeval' values X and Y,
        storing the result in RESULT.
        Return 1 if the difference is negative, otherwise 0.  */
int   timeval_subtract (timeval *result,timeval *x, timeval *y) {
       /* Perform the carry for the later subtraction by updating y. */
       if (x->tv_usec < y->tv_usec) {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
       }
       if (x->tv_usec - y->tv_usec > 1000000) {
         int nsec = (x->tv_usec - y->tv_usec) / 1000000;
         y->tv_usec += 1000000 * nsec;
         y->tv_sec -= nsec;
       }

       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec;

       /* Return 1 if result is negative. */
       return x->tv_sec < y->tv_sec;
     }


