/* 
 * File:   rfport.h
 * Author: john
 *
 * Created on August 26, 2010, 10:55 PM
 */

#ifndef _RFPORT_H
#define	_RFPORT_H



class rfport {
public:
    rfport();
    rfport(int);
    rfport(const rfport& orig);
    virtual ~rfport();
    void SendBeacon(void);

    bool connected;        // true once we are connected
    long msgIn;            // the number of user messages in
    long msgOut;           // the number of user messages we sent out
    long wncout;           // number of WNC messages sent out
    long wncin;            // number of WNC messages in from the radio connected to us
    int  devtype;          // The type of device connected to this RF port. (dXXXX defined above)
    int  DevDesIndx;       // Device Designator Index into OurDevices
private:

};

#endif	/* _RFPORT_H */

