// demodulate.c
// John Sonnenberg   Aug 21, 2011
//
/********************************************************/
//#include "demodulate.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "demodulate.h"
#include "math.h"

using namespace std;

// Local functions
void RemoveDC(void);
void ComputeAverage(void);
void Correlate(void);
bool CheckSineQuality(void);            // return true if it is a SINE wave
int CheckNoiseLevel(void);
void CopyADCbuff(unsigned short* , unsigned char);
void DataSlicer(void);
bool CheckForSync(void);

unsigned short RawSamples[MaxRawSize];           // holds the raw unsigned A/D converter readings
signed short RxSamples[MaxRawSize];              // signed raw samples, centered around 0 with the DC component remoted.
signed int AutoCorr[MaxSamplesPerBit];           // The autocorrelation values of the samples
signed short AverageSampleVal[2];                // The calculated DC average value of all samples in two halves of the raw buffer.
signed short DCaverage = 0;                      // The average of the two AverageSamleVal values.  Very close to the true DC average.

// If different sample/rates are used, these three parameters must be redefined properly.
unsigned char RawSize = MaxRawSize;                 // For now, we use 8 bits, 8X oversample as default
unsigned char SamplesPerBit = MaxSamplesPerBit;     // For now, we use 8 bits, 8X oversample as default
unsigned char RawBits = MaxRawBits;                 // For now, we use 8 bits, 8X oversample as default

unsigned char RxDataShifter = 0;
unsigned char RawInputIndex;            // index pointing to the location that the next A/D will be stored.
unsigned char MyDemodeState = DEM_INIT; // Demodulator state machine state

unsigned char InputHalf = LowerHalf ;     // stores which half is currently being loaded

// Define Demodulation paramters
#define   ADJACENTDELTA   65              // % that the adjacent samples to the peak may be below the peak for a good SINE wave
#define   MAXQUADRATURE   40              // % that the quadrature sample can be relative to the peak.
#define   PREAMBLE        0xaa            // 1010101...
#define   MAXNOISE        35              // During preamble, the max eye closure
#define   SYNC            0x53            // sync word

// Information about the current samples being demodulated.
bool GoodSineWave = false;     // set true when the waveform being analyzed looks like a SINE wave
bool FoundSync = false;        // true when we detect it.
int ByteOffset = 0;             // the ofset into the adc buffer where the byte boundary is.
int MyNoiseLevel = 0;          // 0-100 for eye opening. 0 = no noise. 100 = 100% noise.
unsigned char PeakSample = 0;  // the sample that has the largest magnitude
int PeakIndex = 0;             // The index of the sub-sample that is the peak value (I sample location)
unsigned short AvePeakMag = 0; // The average of the absolute values of the I samples
bool InvertRx = false;         // set true if the modulation looks inverted
int BitsCheckedForSync = 0;    // count how many bits we check looking for preamble.


