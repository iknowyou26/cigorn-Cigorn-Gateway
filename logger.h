/* 
 * File:   logger.h
 * Author: john
 *
 * Created on December 27, 2010, 8:41 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <string>
#include <vector>
#include <fstream>


using namespace std;

#define MAX_ELOG_Q  100
 
class logger {
public:
    logger(string, string);
    logger(const logger& orig);
    virtual ~logger();

    void store(string);
    bool eraseLog(void);
    bool closeLog(void);
    void storeTrimmed(string , int );
    string makefilename(void);
    string FullFileName(void);
    ofstream logStream;

    vector <string> ErrorMessages;


private:
    std::string FileName;
    std::string FileExtension;
    std::string CurrentFullName;


private:

};

#endif	/* LOGGER_H */

