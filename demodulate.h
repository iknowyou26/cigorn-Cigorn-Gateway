/* 
 * File:   demodulate.h
 * Author: john
 *
 * Created on August 27, 2011, 12:05 PM
 */

#ifndef DEMODULATE_H
#define	DEMODULATE_H

#define MaxSamplesPerBit        8      // We'll do 8X oversampling.
#define MaxRawBits              8      // There will be a max of 8 bits in the raw sample buffer;
#define MaxRawSize  MaxRawBits * MaxSamplesPerBit
#define LowerHalf           0
#define UpperHalf           1


// This structure currenlty not used....
enum DemodState{
     DEM_INIT         = 0,   // Initialize the state machine and demodulator logic
     DEM_LOCKING      = 1,   // Looking for preamble and trying to get a clock lock.
     DEM_HUNTING      = 2,   // Found premable. Hunting for sync word.
     DEM_SYNC_FOUND   = 3,   // We found the sync word, and will soon begin demodualting data
     DEM_DEMODUALTE   = 4,   // recovering the data and loading it into the fifo
     DEM_NOISE_ABORT  = 5,   // Noise abort. No more demodulation.
     DEM_FORCED_ABORT = 6,   // Forced abort. No more demodulation.
     DEM_RESTART      = 10   // Transitory. Always restart statem machine through here back to LOCKING
};


// Make these visible for debugging
extern unsigned short RawSamples[];             // holds the raw A/D converter readings
extern signed short RxSamples[];                // signed raw samples, centered around 0 with the DC component remoted.
extern signed int AutoCorr[];                   // The autocorrelation values of the samples


extern unsigned char RawInputIndex;             // index pointing to the location that the next A/D will be stored.
extern unsigned char RawSize;                   // These variables hold the actual sampling info that we use.
extern unsigned char SamplesPerBit;             // These variables hold the actual sampling info that we use.
extern unsigned char RawBits;                   // These variables hold the actual sampling info that we use.
extern signed short DCaverage;                  // The average of the two AverageSamleVal values.  Very close to the true DC average.
extern int MyNoiseLevel;                        // 0-100 for eye opening. 0 = no noise. 100 = 100% noise.
extern int PeakIndex;                           // The index of the sub-sample that is the peak value (I sample location)
extern unsigned char  RxDataShifter;


// Prototype the functions
void DemodulateStatemachine(void);


#endif	/* DEMODULATE_H */

