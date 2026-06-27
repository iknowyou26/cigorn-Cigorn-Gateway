// -*- C++ -*-
/* 
 * File:   io_utilities.
 * Author: john
 *
 * Created on July 28, 2010, 7:01 PM
 */

#ifndef _IO_UTILITIES_
#define	_IO_UTILITIES_

  std::string GetVersionNum(void);
  std::string GetVersionText(void);
  void ConfigureConsole(std::string);
  void RestoreConsole(void);
  int isadir( const char* );
  int filesize(char *);
  int TrimFile(std::string, int);
    std::string GetMyDirectory(void);

#endif	/* _IO_UTILITIES_ */

