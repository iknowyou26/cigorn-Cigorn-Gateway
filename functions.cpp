
/*
 * File:   functions.cpp
 * Created on July 27, 2010, 9:26 PM
 *  Various generic functions we wrote to to handle things like
 *  conversions.
 */

using namespace std;
#include "platform/PlatformConstants.h"
#include "platform/time/PlatformTime.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "functions.h"
#include "ascii.h"
#include "stdio.h"
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#ifndef _WIN32
#ifndef _WIN32
#include <unistd.h>
#include <cstdlib>
#endif
#endif
static std::string CigornGetLoginName()
{
#ifdef _WIN32
    const char* username = std::getenv("USERNAME");
#else
    const char* username = getlogin();
#endif

    return username != nullptr ? std::string(username) : std::string();
}

// Return the operating-system user name.
string get_username(void)
{
    return CigornGetLoginName();
}


// convert integer to a c type string
void to_cstring (char* c, int t, int lenmax)
    {
    int x = 0;
    string s;
    std::stringstream ss;
    ss << t;
    s = ss.str();

    while (x< lenmax){
        c[x] = s[x];
        if (s[x] == NUL) return;
        x++;
    }
    c[lenmax] = NUL;
}

// Find the length of a cstring
int StringLen(char *c){
    int i = 0;
    for (i=0; i< 1000; i++){
        if (c[i] == NUL)
          return i;
    }

    return -1;

}

bool IsSameText(string s1, string s2){
    if (StringToUpper(trim(s1)) == StringToUpper(trim(s2)))
        return true;
    else
        return false;
}

// see if the two strings are the same. stop comparing at any NUL or m characters
// return true if they compare
bool StringCompare(char *c1,char *c2, int m){
    int i = 0;

    if ((*c1 == NUL) || (*c2 == NUL))
        return false;  // a null string will not compare.

    while (m > 0){
        if ((c1[i] == NUL) || (c2[i] == NUL))
            return true;
        if (c1[i] != c2[i])
            return false;
        i++;
        m--;
    }
    return true;  // all compared
}

// Find line n in the text s
string ExtractLine(string s, int n){
    int l = 1;
    int c = 0;
    string o = "";
    bool done = false;

    if (s.size() == 0)
        return "";  // nothing in the string


    while ((!done) && (c < s.size())){
        if ((s[c] == NL) || (s[c] == CR)){
            l++; // next line
            c++;
            // see if the next char is a CR or LF also
            if (c < s.size()){
                if ((s[c] == NL) || (s[c] == CR))
                    c++; // there is a
            }
            c--;
        }

        if (((s[c] != NL) && (s[c] != CR)) && (l == n))
           o = o + s[c];
        c++;
        if (l>n)
            done = true;
    }

    return o;

}

// Find the first number in the string, parse it and return the positive integer value.
int ExtractFirstNum(char *c1, int m){
    int i = 0;
    int r = 0;
    int digits = 0;

    if ((*c1 == NUL) )
        return -1;      // a null string has no response

    while (m > 0){
        if ((c1[i] == SP) && (digits > 0))
            return r;   // done
        if (c1[i] > '9')
            return r;   // done
        if (c1[i] == NUL)
            return r;
        if ((c1[i] >= '0') && (c1[i] <= '9')){
            r = r * 10 + (c1[i]-'0');  // add thenumber
            digits++;
        }
        i++;
        m--;
    }
    return r;  // all done

}

// Find the position in the NULL terminated C string of field m. Comma deliminated.  max digits long
int PositionOfField(char *c1, int m, int max){
    int i = 0;
    int r = 0;
    int fcount = 0;

    if ((*c1 == NUL) )
        return -1;      // a null string has no response

    while (m > 0){
        if (c1[i] == ','){
            // Here is a new field
            if (fcount == m){
                // Found it. Point to the first char in the field
                if ((i<max) && (c1[i+1] != NULL))
                    return i+1;
                else
                    return i;
            }
            fcount = fcount+1;  // count the fields
        }
        // done?
        if (c1[i] == NUL)
            return -1;
        i++;
        if (i >= max)
            return -1;
    }
    return -1;  // not found

}