void DemodulateStatemachine(void){
    static unsigned char MyHalf = LowerHalf;
    static unsigned char StepState = 0;     // each demodulation state will have a few steps. Use this to step through them

    // RawInputIndex must be set before calling this.  It is the index
    // See which half of the raw buffer we are currently filling.
    if (RawInputIndex >= RawSize/2)
        InputHalf = UpperHalf ;     // we are filling the upper half of the ADC buffer from the A/D converter.
    else
        InputHalf = LowerHalf ;     // we are filling the Lower half of the ADC buffer from the A/D converter.

    // See if we can check the buffer half we want to.  If the DMA is loading it, we must wait till
    // the DMA is working on the other half.
    if (MyHalf == InputHalf){
         StepState = 0;
         return;
    }


    switch (MyDemodeState){
        case DEM_INIT:
            // one-time Initialize things

            MyDemodeState = DEM_LOCKING;  // start running.
            StepState = 0;
            break;
        case DEM_LOCKING:
            // Looking for preamble and trying to find a clock lock.
            // Analyze 1/2 the buffer at a time, while the other half is being loaded
            // Time now to do our demodulation of the samples
            switch (StepState){
                case 0:
                    // First normalize the raw over the the DC centered buffer.
                    CopyADCbuff(RawSamples, MyHalf);  // copy half the ADC buffer to my demodulation buffer
                    ComputeAverage();
                    InvertRx = false;                 // for now, assume its not inverted
                    BitsCheckedForSync = 0;           // restart counting
                    StepState++;  // always step through the step states
                    break;
                case 1:
                    // First normalize the raw over the the DC centered buffer.
                    RemoveDC();
                    StepState++;  // always step through the step states
                    break;
                case 2:
                    // Autocorrelate to detect a sine wave
                    Correlate();       // calculates the correlation values
                    StepState++;       // always step through the step states
                    break;
                case 3:
                    // See of the correlated data looks like a SINE wave
                    GoodSineWave = CheckSineQuality();   // sets IsGoodSinewave true/false
                    StepState++;  // always step through the step states
                    break;
                case 4:
                    // See of the correlated data looks like a SINE wave
                    MyNoiseLevel = CheckNoiseLevel();   // sets IsGoodSinewave true/false
                    StepState++;  // always step through the step states
                    break;
                case 5:
                    // Recover the 1s and zeros from the samples
                    StepState++;    // always step through the step states
                    DataSlicer();   // Slice the data in the buffer and put it in the shift register
                    break;
                case 6:
                    // Done with stepping through this MyDemodeState
                    if (MyHalf == 0)
                        MyHalf = 1;         // switch halves
                    else
                        MyHalf = 0;         // switch halves
                    // Done with this pass of the state machine.  
                    // Did we detect preamble???
                    if ((RxDataShifter == PREAMBLE) || (RxDataShifter == ~PREAMBLE) ){
                        if ((MyNoiseLevel < MAXNOISE ) && (GoodSineWave)){
                            MyDemodeState = DEM_HUNTING;   // good preamble, look for the sync word
                        }
                    }
                    StepState = 0;
                    break;
                default:
                    // Error.  Should never get here
                    StepState = 0;
                    break;
            }
           break;
        case DEM_HUNTING:
            // Hunting for the sync word
            switch (StepState){
                case 0:
                    // First normalize the raw over the the DC centered buffer.
                    CopyADCbuff(RawSamples, MyHalf);  // copy half the ADC buffer to my demodulation buffer
                    StepState++;  // always step through the step states
                    break;
                case 1:
                    // First normalize the raw over the the DC centered buffer.
                    RemoveDC();
                    StepState++;  // always step through the step states
                    break;
                case 2:
                    // See of the correlated data looks like a SINE wave
                    MyNoiseLevel = CheckNoiseLevel();   // sets IsGoodSinewave true/false
                    StepState++;  // always step through the step states
                    break;
                case 3:
                    // Recover the 1s and zeros from the samples
                    StepState++;    // always step through the step states
                    DataSlicer();   // Slice the data in the buffer and put it in the shift register
                    BitsCheckedForSync = BitsCheckedForSync + RawBits/2;
                    break;
                case 4:
                    StepState++;    // always step through the step states
                    FoundSync = CheckForSync(); // look at the demodulated data for a sync pattern
                    break;
                case 5:
                    // Done with stepping through this DEM_HUNTING
                    if (MyHalf == 0)
                        MyHalf = 1;         // switch halves
                    else
                        MyHalf = 0;         // switch halves
                    // Done with this pass of the state machine.
                    // Did we detect preamble???
                    if ((RxDataShifter == PREAMBLE) || (RxDataShifter == ~PREAMBLE) ){
                        if ( MyNoiseLevel < MAXNOISE ) {
                            MyDemodeState = DEM_SYNC_FOUND;   // good preamble, look for the sync word
                        }
                    }
                    StepState = 0;
                    break;
                default:
                    // Error.  Should never get here
                    StepState = 0;
                    break;
            }
            break;
        case DEM_SYNC_FOUND:
            // Found sync.  ready to start demodulating a packet.

            break;
        case DEM_DEMODUALTE:
            // Demodulating a data packet

            break;
        case DEM_NOISE_ABORT:
            // Too much noise, so demodulator gave up.

            break;
        case DEM_FORCED_ABORT:
            // Something forced us to stop demodulating using an abort flag. Demodulator gave up.

            break;
        case DEM_RESTART:
            break;

    }


}

