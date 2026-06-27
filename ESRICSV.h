/* 
 * File:   ESRICSV.h
 * Author: adam
 *
 */

#ifndef _ESRICSV_H
#define	_ESRICSV_H

using namespace std;

class ESRICSV {
public:
    static int BuildESRICSV(char* data, int bcout, int src, int dst, int frametype);
};

#endif	/* _ESRICSV_H */

