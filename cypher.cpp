/* 
 * File:   cypher.cpp
 * Author: john
 * DES Encryption/Decryption
 * Created on December 18, 2010, 9:37 PM
 */

#include "cypher.h"
#include <string>
extern "C" {
#include "polarssl/des.h"
}
#include "functions.h"
#include <iostream>
static void bits_to_bytes(char *bits, unsigned char *bytes)
{
    for (int byte = 0; byte < 8; byte++) {
        bytes[byte] = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (bits[byte * 8 + bit])
                bytes[byte] |= (1 << bit);
        }
    }
}

static void bytes_to_bits(unsigned char *bytes, char *bits)
{
    for (int byte = 0; byte < 8; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            bits[byte * 8 + bit] = (bytes[byte] & (1 << bit)) ? 1 : 0;
        }
    }
}
using namespace std;

cypher::cypher() {
    cigornkey= "";
}

cypher::cypher(const cypher& orig) {
}

cypher::~cypher() {
}

string cypher::encypher(string s) {

     char key[99] = "123456789";  // 64 bit DES key
     char encrypted[99];
     int i;
     string S;
     string retval;
     string ds;
     
     StringToCbits(cigornkey, key, 64);  // convert the string to some bits in an array of chars

     StringToCbits(s, encrypted, 64);  // convert the string to some bits in an array of chars
     S = CbitsToHex(encrypted, 64);

     unsigned char keybytes[8], inbytes[8], outbytes[8];
     des_context ctx;

     bits_to_bytes(key, keybytes);
     bits_to_bytes(encrypted, inbytes);

     des_setkey_enc(&ctx, keybytes);
     des_crypt_ecb(&ctx, inbytes, outbytes);

     bytes_to_bits(outbytes, encrypted);

     retval = CbitsToHex(encrypted, 64);

    // cout << "key" << retval << endl;
     for (i=0; i< 64; i++){
     //    cout << CharToAsc(encrypted[i]) << ",";
     }
   //  cout << endl;

     // check
     HexToCbits(retval, encrypted, 64);  // convert the string to some bits in an array of chars

     for (i=0; i< 64; i++){
       //  cout << CharToAsc(encrypted[i]) << ".";
     }
    // cout << endl;

     bits_to_bytes(encrypted, inbytes);
     des_setkey_dec(&ctx, keybytes);
     des_crypt_ecb(&ctx, inbytes, outbytes);
     bytes_to_bits(outbytes, encrypted);
     ds = CbitsToHex(encrypted, 64);

     return retval;
     
}

// set the key for all subsequent uses of the cypher
void cypher::newkey(string s){

     char key[99];               // 64 bit DES key
     s = trim(s);
     cigornkey = s;
     StringToCbits(s, key, 64);  // convert the string to some bits in an array of chars
     //des_setparity(key);
     //setkey(key);

}

// test the cypher engine to make sure it is working OK
// return the number of test we fail.
int cypher::testcypher(void)
{
    string inputText = "1234";
    string encryptedText;
    string outputText;
    string oldkey = cigornkey;
    int retval = 0;

    newkey("t7&dL~g");
    encryptedText = encypher(inputText);
    outputText = decypher(encryptedText);

    if (inputText != outputText)
    {
        retval++;
    }

    newkey("1234567");
    inputText = "1234";
    encryptedText = encypher(inputText);
    outputText = decypher(encryptedText);

    if (inputText != outputText)
    {
        retval++;
    }

    inputText = "cigorn";
    encryptedText = encypher(inputText);
    outputText = decypher(encryptedText);

    if (inputText != outputText)
    {
        retval++;
    }

    // fbccda9146486f28 decodes to 1234 with key "t7&dL~g"
    newkey("t7&dL~g");
    inputText = "1234";
    encryptedText = "fbccda9146486f28";
    outputText = decypher(encryptedText);

    if (inputText != outputText)
    {
        retval++;
    }

    newkey(oldkey);
    return retval;
}


// Take an encrypted string of 1-8 charactors, and decrypt it using DES.
// Return a string of the decrypted data.
string cypher::decypher(string s) {

     char key[] = "123456789";  // 64 bit DES key
     char encrypted[99];
     int i;
     string S;
     string retval;

     //s =  "336e8b02c83a257b";
     //cout << "Input:" << s << endl;
     HexToCbits(s, encrypted, 64);  // convert the string to some bits in an array of chars

 //    des_setparity(key);
 //    setkey(key);

     // run the decryption
     char keybits[99];
     unsigned char keybytes[8], inbytes[8], outbytes[8];
     des_context ctx;

     StringToCbits(cigornkey, keybits, 64);
     bits_to_bytes(keybits, keybytes);
     bits_to_bytes(encrypted, inbytes);

     des_setkey_dec(&ctx, keybytes);
     des_crypt_ecb(&ctx, inbytes, outbytes);

     bytes_to_bits(outbytes, encrypted);
     S = CbitsToHex(encrypted, 64);
     retval = trim(HexToString(S)); // The STRING WILL have trailing NULs filling it, so trim them off

     return retval;

}

// Convert a hex string to an array of bits
void cypher::HexToCbits(string s, char* c, int max){
    int byte = 0;
    int bit = 0;
    int n = 0;

    while (n < max){

        if (byte < s.size()){
            if ((HexVal(s[byte]) & (1 << bit)) == 0)
                *c = 0;
            else
                *c = 1;
        }
        else
            *c = 0;
     //   cout << "b" << s[byte] << "*" << CharToAsc(*c);
        bit++;
        c++;
        n++;
        if (bit == 4){
            bit = 0;
            byte++;
            //cout << endl;
        }
    }
      //  cout << endl;

}


// Convert a string of hex digits to the ASCII
string cypher::HexToString(string s){
    int i=0;
    int byte = 0;
    string o = "";  // output string

    while (byte < s.size()){
        i = HexVal(s[byte]) ;
        byte++;
        if (byte <= s.size())
            i = HexVal(s[byte]) * 16 + i;
        byte++;
        o = o + (char)i;
    }
    return o;
}

// Convert an array of 1s 0s to a string of two-digit hex chars
string cypher::CbitsToHex(char* c, int max) {
    string s;
    int byte = 0;
    int bit = 0;
    int n = 0;
    int i = 0;
    
    while (n < max){
        if (*c > 0)
            i = i + (1 << bit);
        n++;  // next bit in the array 
        bit++;
        c++;
        if (bit == 4){
            bit = 0;
            s = s + IntToHex(i,1);
            byte++;
            i = 0;
        }
    }
    return s;
}

// Convert a string to a series of bits 0 or 1 stored in a char array.
void cypher::StringToCbits(string& s, char* c, int max) {

    int byte = 0;
    int bit = 0;
    int n = 0;
    char ch;

    while (n < max){

        if (byte < s.size()){
            //cout << s[byte];
            ch = (char)s[byte];
            if ((ch & (1 << bit)) == 0)
                *c = 0;
            else
                *c = 1;
        }
        else{
            *c = 0;
        }

        bit++;
        c++;
        n++;
        if (bit >= 8){
            bit = 0;
            byte++;
           // cout << endl;
        }
    }
    //cout << endl;

}

