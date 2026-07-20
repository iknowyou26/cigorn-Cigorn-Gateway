#include "platform/PlatformConstants.h"
/* 
 * File:   logger.cpp
 * Author: john
 * 
 * Created on December 27, 2010, 8:41 PM
 */

#include "logger.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include "functions.h"
#include "Cigorn.h"

using namespace std;

logger::logger(std::string logfile, std::string logext) {
    char cname[500];
    string FullFile;

    
    FileName = logfile;       // remember the base file name and extension
    FileExtension = logext;

    FullFile = makefilename();  // build up the current file name with the date embedded in it.

    CurrentFullName = FullFile;
    to_cstring(cname, FullFile, 500);



    if (logStream.good()){
        logStream.close();
    }

    logStream.open(cname, ios::app);

    if (logStream.good() == false){
        // Can't open the file for some reason.
        cout << "Error: Can't open error log file " << logfile << endl;
    }

}

logger::logger(const logger& orig) {
}

logger::~logger() {
}

string logger::FullFileName() {

    return CurrentFullName;
}


// Log the message to the disk and put it in the queue
void logger::store(string msg) {
    string s;
    char cname[500];

    // See if it is time to start a new log file
    s = makefilename();
    if (CurrentFullName != s){
        //  Time to make a new file
        closeLog();
        CurrentFullName = s;
        to_cstring(cname, CurrentFullName, 500);
        logStream.open(cname, ios::app);

        if (logStream.good() == false){
            // Can't open the file for some reason.
            cout << "Error: Can't open error log file " << CurrentFullName << endl;
        }

    }

    if (logStream.good()){
        s = LocalDate() + " " + LocalTime() + " " + msg;   // Insert the time and date into the log
        logStream << s << endl;
    }

    while (ErrorMessages.size() > MAX_ELOG_Q){
        ErrorMessages.pop_back();  // shrink the q
    }

    // save the error message in a q
    ErrorMessages.insert(ErrorMessages.begin(), s);

}

string logger::makefilename(void){
    string fullfilename;
    string Week;

    Week =  LocalDate(YYYY_WEEK);
    
    fullfilename = LOGDIRNAME + string("/") + trim(FileName) + Week + "." + FileExtension;
    return fullfilename;

    }

// Trim spaces, and tabs from beginning. Remove CR and LF off also. Make l bytes long max
void logger::storeTrimmed(string s, int l)
{
	int i=0;
        bool done = false;

        i = s.size() - 1;
        done = false;
	while((i>=0) && (done == false)){
            if ((s[i]==' ') || (s[i]=='\t') || (s[i]== CR) || (s[i]== NL) || (s[i]== NUL))
                s.erase(i,1);
            else
                done = true;  // no bad chars
            i = s.size() - 1;
        }
        store(s);

}

// Erase the error log, and start a new one.
bool logger::eraseLog(void){
    char cname[255];

    if (logStream.good())
        logStream.close();  // close the stream

    to_cstring(cname, CurrentFullName, 255);

    cout << "removing" << CurrentFullName << endl;

    if(remove(cname) == -1 ){
        cout << "Failed to remove error log " << cname << endl;
       return false;  // failed to delete file
    }
    else{
       logger(FileName, FileExtension);  // make a new error log file
       return true;
    }

    return false;

}

// Erase the error log, and start a new one.
bool logger::closeLog(void){
    char cname[255];

    if (logStream.is_open())
        logStream.close();  // close the stream

    return true;

}





