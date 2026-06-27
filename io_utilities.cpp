/* 
 * File:   io_utilities.cpp
 * Created on July 27, 2010, 9:26 PM
 * Collection of utilites related to the input and output of information to the
 * user interface.
 */

using namespace std;

#include <string>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "Cigorn.h"     

// Return 0 if not a directory, 1 if it is a directory
int isadir( const char* name )
{
  struct stat s;
  lstat( name, &s );
  return ( s.st_mode & S_IFMT ) == S_IFDIR;
}

// ************************************************************************
// Return a string with the application name are revision level formatted.
// ************************************************************************
std::string GetVersionNum(void) {
    std::string s;
    //s = APP_TITLE;

   s = "Version ";
   s += intToString(REV_MAJOR);
   s += ".";
   s += intToString(REV_MINOR);
   s += ".";
   s += intToString(REV_BUILD);

    return s;
}


// ************************************************************************
// Return a string with the application name are revision level formatted.
// ************************************************************************
std::string GetVersionText(void) {
    std::string s;
    //s = APP_TITLE;

   s = APP_TITLE;
   s += " Version ";
   s += intToString(REV_MAJOR);
   s += ".";
   s += intToString(REV_MINOR);
   s += ".";
   s += intToString(REV_BUILD);

    return s;
}

// Set the console output stream
void ConfigureConsole(string fname){
   char cname[255];

   to_cstring(cname, fname, 255);
   old_cout = std::cout.rdbuf();         // store pointer to the old console

   my_cout.open(cname);                  // open the new stream to handle our output
   std::cout.rdbuf(my_cout.rdbuf());     // direct the output to our new stream

};

// Restore the conole output back to the thing that handled it before we took over.
void RestoreConsole(void){

   std::cout.rdbuf(old_cout);

}


// trim the activity file to a manageable size
int TrimFile(string fname, int maxsize){
   char cname[255];
   int size=0;

   // get the file name
   to_cstring(cname, fname, 255);

   size = filesize(cname);

   if (size > maxsize){
     // Momentarily Restore the console
     RestoreConsole();
  
    // restore the buffer
    ConfigureConsole(fname);
  }

  return size;

}



int filesize(char *filename){
     struct stat st;
     size_t retval = 0;
     if(stat(filename,&st) ){
	     // printf("cannot stat %s\n",filename);
     }else{
	   retval = st.st_size;
     }

     return retval;
}

string GetMyDirectory(void){
     char cCurrentPath[FILENAME_MAX];
     #define GetCurrentDir getcwd

     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
         return "";

     cCurrentPath[sizeof(cCurrentPath) - 1] = '/0'; /* not really required */

     return ToString(cCurrentPath, sizeof(cCurrentPath));

};
