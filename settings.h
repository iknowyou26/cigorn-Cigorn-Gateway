/* 
 * File:   settings.h
 * Routines related to getting, saveing, and managing the settings for this program.
 * Author: john
 *
 * Created on August 26, 2010, 9:05 PM
 */

#ifndef _SETTINGS_H
#define	_SETTINGS_H

#include <string>
#include "emailer.h"
#include "webserver.h"

// prototype the functions
int readini(std::string);
void ReadSettings(datatable*);
void ConfigureEmail(datatable*, emailer*);
void ConfigureWeb(datatable*, webserver*);
void ConfigParameter(string , string , string , string , string , string );
void ReadTable(datatable* );

#endif	/* _SETTINGS_H */

