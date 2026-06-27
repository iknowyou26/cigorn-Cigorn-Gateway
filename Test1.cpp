
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

using namespace std;


int RSAtestencrypt( char* argv)
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

    cout << "Testing RSA" << endl;

    printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
                               (unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        printf( " failed\n  ! ctr_drbg_init returned %d\n", ret );
        goto exit;
    }

    printf( "\n  . Reading public key from rsa_pub.txt" );
    fflush( stdout );

    if( ( f = fopen( "rsa_pub.txt", "rb" ) ) == NULL )
    {
        ret = 1;
        printf( " failed\n  ! Could not open rsa_pub.txt\n" \
                "  ! Please run rsa_genkey first\n\n" );
        goto exit;
    }

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = mpi_read_file( &rsa.N, 16, f ) ) != 0 ||
        ( ret = mpi_read_file( &rsa.E, 16, f ) ) != 0 )
    {
        printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
        goto exit;
    }

    rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;

    fclose( f );

    if( strlen( argv ) > 100 )
    {
        printf( " Input data larger than 100 characters.\n\n" );
        goto exit;
    }

    memcpy( input, argv, strlen( argv ) );

    /*
     * Calculate the RSA encryption of the hash.
     */
    printf( "\n  . Generating the RSA encrypted value" );
    fflush( stdout );

    if( ( ret = rsa_pkcs1_encrypt( &rsa, ctr_drbg_random, &ctr_drbg,
                                   RSA_PUBLIC, strlen( argv ),
                                   input, buf ) ) != 0 )
    {
        printf( " failed\n  ! rsa_pkcs1_encrypt returned %d\n\n", ret );
        goto exit;
    }

    /*
     * Write the signature into result-enc.txt
     */
    if( ( f = fopen( "result-enc.txt", "wb+" ) ) == NULL )
    {
        ret = 1;
        printf( " failed\n  ! Could not create %s\n\n", "result-enc.txt" );
        goto exit;
    }

    for( i = 0; i < rsa.len; i++ )
        fprintf( f, "%02X%s", buf[i],
                 ( i + 1 ) % 16 == 0 ? "\r\n" : " " );

    fclose( f );

    printf( "\n  . Done (created \"%s\")\n\n", "result-enc.txt" );

exit:

#if defined(_WIN32)
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

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

#if !defined(POLARSSL_BIGNUM_C) || !defined(POLARSSL_ENTROPY_C) ||   \
    !defined(POLARSSL_RSA_C) || !defined(POLARSSL_GENPRIME) ||      \
    !defined(POLARSSL_FS_IO) || !defined(POLARSSL_CTR_DRBG_C)
