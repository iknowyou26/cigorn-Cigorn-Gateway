/* 
 * File:   POCSAGEncoder.cpp
 * Author: root
 * 
 * Created on May 23, 2013, 4:02 PM
 */

#include "POCSAGEncoder.h"
#include "BCHEncoder.h"
#include <stdint.h>
#include <bitset>


namespace {
    /**
     * Builds a frame without preamble.
     * @param pager
     * @param message
     * @param buffer Buffer to put the frame in. The buffer will be assumed to be 
     * @param bufferSize Maximum size of the buffer
     * @param startIndex The index into buffer in which the first codeword (address) was placed
     * @param endIndex The index into buffer in which the final codeword was placed.
     * @return Whether the frame was successfully fit in the buffer
     */
    bool buildFrame(PagerTableEntry pager, string message, uint32_t *buffer, int bufferSize, int *startIndex, int *endIndex){
        // Where in the message the address appears changes based on the address
        int capCode = pager.capCode;

        // Calculate the start index. Where the message starts is based on the cap code
        int startFrame = capCode%8;
        *startIndex = startFrame * 2 + 1; // +1 - there will always be a synch word on the front

        // Calculate the end index. This can be slightly tricky since ASCII characters may be split
        int bitCount;
        if(pager.pageDataType == "Alpha"){
            bitCount = message.length() * POCSAG_BITS_PER_ASCII;
        }else{ // Numeric
            bitCount = message.length() * POCSAG_BITS_PER_NUMERIC;
        }
        int wordCount = bitCount / POCSAG_DATA_BIT_PER_WORD;
        // Correct for truncating
        if(bitCount % POCSAG_DATA_BIT_PER_WORD != 0){
            wordCount++;
        }
        *endIndex = *startIndex + wordCount; // Don't subtract 1 since we insert the address at the start index 
        // Now we have to add any synch words that may fall in the middle
        for(int i = *startIndex; i <= *endIndex; i++){
            if(i % POCSAG_WORD_PER_BATCH == 0){
                // This is a synch word
                (*endIndex)++;
            }
        }

        // So, does it fit?
        if(*endIndex >= bufferSize){
            // Doesn't fit
            return false;
        }

        // Fill the buffer with idle codewords to start
        for(int i = 0; i < bufferSize; i++){
            buffer[i] = POCSAG_IDLE_CODEWORD;
        }

        // Place synch words at the start of every batch
        for(int i = 0; i < bufferSize; i+=POCSAG_WORD_PER_BATCH){
            buffer[i] = POCSAG_SYNCH_CODEWORD;
        }

        // Start placing codewords
        int currentCodeWord = *startIndex;
        CBCHEncoder m_bch;

        // First the address
        int encodedAddress = capCode >> 3;
        encodedAddress = encodedAddress<<2;
        // Code is 11 for alpha, 00 for numeric
        if(pager.pageDataType == "Alpha"){
            encodedAddress |= 0x3;
        }else{
            encodedAddress &= ~(0x03);
        }
        encodedAddress = encodedAddress<<11;
        m_bch.SetData(encodedAddress);
        m_bch.Encode();
        buffer[currentCodeWord] = m_bch.GetEncodedData();
        currentCodeWord++;

        // Now data
        int messageLen = message.length();
        int messageBitLen = 0;
        bitset<POCSAG_MAX_BATCH_PER_TX * POCSAG_WORD_PER_BATCH * POCSAG_DATA_BIT_PER_WORD> messageBits;

        // Pack messageBits with message
        for(int i = 0; i < messageLen; i++){
            char c = message[i];

            if(pager.pageDataType == "Alpha"){
                for(int j = 0; j < POCSAG_BITS_PER_ASCII; j++){
                    messageBits[i*POCSAG_BITS_PER_ASCII + j] = (c >> j) & 0x01; // Bits go in in reverse order (LSB first)
                    messageBitLen++;
                }
                if(i == messageLen - 1){
                    // We need to fill anything unused with nulls
                    int loc = (i+1)*POCSAG_BITS_PER_ASCII;

                    while(messageBitLen % POCSAG_DATA_BIT_PER_WORD != 0 && loc < i + POCSAG_DATA_BIT_PER_WORD){
                        messageBits[loc] = 0;
                        loc++;
                    }
                }
            }else{
                int value;
                
                if(c == 'U'){
                    value = POCSAG_NUMERIC_URGENCY_INDICATOR;
                }else if(c == '-'){
                    value = POCSAG_NUMERIC_HYPHEN_CHARACTER;
                }else if(c == ')'){
                    value = POCSAG_NUMERIC_CLOSE_PARENTHESIS_CHARACTER;
                }else if(c == '('){
                    value = POCSAG_NUMERIC_OPEN_PARENTHESIS_CHARACTER;
                }else if(c == ' ' || c < '0' || c > '9'){
                    // Anything invalid gets a space
                    value = POCSAG_NUMERIC_SPACE_CHARACTER;
                }else{
                    value = c - 48;
                }
                
                for(int j = 0; j < POCSAG_BITS_PER_NUMERIC; j++){
                    messageBits[i*POCSAG_BITS_PER_NUMERIC + j] = (value >> j) & 0x01; // Bits go in in reverse order (LSB first)
                    messageBitLen++;
                }

                if(i == messageLen - 1){
                    // We need to fill anything unused with spaces
                    int loc = i + 1; // starting with the next character

                    while(messageBitLen % POCSAG_DATA_BIT_PER_WORD != 0 && loc < i + 10){
                        for(int j = 0; j < POCSAG_BITS_PER_NUMERIC; j++){
                            messageBits[loc*POCSAG_BITS_PER_NUMERIC + j] = (POCSAG_NUMERIC_SPACE_CHARACTER >> j) & 0x01; // Bits go in in reverse order (LSB first)
                            messageBitLen++;
                        }
                        loc++;
                    }
                }
            }
        }

        // Now pull out POCSAG_DATA_BIT_PER_FRAME bits at a time, encode and place in message
        for(int i = 0; i < messageBitLen; i += POCSAG_DATA_BIT_PER_WORD){
            int codeWord = 1 << 31; // Indicates data

            for(int j = 0; j < POCSAG_DATA_BIT_PER_WORD; j++){
                codeWord |= messageBits[i+j]<<(30-j);
            }

            m_bch.SetData(codeWord);
            m_bch.Encode();
            codeWord = m_bch.GetEncodedData();

            if(currentCodeWord % POCSAG_WORD_PER_BATCH == 0){
                // Skip sync words
                currentCodeWord++;
            }

            buffer[currentCodeWord] = codeWord;
            currentCodeWord++;
        }

        return true;
    }
    
