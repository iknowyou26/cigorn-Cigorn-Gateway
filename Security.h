/* 
 * File:   Security.h
 * Author: john
 *
 * Created on September 1, 2012, 7:20 AM
 */

#ifndef SECURITY_H
#define	SECURITY_H

#include <string>
using namespace std;

#define MAXSECURITYDATA  100
#define MAXENCRYPTEDLEN  256

class Security {
public:
    Security();
    Security(const Security& orig);
    virtual ~Security();

    int RSAencrypt( char* ,char* , unsigned int* );
    string RSAencrypt( string&);
    int RSAkeyGen( std::string&);
    int RSAkeyGen(void);
    int RSAdecode(std::string& );
    int RSAdecode(const char* , size_t , std::string& );
    int RSAdecode(const char* , size_t );
    string RSAdecodeString(string );
    int TestSecurity(int);

    string MyPublicKey;
    string DecodedText;
    string ResultTextGen;
    int DecodedByteCount;
    char DecodedBytes[MAXSECURITYDATA+10];
    char EncodedBytes[MAXENCRYPTEDLEN+10];  // the binary encrypted data
    char EncodedChars[MAXENCRYPTEDLEN+10];  // The ASCII Hex encrypted data
    int EncodedCount;
    size_t MaxDataLength;      // seems to be arbirarily set to 100.  Why

private:

};
#endif	/* SECURITY_H */

