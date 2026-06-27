
#ifndef _CRT_SECURE_NO_DEPRECATEe
#define _CRT_SECURE_NO_DEPRECATEe 1
#endif

#include <string.h>
#include <stdio.h>
#include <iostream>

#include "polarssl/config.h"

#include "polarssl/rsa.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "Test1.h"
#include "functions.h"
#include "Security.h"
#include "Cigorn.h"

using namespace std;


Security::Security() {
   MyPublicKey = "";
   DecodedText = "";
   DecodedByteCount = 0;
   MaxDataLength = MAXSECURITYDATA;      // seems tobe arbirarily set to this.  Why?
   DecodedBytes[0] = NULL;
   EncodedCount=0;
   EncodedChars[0] = NULL;
   ResultTextGen="";
}

Security::Security(const Security& orig) {

}

Security::~Security() {

}

// Encrypt a string and return the cyphered text
string Security::RSAencrypt( string& textIn){

    unsigned int thecount=0;
    char ch;
    int i;
    string retval="";

    if (textIn.size()== 0)
        return "";

    RSAencrypt((char*)textIn.c_str(), EncodedBytes, &thecount);

    i = 0;
    while (i < thecount){
        ch = EncodedBytes[i];
        //cout << " E*" << (int)EncodedBytes[i] << "=" << CharToHex(ch);
        retval = retval + CharToHex(ch);
        i++;
    }

    i = 0;
    while ((i < retval.size()) && (i < MAXENCRYPTEDLEN)){
        EncodedChars[i] = retval[i];
        i++;
    }
    EncodedChars[i] = NULL;
    EncodedCount = thecount;
    return retval;

}


int Security::RSAencrypt( char* DataIn, char* EncryptedData, unsigned int* EncCount)
{
    FILE *f;
    int ret;
    size_t i;
    rsa_context rsa;
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    unsigned char input[1024];
    unsigned char buf[512];
    char *pers = "rsa_encrypt";

    ret = 1;

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
                               (unsigned char *) pers, strlen( pers ) ) ) != 0 ){
        //printf( " failed\n  ! ctr_drbg_init returned %d\n", ret );
        goto exit;
    }

    //printf( "\n  . Reading public key from rsa_pub.txt" );

    if( ( f = fopen( "rsa_pub.txt", "rb" ) ) == NULL )
    { // faile to open file
        ret = 1;
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = mpi_read_file( &rsa.N, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.E, 16, f ) ) != 0 ) {
        //printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
        goto exit;
    }

    rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
    fclose( f );

    if( strlen( DataIn ) > MaxDataLength )
    {
        // printf( " Input data larger than 100 characters.\n\n" );
        goto exit;
    }

    memcpy( input, DataIn, strlen( DataIn ) );

     // Calculate the RSA encryption of the hash.
    if( ( ret = rsa_pkcs1_encrypt( &rsa, ctr_drbg_random, &ctr_drbg, RSA_PUBLIC, strlen( DataIn ),
                                   input, buf ) ) != 0 ){
        // failed to encrypt
        goto exit;
    }

    // Store the encrypted data to the c array
    for( i = 0; i < rsa.len; i++ ){
        EncryptedData[i] = buf[i];
    }
    *EncCount = (unsigned int)i;
    
    
exit:

    return( ret );
}