    /**
     * Returns the int that is packed into 4 bytes starting at bytes
     * @param bytes First byte, the most significant byte
     * @return 
     */
    int getIntFromBytes(char *bytes){
        int value = 0;
        value |= (((int) bytes[0]) << 24) & 0xFF000000;
        value |= (((int) bytes[1]) << 16) & 0x00FF0000;
        value |= (((int) bytes[2]) <<  8) & 0x0000FF00;
        value |= (((int) bytes[3])      ) & 0x000000FF;
        value = ~value; // Bits on the air are inverted
        return value;
    }
}

namespace POCSAGEncoder {
    /**
     * Encodes a page into a raw POCSAG page including pre-amble.
     * @param pager A PagerTableEntry which specifies CAPCODE, data type, etc.
     * @param message The ASCII formatted message to send to the pager
     * @param destBuffer A pointer to a byte buffer in which to put the data
     * @param maxBufferSize Maximum size the page can grow to.
     * @return Number of bytes actually used
     */
    int encode(PagerTableEntry pager, string message, char *destBuffer, int maxBufferSize){
        if(pager.capCode < 0){
            // Invalid.
            return 0;
        }
        
        // TODO: check if characters are valid
        uint32_t pocsagMessage[POCSAG_WORD_PER_BATCH*POCSAG_MAX_BATCH_PER_TX]; // Words, represented as ints 
        
        int startIndex = 0, endIndex = 0;
        buildFrame(pager, message, pocsagMessage, POCSAG_WORD_PER_BATCH*POCSAG_MAX_BATCH_PER_TX, &startIndex, &endIndex);

        // Round the end index up to the nearest whole frame
        if(endIndex % POCSAG_WORD_PER_BATCH != 0){
            endIndex += POCSAG_WORD_PER_BATCH - (endIndex % POCSAG_WORD_PER_BATCH);
        }
        
        // Preamble
        for(int i = 0; i < POCSAG_PREAMBLE_LENGTH && i < maxBufferSize; i++){
            destBuffer[i] = ~(0xAA); // POCSAG uses reversed bits on the air
        }

        // Message
        int currentCodeWord = 0;
        for(int i = POCSAG_PREAMBLE_LENGTH; i < POCSAG_PREAMBLE_LENGTH + (endIndex*4) && i < maxBufferSize; i+=4){ // 4 byte chunks
            // Bitwise not, POCSAG is reversed on the air
            destBuffer[i]   = ~((pocsagMessage[currentCodeWord] >> 24) & 0xFF);
            destBuffer[i+1] = ~((pocsagMessage[currentCodeWord] >> 16) & 0xFF);
            destBuffer[i+2] = ~((pocsagMessage[currentCodeWord] >>  8) & 0xFF);
            destBuffer[i+3] = ~((pocsagMessage[currentCodeWord]      ) & 0xFF);
            currentCodeWord++;
        }
        
        return POCSAG_PREAMBLE_LENGTH + currentCodeWord * 4;
    }
    
