#ifndef FREQUENCIES_H
#define FREQUENCIES_H

// Frequencies
// Lower frequency limit 87.5MHz (in Hz)
//#define MIN_FREQUENCY  87500000
#define MIN_FREQUENCY  1000000
// Upper frequency limit 108MHz (in Hz)
//#define MAX_FREQUENCY 108000000
#define MAX_FREQUENCY 220000000
// all frequencies are in 1 Hz

#define HZ 1
#define KHZ 1000
#define MHZ 1000000

// in Hz
#define INITIAL_FREQUENCY RADIONOORD * MHZ

#define MIN_IF1 10000000
#define MAX_IF1 11000000
#define DEFAULT_IF1 10660000
#define MIN_IF2 100000
#define MAX_IF2 500000
#define DEFAULT_IF2 450000

// Frequency correction which is used to set the frequency
// of the first local oscillator.
#define FREQUENCY_CORRECTION -10000

#endif