/*
 *  Example RSA key generation program
 *
 *  Copyright (C) 2006-2011, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#define KEY_SIZE 1024
#define EXPONENT 65537

// create my public key for this instance of the class
int Security::RSAkeyGen(void){
    int i;
    i = RSAkeyGen(MyPublicKey);
    return i;
}

int Security::RSAkeyGen(string& PubKey)
{
    size_t sz;
    sz = 1024;
    char cbuff[1024];
    int ret;
    rsa_context rsa;
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    FILE *fpub  = NULL;
    FILE *fpriv = NULL;
    char *pers = "rsa_genkey";
    stringstream ss;   // use for debug message outputting

    ResultTextGen="";
    fflush( stdout );

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
                               (unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        //CoutM2(ss) << "RSA Key Gen error. ctr_drbg_init returned:" << ret <<  endl;
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = rsa_gen_key( &rsa, ctr_drbg_random, &ctr_drbg, KEY_SIZE,
                             EXPONENT ) ) != 0 )
    {
        //CoutM2(ss) << "RSA Key Gen error. rsa_gen_key returned:" << ret <<  endl;
        goto exit;
    }

    mpi_write_string( &rsa.N, 16, cbuff, &sz);
    PubKey = ToString(cbuff);

    if( ( fpub = fopen( "rsa_pub.txt", "wb+" ) ) == NULL )
    {
        //CoutM2(ss) << "RSA Key Gen error. rcould not open rsa_pub.txt."  <<  endl;
        ret = 1;
        goto exit;
    }

    if( ( ret = mpi_write_file( "N = ", &rsa.N, 16, fpub ) ) != 0 ||
        ( ret = mpi_write_file( "E = ", &rsa.E, 16, fpub ) ) != 0 )
    {
        //CoutM2(ss) << "RSA Key Gen error. mpi_write_file error:" << ret <<  endl;
        goto exit;
    }

    if( ( fpriv = fopen( "rsa_priv.txt", "wb+" ) ) == NULL )
    {
        //CoutM2(ss) << "RSA Key Gen error. rcould not open rsa_priv.txt."  <<  endl;
        ret = 1;
        goto exit;
    }

    if( ( ret = mpi_write_file( "N = " , &rsa.N , 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "E = " , &rsa.E , 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "D = " , &rsa.D , 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "P = " , &rsa.P , 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "Q = " , &rsa.Q , 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "DP = ", &rsa.DP, 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "DQ = ", &rsa.DQ, 16, fpriv ) ) != 0 ||
        ( ret = mpi_write_file( "QP = ", &rsa.QP, 16, fpriv ) ) != 0 )
    {
        //CoutM2(ss) << "RSA Key Gen error. mpi_write_file err:" << ret <<  endl;
        goto exit;
    }

exit:

    if (ret == 0)
        CoutM0(ss) << "RSA Key Generated." << endl;
    else
        CoutM0(ss) << "RSA Key Generation failed. Code" << ret <<  endl;

    ResultTextGen = ss.str();
    ss.str("");                   // clear the buffer

    if( fpub  != NULL )
        fclose( fpub );

    if( fpriv != NULL )
        fclose( fpriv );

    rsa_free( &rsa );

    return( ret );
    // Convert the public key to a string
    
}


int Security::RSAdecode(string& txt)
{
    FILE *f;
    int ret, c;
    size_t i;
    rsa_context rsa;
    unsigned char result[1024];
    unsigned char buf[512];

    memset(result, 0, sizeof( result ) );
    ret = 1;

    // printf( "\n  . Reading private key from rsa_priv.txt" );

    if( ( f = fopen( "rsa_priv.txt", "rb" ) ) == NULL )
    {
        // Could not open rsa_priv.txt\n" \\ Please run rsa_genkey first\n\n" );
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = mpi_read_file( &rsa.N , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.E , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.D , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.P , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.Q , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DP, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DQ, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.QP, 16, f ) ) != 0 )
    {
        // printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
        goto exit;
    }

    rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
    fclose( f );

    /*
     * Extract the RSA encrypted value from the text file
     */
    ret = 1;

    if( ( f = fopen( "result-enc.txt", "rb" ) ) == NULL )
    {
        // printf( "\n  ! Could not open %s\n\n", "result-enc.txt" );
        goto exit;
    }

    i = 0;

    while( fscanf( f, "%02X", &c ) > 0 &&
           i < (int) sizeof( buf ) )
        buf[i++] = (unsigned char) c;

    fclose( f );

    if( i != rsa.len )
    {
        // printf( "\n  ! Invalid RSA signature format\n\n" );
        goto exit;
    }

    /*
     * Decrypt the encrypted RSA data and print the result.
     */
    
    if( ( ret = rsa_pkcs1_decrypt( &rsa, RSA_PRIVATE, &i, buf, result, 1024 ) ) != 0 )
    {
        //printf( " failed\n  ! rsa_pkcs1_decrypt returned %d\n\n", ret );
        goto exit;
    }

    txt.append((const char*)result);
  
    ret = 0;

exit:

   return( ret );
}

// decode cypher text into a string and return it
int Security::RSAdecode(const char* cyphertxt, size_t bytecount, string& txt)
{
    FILE *f;
    int ret, c;
    size_t i;
    rsa_context rsa;
    unsigned char result[1024];
    unsigned char buf[512];

    DecodedText = "";
    DecodedByteCount = 0;

    memset(result, 0, sizeof( result ) );
    ret = 1;

    // printf( "\n  . Reading private key from rsa_priv.txt" );

    if( ( f = fopen( "rsa_priv.txt", "rb" ) ) == NULL )
    {
        // Could not open rsa_priv.txt\n" \\ Please run rsa_genkey first\n\n" );
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = mpi_read_file( &rsa.N , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.E , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.D , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.P , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.Q , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DP, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DQ, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.QP, 16, f ) ) != 0 )
    {
        // printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
        goto exit;
    }

    rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
    fclose( f );


    i = 0;
    while( i < bytecount && i < (int) sizeof( buf ) )
        buf[i++] = (unsigned char) cyphertxt[i];

    if( i != rsa.len )
    {
        // printf( "\n  ! Invalid RSA signature format\n\n" );
        goto exit;
    }

    /*
     * Decrypt the encrypted RSA data and print the result.
     */

    if( ( ret = rsa_pkcs1_decrypt( &rsa, RSA_PRIVATE, &i, buf, result, 1024 ) ) != 0 )
    {
        //printf( " failed\n  ! rsa_pkcs1_decrypt returned %d\n\n", ret );
        goto exit;
    }

    txt.append((const char*)result);
    DecodedText = txt;
    DecodedByteCount = txt.size();
    ret = 0;

