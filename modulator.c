/********************************************************/
// modulator.c
// John Sonnenberg   Aug 21, 2011
//
/********************************************************/
//#include "modulator.h"


#define COSsize  17
// Raised-cosine look-up table.  17 entires are 0-180 degrees.  scalled 0-1 is 0-512
//                            0     1    2   3     4    5   6     7    8    9   10   11     12    13    14    15    16
signed short raisedsine[] = {512, 502, 473, 425, 362, 284, 195, 100, 0, -100, -195, -284, -362, -425, -473, -502, -512};


// ModulateToTable_TwoLevel
// Encodes into a 2-level modulation buffer a sequence of data bytes held in an array of bytes
// Inputs:
//      DataBytes     Pointer to the input data that will be used to create the modulation table
//      ByteCount     Number of bytes to create modulation for
//      MaxModLength  Maximum number of words that the ModSequence array of unsigned shorts cab hold
//      SampleRate    The number of modulation samples/bit
// Output
//      Fills the array of unsigned short words pointed to by ModSequence with the modulation data.
//      The least-significant bit of each byte is loaded into the modulator first.
// return the number of words encoded into the ModSequence.  0=no encoding.  -1= error and cannot encode for some reason

int ModSequence( const char* DataBytes, int ByteCount, unsigned short* ModSequence,
                              const int MaxModLength, const int SampleRate )
{


    char* BytePointer = (char*)DataBytes;              // Pointer that will walk through the array of bytes as we modulate
    unsigned short* ModPointer =  ModSequence;  // Pointer that will walk through the array of short words as we compute modulation levels.
    int ModCount = 0;                           // The number of entires we put into the ModSequence array
    int bitcount = 0;                           // counts 0-7 for each bit in the byte
    int subsample = 0;                          // counts 0-(n-1) for each sub-sample in the bit.
    int LastBit = 0;                            // The binary value of the last bit we encoded
    int x;
    unsigned char currentbyte;                  // The byte in that we are currently trying to compute the modulation for
    unsigned char CurrentBit;                   // the bit value in the byte we are currently computing the
    signed short ModLevel;                      // the modulation level for a particular sub-bit computation
    int  CosIndex;                              // the index into the rasied cosing table

    while ((ByteCount > 0) && (ModCount < MaxModLength)){
        // Loop here each byte we will encode
        currentbyte = *BytePointer;             // get the byte to encode
        bitcount = 0;                           // least significant bit first
        while (bitcount < 8){
            // Loop here heach bit we will encode
            x = SampleRate;
            // Is the bit we are encoding a 1 or a zero?
            if ((currentbyte & (2^bitcount) > 0))
                CurrentBit = 1;
            else
                CurrentBit = 0;

            x = 1;
            while (x <= SampleRate){
                // loop here each entry in the modulation table
                if (CurrentBit == LastBit){
                    // No change in level, so keep it fixed at the same level for the whole bit. 
                    ModLevel = raisedsine[COSsize];    // The final mod level is always the last entry in the cosine table.
                    if (CurrentBit == 1)
                        ModLevel = ModLevel * -1;
                }else{
                    // We are transistioning between two different levels
                    CosIndex =((COSsize-1)/SampleRate) * x + 1;
                    ModLevel = raisedsine[CosIndex] ;
                    if (CurrentBit == 1)
                        ModLevel = ModLevel * -1;
                }
                *ModSequence = ModLevel;
                // Point to the next entry in the modulation table
                ModSequence++;
                ModCount++;
                if (ModCount >= MaxModLength)
                    return -1;  // ran out of room in the modulation buffer to build up the mod table.
                x++;  // next sub-sample
            }

            bitcount ++;   // next bit
        }

        BytePointer++;
        ByteCount--;
        
    };
    return ModCount;
}


int  TestModulator(void){

    char  d[100];
    unsigned short s[1000];
    int x;

    x = ModSequence(d, 10, s, 1000, 4);

    return 3;

}

char testme(int u){
    return 4;
}