int main( int argc, char *argv[] )
{
    ((void) argc);
    ((void) argv);

    printf("POLARSSL_BIGNUM_C and/or POLARSSL_ENTROPY_C and/or "
           "POLARSSL_RSA_C and/or POLARSSL_GENPRIME and/or "
           "POLARSSL_FS_IO and/or POLARSSL_CTR_DRBG_C not defined.\n");
    return( 0 );
}
#else
int RSAkeygen( void)
{
    int ret;
    rsa_context rsa;
    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    FILE *fpub  = NULL;
    FILE *fpriv = NULL;
    char *pers = "rsa_genkey";

    printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
                               (unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        printf( " failed\n  ! ctr_drbg_init returned %d\n", ret );
        goto exit;
    }

    printf( " ok\n  . Generating the RSA key [ %d-bit ]...", KEY_SIZE );
    fflush( stdout );

    rsa_init( &rsa, RSA_PKCS_V15, 0 );

    if( ( ret = rsa_gen_key( &rsa, ctr_drbg_random, &ctr_drbg, KEY_SIZE,
                             EXPONENT ) ) != 0 )
    {
        printf( " failed\n  ! rsa_gen_key returned %d\n\n", ret );
        goto exit;
    }

    printf( " ok\n  . Exporting the public  key in rsa_pub.txt...." );
    fflush( stdout );

    if( ( fpub = fopen( "rsa_pub.txt", "wb+" ) ) == NULL )
    {
        printf( " failed\n  ! could not open rsa_pub.txt for writing\n\n" );
        ret = 1;
        goto exit;
    }

    if( ( ret = mpi_write_file( "N = ", &rsa.N, 16, fpub ) ) != 0 ||
        ( ret = mpi_write_file( "E = ", &rsa.E, 16, fpub ) ) != 0 )
    {
        printf( " failed\n  ! mpi_write_file returned %d\n\n", ret );
        goto exit;
    }

    printf( " ok\n  . Exporting the private key in rsa_priv.txt..." );
    fflush( stdout );

    if( ( fpriv = fopen( "rsa_priv.txt", "wb+" ) ) == NULL )
    {
        printf( " failed\n  ! could not open rsa_priv.txt for writing\n" );
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
        printf( " failed\n  ! mpi_write_file returned %d\n\n", ret );
        goto exit;
    }
/*
    printf( " ok\n  . Generating the certificate..." );

    x509write_init_raw( &cert );
    x509write_add_pubkey( &cert, &rsa );
    x509write_add_subject( &cert, "CN='localhost'" );
    x509write_add_validity( &cert, "2007-09-06 17:00:32",
                                   "2010-09-06 17:00:32" );
    x509write_create_selfsign( &cert, &rsa );
    x509write_crtfile( &cert, "cert.der", X509_OUTPUT_DER );
    x509write_crtfile( &cert, "cert.pem", X509_OUTPUT_PEM );
    x509write_free_raw( &cert );
*/
    printf( " ok\n\n" );

exit:

    if( fpub  != NULL )
        fclose( fpub );

    if( fpriv != NULL )
        fclose( fpriv );

    rsa_free( &rsa );

#if defined(_WIN32)
    printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
#endif /* POLARSSL_BIGNUM_C && POLARSSL_ENTROPY_C && POLARSSL_RSA_C &&
          POLARSSL_GENPRIME && POLARSSL_FS_IO && POLARSSL_CTR_DRBG_C */


int RSAdecodetest(void )
{
    FILE *f;
    int ret, c;
    size_t i;
    rsa_context rsa;
    unsigned char result[1024];
    unsigned char buf[512];

    memset(result, 0, sizeof( result ) );
    ret = 1;

    printf( "\n  . Reading private key from rsa_priv.txt" );
    fflush( stdout );

    if( ( f = fopen( "rsa_priv.txt", "rb" ) ) == NULL )
    {
        printf( " failed\n  ! Could not open rsa_priv.txt\n" \
                "  ! Please run rsa_genkey first\n\n" );
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
        printf( " failed\n  ! mpi_read_file returned %d\n\n", ret );
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
        printf( "\n  ! Could not open %s\n\n", "result-enc.txt" );
        goto exit;
    }

    i = 0;

    while( fscanf( f, "%02X", &c ) > 0 &&
           i < (int) sizeof( buf ) )
        buf[i++] = (unsigned char) c;

    fclose( f );

    if( i != rsa.len )
    {
        printf( "\n  ! Invalid RSA signature format\n\n" );
        goto exit;
    }

    /*
     * Decrypt the encrypted RSA data and print the result.
     */
    printf( "\n  . Decrypting the encrypted data" );
    fflush( stdout );

    if( ( ret = rsa_pkcs1_decrypt( &rsa, RSA_PRIVATE, &i, buf, result,
                                   1024 ) ) != 0 )
    {
        printf( " failed\n  ! rsa_pkcs1_decrypt returned %d\n\n", ret );
        goto exit;
    }

    printf( "\n  . OK\n\n" );

    printf( "The decrypted result is: '%s'\n\n", result );

    ret = 0;

exit:

   return( ret );
}
