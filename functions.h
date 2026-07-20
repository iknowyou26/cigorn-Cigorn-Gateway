// -*- C++ -*-
/* 
 * File:   functions.
 * Author: john
 *
 * Created on July 28, 2010, 7:20 PM
 */

#ifndef _FUNCTIONS_
#define	_FUNCTIONS_
#include <string>
#include "platform/Platform.h"
#include <queue>
using namespace std;

// prototype the functions
string get_username(void);
string intToString(int);
string intToString(int , int );
string IntToHex(int, int);
string IntToHex(int);
string CharToHex(char);
char AsciiHexToChar(char);
string IDToString(int);
int HexToInt(string);
string CharToAsc(char);
time_t StringtoTime(string);
string longToString(long);
string LocalTime(void);
string LocalTime(string);
string LocalDate(void);
string LocalDate(string);
string LocalGMTdiff(void);
string DateTimeString(time_t);
void to_cstring (char*, int, int);
string IntToIP4(int);

void to_cstring (char* , string , int );
bool StringCompare(char *, char *, int);
bool IsSameText(string , string);
std::string ToString(char *);
std::string ToString(const char *);
std::string ToString(char* , int );
std::string ENDL(void);
std::string GetSubString(string , int );
int ExtractFirstNum(char *, int );
string ExtractLine(string , int );
string StringLeft(string , int );
std::string VersionString(int , int , int) ;
string StringToUpper(string);
string StringToLower(string);
int StringToInt(string);
bool StringIsInteger(const string& );
string trim(string);                // custom string trim() funtion
void CopyCstr(char*, char*, int );
string doubleToString(double , int );
string Right(string , int );
bool TextExists(int , char[]);
int StringLen(char *);
int GetIntVal(string);
bool ParseDoublet(string, int&, int&);
bool GetSubInt(string, int, int &);
unsigned long ddhash(char *, int, string);
bool IsStringTrue(char*);
bool IsStringTrue(string );
string FixedRight(string,int);   // Make space-padded right justified fixed-length string
string FixedRight(int, int);
long StringToLong(string );
string LongToString(long );
string DeltaTime(time_t, time_t);
string LocalTimeStamp(void);
string LocalTimeStamp(string, string);
double TimeNow(void);
string BoolToString(bool);
string CrNl(void);
int CharPos(char*, char, int);
int FindString(const char *, string, int);
string CenterString(string , int );
int PositionOfField(char *, int, int);
bool IsHex(char);
bool IsDecimal(char);
bool IsDateTime(const string&);

int HexVal(char );
int DecVal(char );
int DblToInt(double);
double StringToDouble(string);
string CharToString(char);
double AverageQ(vector<double>);
string StreamStrToString(string);
string DisplayHexData(char* , int);
bool WildCardMatch(string, string);
string b64_encode(unsigned char const* , unsigned int );
string b64_decode(std::string const& );

int getMainLoopSpeed();
int getCommLoopSpeed();
int getTCPLoopSpeed();

#define YYYY_MM_DD      "%Y-%m-%d"
#define month_dd_YYYY   "%b %d %Y"
#define HH_MM_SS        "%H:%M:%S"
#define YYYY_WEEK       "%Y-%W"

#endif	/* _FUNCTIONS_ */

