/* 
 * File:   EventTimer.h
 * Author: john
 *
 * Created on June 3, 2011, 3:22 AM
 */

#ifndef EVENTTIMER_H
#define	EVENTTIMER_H

class EventTimer {
public:
    EventTimer();
    EventTimer(const EventTimer& orig);
    virtual ~EventTimer();
    void start( void);
    double timeinterval( void) ;

    double starttime;
    double nowtime;

private:

};

#endif	/* EVENTTIMER_H */