exit:

   return( ret );
}

// decode cypher text into a byte array
string Security::RSAdecodeString(string CypherText){
    int ret;
    int i;
    size_t sz = 0;
    char CyChs[MAXENCRYPTEDLEN+10];  // The ASCII Hex encrypted data

    i=0;
    while (i < CypherText.size()){
        CyChs[i/2] = AsciiHexToChar(CypherText[i]) * 16;  // top nibble
        //cout << "&" << CypherText[i] << "=" << (int)CyChs[i/2] << "  ";
        //cout << "Dec:" << (char)CypherText[i];
        i++;
        CyChs[i/2] = CyChs[i/2] + AsciiHexToChar(CypherText[i]);  // bottom nibble
        //cout << (char)CypherText[i] << "(" << (int)CyChs[i/2] << ")" << "{" << (int)EncodedBytes[i] << "} ";
        i++;
    }
    sz = (size_t)i/2;

    cout << endl << "Encoded:" << EncodedCount << " bytes" << endl;
    for (i=0; i< EncodedCount; i++){
      if (EncodedBytes[i] != CyChs[i])
          cout << "Er:" << i << " ";
    }
    cout << endl;
    
    ret = RSAdecode(CyChs, sz);

    cout << "Returned:" << ret << endl;
    cout << "Text:" << DecodedText << endl;


    return DecodedText;
}


// decode cypher text into a byte array
int Security::RSAdecode(const char* cyphertxt, size_t bytecount)
{
    FILE *f;
    int ret, c;
    size_t i;
    rsa_context rsa;
    unsigned char result[1024];
    unsigned char buf[512];
    char ch;

    DecodedText = "";
    DecodedByteCount = 0;

    memset(result, 0, sizeof( result ) );
    ret = 1;

    // printf( "\n  . Reading private key from rsa_priv.txt" );

    if( ( f = fopen( "rsa_priv.txt", "rb" ) ) == NULL )
    {
        // Could not open rsa_priv.txt\n" \\ Please run rsa_genkey first\n\n" );
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = mpi_read_file( &rsa.N , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.E , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.D , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.P , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.Q , 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DP, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.DQ, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.QP, 16, f ) ) != 0 )
    {
        // printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
        goto exit;
    }

    rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
    fclose( f );


    i = 0;
    while( i < bytecount && i < (int) sizeof( buf ) )
        buf[i++] = (unsigned char) cyphertxt[i];

    if( i != rsa.len )
    {
        // printf( "\n  ! Invalid RSA signature format\n\n" );
        goto exit;
    }

    /*
    * Decrypt the encrypted RSA data and print the result.
    */
    if( ( ret = rsa_pkcs1_decrypt( &rsa, RSA_PRIVATE, &i, buf, result, 1024 ) ) != 0 )
    {
        //printf( " failed\n  ! rsa_pkcs1_decrypt returned %d\n\n", ret );
        goto exit;
    }

    i=0;
    while( result[i] != NULL && i < (int) sizeof( DecodedBytes) ){
        DecodedBytes[i] = result[i];
        ch = result[i];
        DecodedText.append(&ch,(size_t)1);
        i++;
    }
    DecodedBytes[i] = NULL;
    DecodedByteCount = i;
    ret = 0;

exit:

   return( ret );
}


// RSAkeygen() must be called before doing this test
int Security::TestSecurity(int msglevel){

  #define TESTMESSAGE  "Raveon Test 123"
   char* CC = TESTMESSAGE ;
   char enc[10000];
   unsigned int thecount = 0;
   size_t ccount;
   string ClearText = "";
   string Stt = TESTMESSAGE ;
   string Cyph;
   //RSAkeygen();
   // RSAkeyGen();
   if (msglevel > 0)
        cout << "Pub Key ( " <<  MyPublicKey.size() << ") :" << MyPublicKey << endl;

   //RSAtestencrypt(CC);
   Cyph = RSAencrypt(Stt);
   //SyncSecurity.RSAencrypt(CC, enc, &thecount);
   if (msglevel > 0)
        cout << "Cypher data byte count =" <<  EncodedCount << endl;
   if (msglevel > 0)
        cout << "Cypher message: " << Cyph << endl;

   ccount = (size_t)thecount;
   //SyncSecurity.RSAdecode(enc, ccount, ClearText);
   // ClearText = SyncSecurity.RSAdecode(enc, ccount);
   ClearText = RSAdecodeString(Cyph);
   if (ClearText == TESTMESSAGE)
        return 0;
   else
        return 1;

   if (msglevel > 0)
       cout << "Decoded (" <<  DecodedByteCount << ") :" << ClearText << endl;

}