// search the c1 cstring for the s string.  max m bytes in C1
int FindString(const char *c1, string s, int m){
    int i = 0;
    int x = 0;
    bool done = false;
    bool nomatch = false;

    if ((*c1 == NUL))
        return -1;  // a null string will not compare.

    while (i < m){
        if (i>=m)
            return -1;  // not found
        if (c1[i]== NUL)
            return -1;
        if ((m-i) < s.size())
            return -1;  // ran out of chars

        if (c1[i] == s[0]){
            // we may have a match
            nomatch = false;
            // see if the strings match
            for (x=0; x<s.size(); x++){
                if(c1[i+x] != s[x]){
                   nomatch = true;
                   break;
                }
            }
            if (nomatch == false)
                return i;  // found the string.
        }
        i++;

    }
    return -1;  // no compare

}

std::string ENDL(void){
    string s = "  ";
    s[0] = CR;
    s[1] = NL;
    return s;
}
// convert string to a c type string
void to_cstring(char *c, const string s, int lenmax)
    {
    int x = 0;
    
    while (x < lenmax){
        *c = s[x];
        if (s[x] == NUL) return;
        x++;
        c++;
    }
}

// Copy a c string to a string
string ToString(char* s1, int maxl){
    string s = "";
    int i = 0;

    if(s1 != 0){ // Check for null pointer
        while ((*s1 != NUL) && (i < maxl)){
            s = s + *s1;
            s1++;
            i++;
        }
    }
    return s;

}

// Copy a c string from one place to another.  s2 > s1.
// NUL terminate, so it addes one byte to the end of the string!
// Stops copying when it hits a NULL.
void CopyCstr(char* s1, char* s2, int maxl){
    int i=0;
    while ((*s2 != NUL) && (i<maxl)){
        *s1 = *s2;
        s1++; s2++;
        i++;
    }
    *s1 = NUL;  // null terminate the string.
}

std::string VersionString(int v1, int v2, int v3){
    string s;
    s=intToString(v1) + "." + intToString(v2) + "." + intToString(v3);
    return s;
}

// is a string true?
bool IsStringTrue(char *c){
    if ((*c == 't') || (*c == 'T') || (*c == '1'))
        return true;
    else
        return false;
}

// is a string true?
bool IsStringTrue(string s){
    if (s.size() == 0)
        return false;
    
    if ((s.at(0) == 't') || (s.at(0) == 'T') || (s.at(0)== '1'))
        return true;
    else
        return false;
}

string CharToString(char c){
    string s="";
    if (c != NUL)
        s = s + c;
    return s;
}

string BoolToString(bool v){
    if (v == true)
        return "TRUE";
    else
        return "FALSE";
}

string ToString(char *c)
{
    string s = "";

    while (*c != NUL){
        s = s + *c;
        c++;
    }

   return s;
}

string ToString(const char *c)
{
    if (c){
        std::stringstream ss;
        ss << c;
        return ss.str();
    }
    else
        return "";
}

std::string doubleToString(double x, int n){

    std::stringstream ss;
    std::string s;
    ss << fixed;
    ss.precision(n);
    ss << x;
    s = ss.str();

    return s;
}

// return true if the string is an integer number
// Every digit must be a number
bool StringIsInteger(const string& s){
    int i = 0;
    bool digit = false;

    if (s.empty())
        return false;

    if (s.size() == 0)
        return false;

    // go past any leading spaces
    while (i< s.size()){
        if (s[i] == ' ')
            i++;
        else
            break;
    }

    while (i< s.size()){
        if ((s[i]<'0') || (s[i]>'9'))
            return false;
        digit = true;     // we found a digit
        i++;
    }
    return digit;

}

long StringToLong(string s){

      std::istringstream inpStream(s);
      long inpValue = 0;

      if (inpStream >> inpValue)
      {
        return inpValue;// ... Success!!  test is a number.
      }
      else
      {
        return 0;// ... Failure!!  test is not a number.
      }
}


int StringToInt(string s){


      if (s.empty())
        return 0;

      if (s.size() > 20)
        return 0;
    
      std::istringstream inpStream(s);
      int inpValue = 0;

      if (inpStream >> inpValue)
      {
        return inpValue;// ... Success!!  
      }
      else
      {
        return 0;// ... Failure!!  
      }
}

// Find a number withing a string, and return its integer value
int GetIntVal(string s){
    int x = 0;
    int y;
    bool num = false;

    for (y=0; y<s.size(); y++){
        if ((s[y] >= '0') && (s[y] <= '9')){
            num = true;
            x = x*10 + (s[y] - '0');
        }
        else{
            if (num == true)
                return x;
        }
    }
    return x;
}

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}


// Return a formatted string "xxx (hhhh hex)"
std::string IDToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str() + "(" + IntToHex(i, 4) + "hex)";

    return s;
}