//unsigned short rawsamples[MaxRawSize];           // holds the raw unsigned A/D converter readings
//signed short RxSamples[MaxRawSize];              // signed raw samples, centered around 0 with the DC component remoted.
//signed short AverageSampleVal[2];                // The calculated DC average value of all samples in two halves of the raw buffer.
//signed short DCaverage = 0;                      // The average of the two AverageSamleVal values.  Very close to the true DC average.


// return true once we detect sync pattern.
bool CheckForSync(void){
    static unsigned short sr = 0xaaaa;     // a shift register
    unsigned short x;
    unsigned short mask = 0xff00;
    unsigned short syncpattern = (SYNC << 8);
    int i;
    bool retval = false;
    int shiftoffset = 0;

    x = RxDataShifter;    // the most recent data we shifted in

    sr = sr >> 8;         // move 8 bits over.
    sr = sr & 0x00ff;
    sr = sr || (x << 8);  // put the most recent 8 bits into the 16 bit shift register
    for (i = 0; i< 8; i++){
        if ((sr & mask) == (syncpattern && mask)){
            // Found sync
            retval = true;
            InvertRx = false;
            shiftoffset = i;
        }
        if ((sr & mask) == (~syncpattern && mask)){
            // Found sync
            retval = true;
            InvertRx = true;
            shiftoffset = i;
        }
    }

    ByteOffset = shiftoffset;
    return retval;
}

void DataSlicer(void){
    // Slice the data and put it into the RxDataShifter.  We slice the lower-half only.
    int bitcount = 0;   // Count how many bits we shift in
    int databit;

    // Do it for each bit.  Compute the AvePeakMag Average Peak magnitude of the I samples
    bitcount = 0;
    while (bitcount < RawBits){
        if ((RxSamples[PeakIndex + bitcount * SamplesPerBit]) > 0 )
            databit = 0x80;
        else
            databit = 0;
        if (InvertRx)
            databit = databit ^ 0x80;  // invert the data because the sync word was inverted
        RxDataShifter = RxDataShifter >> 1;         // store the new data bit
        RxDataShifter = 0x7F & RxDataShifter;
        RxDataShifter = RxDataShifter + databit;    // store the new data bit
        bitcount++;  // next bit
    }

}

// Copy over 1/2 of the ADC buffer to my demodulation buffer
void CopyADCbuff(unsigned short* ADCbuff, unsigned char Half){

    // First shift down my buffer to make room for anyther bunch of ACD readings
    int i;
    int p;  // The index where we start copying ADC data from
    int x;  // The inxed of the RawSample buffer we are copying to

    for (i = 0; i < (RawSize/2); i++){
        if (i < MaxRawSize)
            RawSamples[i] = RawSamples[i + (RawSize/2)];  // move the buffer down by 1/2 the samples.  0 based.
    }

    // Now put in the new samples
    if (Half > 0)
        p = (RawSize/2);
    else
        p = 0;

    for (i = 0; i < (RawSize/2); i++){
        x = i + (RawSize/2);
        if (x < MaxRawSize)
            RawSamples[x] = ADCbuff[ i + p];  // append the new samples
    }

}

// See if the demodulated signal looks all noisey, or not.  
// 0-100 for eye opening. 0 = no noise. 100 = 100% noise.
int CheckNoiseLevel(void){
    int bitcount = 0;   // Count how many bits we correlate
    int i;
    signed short  AveEye=0;   // teh average eye opening
    int retval;

    // Range check
    if (PeakIndex >= SamplesPerBit)
        return 0;    // error. Don't crash

    // Which bit in the RxSamples array will we start with first?

    // Find the average peak first
    i = SamplesPerBit + PeakIndex;   // Make an index pointing to the middle of the first bit to demodulate
    // Do it for each bit.  Compute the AvePeakMag Average Peak magnitude of the I samples
    AvePeakMag = 0;
    bitcount = 0;
    while (bitcount < RawBits){
        AvePeakMag = AvePeakMag + abs(RxSamples[i + bitcount * SamplesPerBit]);
        bitcount++;  // next bit
    }

    AvePeakMag = AvePeakMag / bitcount;  // average it

    // Now find the eye opening for each bit
    bitcount = 0;
    while (bitcount < RawBits){
        AveEye = AveEye + abs(AvePeakMag - abs(RxSamples[i + bitcount * SamplesPerBit]));
        bitcount++;  // next bit
    }

    AveEye = AveEye / bitcount;  // everage it

    retval = AveEye * 100 / AvePeakMag;   // Eye opening in %
    if (retval < 0)
        retval = 0;
    if (retval>100)
        retval = 100;
    return retval;

}


