/* 
 * File:   POCSAGEncoder.h
 * Author: Adam Hickerson
 *
 * Created on May 23, 2013, 4:02 PM
 */

using namespace std;

#include "BinaryEntry.h"
#include "PagerTable.h"
#include <string>

#ifndef POCSAGENCODER_H
#define	POCSAGENCODER_H

#define POCSAG_DATA_BIT_PER_WORD 20
#define POCSAG_BITS_PER_ASCII 7                 
#define POCSAG_BITS_PER_NUMERIC 4
#define POCSAG_PREAMBLE_LENGTH 72
#define POCSAG_MAX_BATCH_PER_TX 8
#define POCSAG_WORD_PER_BATCH 17

#define POCSAG_NUMERIC_URGENCY_INDICATOR 0x0B
#define POCSAG_NUMERIC_SPACE_CHARACTER 0x0C
#define POCSAG_NUMERIC_HYPHEN_CHARACTER 0x0D
#define POCSAG_NUMERIC_CLOSE_PARENTHESIS_CHARACTER 0x0E
#define POCSAG_NUMERIC_OPEN_PARENTHESIS_CHARACTER 0x0F

#define POCSAG_IDLE_CODEWORD 0x7A89C197
#define POCSAG_SYNCH_CODEWORD 0x7CD215D8

namespace POCSAGEncoder {
    int encode(PagerTableEntry pager, string message, char *destBuffer, int maxBufferSize);
    bool append(PagerTableEntry pager, string message, char *destBuffer, int *byteCount, int maxBufferSize);
    void invert(char *buffer, int byteCount);
};

#endif	/* POCSAGENCODER_H */