    /**
     * Appends a page on the end of an existing raw POCSAG page. The page should
     * already have pre-amble on the front
     * @param pager
     * @param message
     * @param destBuffer
     * @param byteCount A reference which will contain the new byte count
     * @param maxBufferSize
     * @return Whether the message fit
     */
    bool append(PagerTableEntry pager, string message, char *destBuffer, int *byteCount, int maxBufferSize){
        // Locate the first available byte
        int firstAvailableByte = *byteCount;
        int checkByte;
        for(checkByte = *byteCount - 4; checkByte >= 0; checkByte -= 4){
            if(getIntFromBytes(&(destBuffer[checkByte])) == POCSAG_SYNCH_CODEWORD
                    || getIntFromBytes(&(destBuffer[checkByte])) == POCSAG_IDLE_CODEWORD){
                // This is a valid place to place a page
                firstAvailableByte = checkByte;
            }else{
                // Found data
                break;
            }
        }
        
        // Build this messages frame
        uint32_t pocsagMessage[POCSAG_WORD_PER_BATCH*POCSAG_MAX_BATCH_PER_TX]; // Words, represented as ints 
        int startIndex = 0, endIndex = 0;
        buildFrame(pager, message, pocsagMessage, POCSAG_WORD_PER_BATCH*POCSAG_MAX_BATCH_PER_TX, &startIndex, &endIndex);
        
        // Round the end index up to the nearest whole batch
        if(endIndex % POCSAG_WORD_PER_BATCH != 0){
            endIndex += POCSAG_WORD_PER_BATCH - (endIndex % POCSAG_WORD_PER_BATCH);
        }
        
        // Find where we're going to stick this- it may squeeze in to the current batch or need to go in the next one
        int byteStart = *byteCount - (POCSAG_WORD_PER_BATCH * 4) + startIndex * 4;
        int copyStartIndex;
        int newByteCount;
        if(startIndex == 1 && getIntFromBytes(&(destBuffer[*byteCount - 4])) != POCSAG_IDLE_CODEWORD){
            // Some paging receivers don't like to see <data><sync><new address>. We're just going to stop at this point and send a new pre-amble
            return false;
        }else if(byteStart <= firstAvailableByte){ // Some paging receivers can't handle an address immediately following data, so we leave a minimum gap of 1
            // We're going to start a new frame
            byteStart = *byteCount;
            copyStartIndex = 0;
            newByteCount =  *byteCount + (endIndex - copyStartIndex) * 4;
        }else{
            // We're continuing the current batch (although we may still add one depending on lengths
            copyStartIndex = startIndex; 
            newByteCount = *byteCount;
            if(endIndex > POCSAG_WORD_PER_BATCH){
                // Adding additional batches due to message overrun
                newByteCount += POCSAG_WORD_PER_BATCH * ((endIndex / POCSAG_WORD_PER_BATCH) - 1) * 4;
            }
        }
        
        // Make sure it will fit
        if(newByteCount >= maxBufferSize){
            // Leave the buffer as is
            return false;
        }
        
        // We're committed. Copy in
        *byteCount = newByteCount; // Safe now to set the new byte count
        int currentCodeWord = copyStartIndex;
        for(int i = byteStart; i < byteStart + (endIndex - copyStartIndex) * 4; i+=4){ // 4 byte chunks
            // Bitwise not, POCSAG is reversed on the air
            destBuffer[i]   = ~((pocsagMessage[currentCodeWord] >> 24) & 0xFF);
            destBuffer[i+1] = ~((pocsagMessage[currentCodeWord] >> 16) & 0xFF);
            destBuffer[i+2] = ~((pocsagMessage[currentCodeWord] >>  8) & 0xFF);
            destBuffer[i+3] = ~((pocsagMessage[currentCodeWord]      ) & 0xFF);
            currentCodeWord++;
        }
        
        return true;
    }
    
    void invert(char *buffer, int byteCount){
        for(int i = 0; i < byteCount; i++){
            buffer[i] = ~buffer[i];
        }
    }
}