// Check the correlation, and see if this looks like a sine wave,
bool CheckSineQuality(void){

    int i;
    int x;

    bool IsGoodSinewave = true;    // set true when the waveform being analyzed looks like a SINE wave
    signed short PeakValue = 0;

    // Find the biggest sample point in the correlator.
    for(i = 0; i < SamplesPerBit; i++){
         if(abs(AutoCorr[i]) > PeakValue){
             PeakValue = abs(AutoCorr[i]);
             PeakIndex = i;
         }
     }

     // Now see if the adjacent samples are fairly large (within 60%)
     
     i = PeakIndex;   // point at the biggest sample
     i--;             // go to the previous sample
     if (i<0)
         i = (SamplesPerBit -1);  // wrap around
     x = abs(AutoCorr[i]);  // get the prior sample to the peak

     i = PeakIndex;   // point at the biggest sample
     i++;             // go to the next sample
     if (i >= SamplesPerBit)
         i = 0;  // wrap around
     x = (x + abs(AutoCorr[i]))/2;  // add the sample after the peak to the prior sample, and average.

     if (x <  (PeakValue * ADJACENTDELTA)/100)
        IsGoodSinewave = false;   // If adjacent samples were < about 75% of the peak, this is not a sine wave.

     // Now see if the quadrature samples (1/2 bit way) are very small. They should be near zero.
     i = PeakIndex + SamplesPerBit/2;   // p;oint 1/2 bit over
     if (i >= SamplesPerBit)
         i = i - SamplesPerBit;  // wrap around
     x = abs(AutoCorr[i]);       // get the sample 1/2 bit away from the peak

     if (x > ((PeakValue * MAXQUADRATURE)/100))
        IsGoodSinewave = false;     // If quadrature sample is > than about 25%, this is not a sine wave.


     cout << "Peak = " << PeakIndex << " = " << PeakValue << endl;
     cout << "Is good preamble = " << IsGoodSinewave << endl;

     return IsGoodSinewave;   // return true if this looks sorta like a SINE wave
}

void ComputeAverage(){
    // Compute the average ADC value for the raw buffer
    int i;
    int total = 0;
    
    for(i = 0; i < RawSize; i++){
       total = RawSamples[i] + total;
    }
    DCaverage = total / i;

}

//AutoCorr[]
void Correlate(void){
    // Compute the correlation betweent he over-samples
    int i;
    int i1,i2;          // indexs for the two values we'll check
    int s1 = 0;
    int s2 = 0;
    int bit = 0;        // which bit are we checking
    int bitcount = 0;   // Count how many bits we correlate

    // Do it for each bit
    while (bitcount < (RawBits - 1)){
        // loop through each sample in a bit, and correlate it with the corresponding sample in the next bit
        for(i = 0; i < SamplesPerBit; i++){
            i1 = i + bit * SamplesPerBit;           // Index for sample #1
            s1 = RxSamples[i1];
            i2 = i + (bit + 1) * SamplesPerBit;     // index for sampe #2, one bit over
            s2 = RxSamples[i2];
            if (bitcount == 0){
                // First time through
                AutoCorr[i] = (s1 + (-1 * s2)) / 2;   // add two and take their magnitude
            }else
                AutoCorr[i] = (AutoCorr[i] + (s1 + (-1 * s2)/2))/2;   // add two and take their magnitude
        }
        bitcount++;
        bit++;
    }


}

void RemoveDC(void){
    // Copy the RAW over to the RxSamples, and remove the DC bias.
    int i;
    int index;

    for(i = 0; i < RawSize; i++){
        if (i < MaxRawSize)
            RxSamples[i] = RawSamples[i] - DCaverage;
    }

}