// Int i to a string.  dd digits with leading zeros
std::string intToString(int i, int d)
{
    stringstream ss;
    string s="";
    bool  neg = false;
    if (i < 0){
        neg = true;
        i = -1 * i;
    }

    ss << i;                // make a string
    s = ss.str();

    // pad with leading zeros
    while (s.size() < d)
       s = "0" + s;

    if (d < (s.size()-1))
       s = s.substr(s.size()-d, s.size());

    if (neg == true){
        s = "-" + s;
    }
    else{
        s = " " + s;
    }

    return s;
}


char AsciiHexToChar(char c){

    if (c <= '9')
        return (c-'0');
    if ((c >= 'a') && (c <= 'f'))
        return (c - 'a' + 10);
    if ((c >= 'A') && (c <= 'F'))
        return (c - 'A' + 10);

    return 0;

}

int HexToInt(string s){
    unsigned int x;
    std::stringstream ss;
    ss << std::hex << s;
    ss >> x;
    return x;
}

std::string IntToHex(int i)
{
    stringstream ss;
    string s;

    ss << std::hex << i;                // make a hex string
    s = ss.str();
 
    // Convert a-f to A-F
    for(unsigned int i=0; i < s.length(); i++){
      s[i] = toupper(s[i]);
    }
    return s;
}

string IntToIP4(int ip){
  stringstream ss;
  char result[16];

  sprintf(result, "%d.%d.%d.%d",
    (ip & 0xFF),
    (ip >> 8) & 0xFF,
    (ip >> 16) & 0xFF,
    (ip >> 24) & 0xFF);

  ss << result;
  return ss.str();

}

std::string CharToAsc(char c)
{
    int i;
    i = c;
    return intToString(i);
}

std::string LongToString(long i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();
    return s;
}

// return left p chars of string st
string StringLeft(string st, int p){
    int i = 0;
    std::stringstream ss;
    std::string s = "";

    if (p > st.size())
        return s;
    
    while ((i < p) && (i < st.size())){
       ss << st.at(i);
       i++;
    };
    s = ss.str();

    return s;
}

string StringToUpper(string strToConvert)
{//change each element of the string to upper case
    if (strToConvert.size() == 0)
        return "";

   for(unsigned int i=0;i<strToConvert.length();i++){
      strToConvert[i] = toupper(strToConvert[i]);
   }
   return strToConvert;//return the converted string
}

string StringToLower(string strToConvert)
{//change each element of the string to lower case
   for(unsigned int i=0;i<strToConvert.length();i++)
   {
      strToConvert[i] = tolower(strToConvert[i]);
   }
   return strToConvert;//return the converted string
}

// Return the integer value of the p filed in the s string of fields separated by commas
// Use to get integers out of a NMEA message.
bool GetSubInt(string s, int p, int &retval){
    string st = GetSubString(s, p);

    if (st.size() == 0)
        return false;

    retval = StringToInt(st);

    return true;

}

// return a substring of string s. Separator is a space or comma.
// " { [ (  designate strings that may have spaces or commas
string GetSubString(string s, int p)
{
    int i = 0;
    int fcount = 0;
    char gp = NUL;     // the grouping charator if we run into one:  "([{
    std::vector<char> cs(s.length() + 1);
    string rets = "";
    char sp;

    
    to_cstring(cs.data(), s, s.length());


    while (i < s.length()){
        if ((cs[i] != NUL) && (cs[i] != CR) && (cs[i] != NL)){
            // First see what type of text separator we are looking for.
            sp = '"';
            if (gp == NUL){
                // There is currently no text separator
                if ((cs[i] == '(') || (cs[i] == '{') || (cs[i] == '[') || (cs[i] == '"') || (cs[i] == '&')){
                    gp = cs[i];  // we found a text separator charactor

                }
            }
            if (gp == '(')
                sp = ')';
            if (gp == '[')
                sp = ']';
            if (gp == '{')
                sp = '}';
            if (gp == '"')
                sp = '"';

            if ((((cs[i] != SP) && (cs[i] != ',') && (cs[i] != ':')) && (gp == NUL))
                                || ((gp != NUL) && (cs[i] != sp) && (cs[i] != '&') )  ) {
                // Build up the sub-string
                rets = rets + cs[i];
                if (fcount == 0){
                    fcount++;   // we got the first field
                }
            }
            else
            { // We hit a separator
              if (rets.length() > 0) {
                 if ((gp != NUL) && (gp != '"') && (cs[i] != '&'))
                     rets = rets + cs[i];  // append the separator to the string, unless it was a quote mark.
                 if (fcount == p)
                    break;  // we found the string. done
                 // We found a string. Count it
                 fcount++;
                 gp = NUL;   // start over looking for text separated by spaces and commas
                 sp = NUL;
                 rets="";    // start over
              }
              rets= "";   // new string
              if (cs[i] == NL){
                  // We hit the end of the line
                  break;
              }

            }
        }
        else{
            // We reached the end of the input line
            break;  // exit the while...
        }
        
        i++;
    }
    if (fcount == p)
        return rets;
    else
        return "";
;
}


unsigned long ddhash(char *str, int n, string strg) {
        unsigned long myhash = 0;
        unsigned long total = 0;

        unsigned long c;
        int i=0;

        while ((i < n)){
            c = (unsigned char)str[i];
            if (i < strg.size())
               c = c + strg[i] * 256;
            total = total + c;
            if (myhash & 0x80000000)
                myhash = c ^ ((myhash << 1) | 1 );
            else
                myhash = c ^ (myhash << 1);
            i++;
            if ((myhash & 0x80000000) || (myhash & 0x10000))
                myhash = (myhash << 1) | 1;
            else
                myhash = (myhash << 1);
            if ((myhash & 0x80000000) || (myhash & 0x200))
                myhash = (myhash << 1) | 1;
            else
                myhash = (myhash << 1);
        }
        return myhash + total;
    }

std::string longToString(long i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

string Right(string s, int l){
    
    if (l <= s.size()){
       s = s.substr(0,l);
    }
    return s;
    
}

// Trim spaces, and tabs from beginning. Trim trailing CR and LF off also.
string trim(const string st)
{
	int i=0;
        bool done = false;
        string s = st;

        if (s.size() == 0)
            return "";

        // Trim off leading spaces and tabs
        while((done == false) && (s.size() > 0)) {
            if ((s[0]==' ') || (s[0]=='\t'))
                s.erase(0,1);// erase the space or tab
            else
                done = true;
	}

        i = s.size() - 1;
        done = false;
	while((i>=0) && (done == false)){
            if ((s[i]==' ') || (s[i]=='\t') || (s[i]== CR) || (s[i]== NL) || (s[i]== NUL))
                s.erase(i,1);
            else
                done = true;  // no bad chars
            i = s.size() - 1;
        }

        return s;
}



string DeltaTime(time_t t1, time_t t2){
    string s = "";
    double d;
    int days;
    double hours;
    std::stringstream ss;
  
  if ( t1 != (time_t)(-1) && t2 != (time_t)(-1) )
   {
	 d = difftime(t1, t2) / (60 * 60 * 24);
         days = (int)d;
         hours = (d - days)*24;
	 //ss << ctime(&t1);
	 //ss << ctime(&t2);
	 ss << days << " days, " << doubleToString(hours, 3) << " hours" << endl;
         return ss.str();
   }
    return "";
}


// "%H:%M:%S"
string LocalTime(){
    return LocalTime(HH_MM_SS);
}

// broken sub.
time_t StringtoTime(string S)
{
    time_t retval = time(NULL);
    struct tm tyme{};

    std::istringstream input(S);
    input >> std::get_time(&tyme, "%H:%M:%S");

    if (!input.fail())
    {
        cout << "T:" << tyme.tm_hour
             << " M:" << tyme.tm_min
             << " S:" << tyme.tm_sec
             << endl;

        retval = mktime(&tyme);
    }

    return retval;
}

// return a string of the Local Time in the HH:MM:SS 24-hour format
string LocalTime(string frmt){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  strftime (buffer,80, frmt.c_str(),timeinfo);
  return string(buffer);

}


// return a string of the time and date
string DateTimeString(time_t tm){

  struct tm * timeinfo;
  char buffer[80];
  string frmt = "%Y-%m-%d %H:%M:%S";

  timeinfo = localtime( &tm );
  strftime (buffer,80, frmt.c_str(),timeinfo);
  return string(buffer);

}


string LocalDate(){
    //month_dd_YYYY
    return LocalDate(month_dd_YYYY);
}

// return a string of the Local Date  24-hour format
string LocalDate(string frmt){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time ( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime (buffer,80,frmt.c_str(),timeinfo);
  return string(buffer);
}

// Return the number of seconds with 4 digits of precision.
double TimeNow(void)
{
    return static_cast<double>(cigorn::UnixTimeMilliseconds()) / 1000.0;
}



// return true if the string is a valid date time string
bool IsDateTime(const string& s){
   // need to write this...
    return true;
}

string LocalTimeStamp(){

    return LocalTimeStamp(month_dd_YYYY,  HH_MM_SS);
}

string LocalTimeStamp(string Dfmt, string Tfmt){
    
    string s = LocalDate(Dfmt.c_str()) + " " + LocalTime(Tfmt.c_str());

    time_t curr=time(0);                      // current local time
    tm local = *gmtime(&curr);                // convert curr to GMT, store as tm
    time_t utc = (mktime(&local));            // convert GMT tm to GMT time_t
    double diff = difftime(curr,utc)/3600;    // difference in hours

    int d = diff;

    if (d >= 0)
        s = s + "+" + intToString(d, 2);
    else
        s = s + intToString(d, 2);
    return s;

}

string LocalGMTdiff(void){
    string s = "";

    time_t curr=time(0);                      // current local time
    tm local = *gmtime(&curr);                // convert curr to GMT, store as tm
    time_t utc = (mktime(&local));            // convert GMT tm to GMT time_t
    double diff = difftime(curr,utc)/3600;    // difference in hours

    int d = diff;
    if (d >= 0)
        s = "GMT+" + intToString(d, 2);
    else
        s = "GMT-" + intToString(d, 2);
    return s;

}



bool TextExists(int argc, char argv[]){
    return true;
}
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <queue>
// Sleep milliseconds
void sleepMilli(long ms)
{
    if (ms <= 0)
        return;

    cigorn::SleepMilliseconds(static_cast<std::uint64_t>(ms));
}

// Parse a two-integer string in the format of  {aaaa bbbb} where aaaa is the first number and
// bbb is the second integer.
bool ParseDoublet(string idlimit, int &lowID, int &upID ){
    int i = 0;
 
    lowID = -1;
    upID  = -1;
    idlimit = trim(idlimit);

    // First step over any non-numeric charactors
    while ((i < idlimit.size()) && ((idlimit[i] < '0') || (idlimit[i] > '9'))) {
        i++;
    }
    // Read in the first number
    if (i < idlimit.size()){
        lowID = 0;   // there is a digit here
        while ((i < idlimit.size()) && (idlimit[i] >= '0') && (idlimit[i] <= '9')) {
            lowID = lowID * 10 + (idlimit[i] - '0');
            i++;
        }
    }

    // Now step over any non-numeric charactors
    while ((i < idlimit.size()) && ((idlimit[i] < '0') || (idlimit[i] > '9'))) {
        i++;
    }

    // Read in the second number
    if (i < idlimit.size()){
        upID = 0;   // there is a digit here
        while ((i < idlimit.size()) && (idlimit[i] >= '0') && (idlimit[i] <= '9')) {
            upID = upID * 10 + (idlimit[i] - '0');
            i++;
        }
    }

    if ((lowID >=0) && (upID >=0))
        return true;
    else
        return false;
}

// Make space-padded right justified fixed-length string
string FixedRight(string s ,int i){
    if (i<1)
        return "";

    while (s.size() < i)
        s = " " + s;

    return Right(s, i);

}

string FixedRight(int n, int i){

    if (i<1)
        return "";

    string s = intToString(n);
    return FixedRight(s,i);

};

// Return a string of width w, s in center padded with spaces
string CenterString(string s, int w){
    int x = s.size();

    if (x >= w)
        return s;

    while (s.size() < (x+(w-x)/2)){
        s = " " + s;
    }
    while (s.size() < w){
        s = s + " ";
    }

    return s;

};


string CrNl(void){
    string s;

    s = s.append(1,CR);
    s = s.append(1,NL);

    return s;

}

// return the position of c in buf.  maximum value is max. 
// -1 meand not found.  0=first position.
int CharPos(char *buf, char c, int max){
    int i = 0;
    while (i < max){
        if (*buf == c)
            return i;
        buf++;
        i++;
    }
    return -1;// not found.
}

bool HEX[] =
      {
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          true,  true,  true,  true,  true,  true,  true,  true,
          true,  true,  false, false, false, false, false, false,
          false, true,  true,  true,  true,  true,  true,  false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, true,  true,  true,  true,  true,  true,  false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false,
          false, false, false, false, false, false, false, false };

bool IsHex(char c){
    if (c > 0x7F)
        return false;

    return HEX[c];

}

bool IsDecimal(char c){
   if (( c>= '0') && ( c <= '9'))
       return true;
   else
       return false;

}

// Convert a single ASCII hex digit to its decimal value
int HexVal(char c){
    if (( c>= '0') && ( c <= '9'))
            return (c-'0');
    if (( c>= 'a') && ( c <= 'f'))
            return (c-'a' + 10);
    if (( c>= 'A') && ( c <= 'F'))
            return (c-'A' + 10);
    return -1;

}


// Convert i to a hex string with d digits
string CharToHex(char ch){
    string s="";
    char i;

    i = ((ch & 0xf0) / 16);
    if (i < 10)
        i = i + '0';
    else
        i = i - 10 + 'a';
    s = s + i;
    i = (ch & 0x0f);
    if (i < 10)
        i = i + '0';
    else
        i = i - 10 + 'a';
    s = s + i;
    
  return s;

}

// Convert i to a hex string with d digits
string IntToHex(int i, int d){
    stringstream ss;
    string s;

    ss << hex << i;   // make a hex string
    s = ss.str();

    // Convert a-f to A-F
    for(unsigned int i=0; i < s.length(); i++){
       s[i] = toupper(s[i]);
    }
    
    // leading zeros
    while (s.size() < d)
        s = "0" + s;

    if (d <= s.size()){
       s = s.substr(s.size()-d, s.size());
    }
    return s;

}

// Convert a single ASCII digit to its decimal value
// Return -1 if invalid
int DecVal(char c){
    if (( c>= '0') && ( c <= '9'))
            return (c -'0');

    return -1;

}

// Convert a double to an integer
int DblToInt(double d){
    int i;
    i = d;
    return i;
}

double StringToDouble(string s){

      std::istringstream stm;
      stm.str(s);
      double d;
      stm >> d;
      return d;
}

string DisplayHexData(char* c, int count){
    stringstream ss;
    int i, y;

    ss << "Data:" ;

    for (i=0; i<count; i++){
        y = c[i];
        ss << IntToHex(y, 2) << " ";
    }

    return ss.str();

}


// Average all of the entries in a queue.
double AverageQ(vector<double>  q){
    int i;
    double d=0;
    if (q.size()<= 0)
        return 0;
    for (i=0; i < q.size(); i++){
        d = d+q.at(i);
    }
    d = d /q.size();
    return d;
}

string StreamStrToString(string s){
    return s;
}

// Test to see if S2 matched S1, using * for wildcard match.  
// s1 may use a wildcard charactor
bool WildCardMatch(string s1,string s2){
    int i = 0;

    while ((i < s1.size()) && (i < s2.size())){
        if (s1[i] == '*')
            return true;   // we matched
        if (s1[i] != s2[i])
            return false;  // they do not match
        i++;  
    }

    // We got here without finding a mismatch. Are they the same length?
    while (s1.size() == s2.size())
        return true;   // they matched and were the same length.

    return false;
}


#include <iostream>
#include <string>
#include <vector>
#include <assert.h>

string b64_encode(const std::string& src){
    static const char base64_table[] ={
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/"
    };
    std::string dst;
    dst.reserve(((src.size()+2)/3)*4);

    for(unsigned src_pos = 0; src_pos < src.size(); src_pos += 3){
        const unsigned filling =
        (src_pos + 3)>src.size() ?
        src.size() - src_pos :
        3;

        dst += base64_table[unsigned(src[src_pos + 0] & 0xFC) >> 2]; // FC = 11111100
        if(filling == 1){
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4)]; // 03 = 11
            dst += '=';
            dst += '=';
        }
        else if(filling == 2)
        {
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4) | (unsigned(src[src_pos + 1] & 0xF0) >> 4)]; // 03 = 11
            dst += base64_table[((src[src_pos + 1] & 0x0F) << 2) | (unsigned(src[src_pos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
            dst += '=';
        }
        else
        {
            assert(filling == 3);
            dst += base64_table[((src[src_pos + 0] & 0x03) << 4) | (unsigned(src[src_pos + 1] & 0xF0) >> 4)]; // 03 = 11
            dst += base64_table[((src[src_pos + 1] & 0x0F) << 2) | (unsigned(src[src_pos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
            dst += base64_table[src[src_pos + 2] & 0x3F]; // 3F = 111111
        }
    }
    return dst;
}

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
   return (isalnum(c) || (c == '+') || (c == '/'));
}

string b64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}

string b64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}


