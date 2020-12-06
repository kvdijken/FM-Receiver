#ifndef STATION_H
#define STATION_H

#include "frequencies.h"

// In this file the preprogrammed stations are mentioned.

#define NSTATIONS 30

// 30 preprogrammed radiostations, in MHz
#define RADIO10_1 87.6
#define RADIO2 88.0
#define _3FM 88.6
#define GRUNNFM 89.1
#define BNR 89.6

#define SUBLIMEFM 90.3
#define RADIODRENTHE 90.8
#define SIMONEFM_1 91.3
#define RADIO1 91.8
#define RADIOCONTINU 92.4

#define SIMONEFM_2 92.9
#define SLAMFM_1 93.7
#define _100PROCENTNL_2 94.2
#define RADIO4 94.8
#define REGIOFM 95.3

#define RADIONOORD 97.5
#define JOYRADIO 98.5
#define _100PROCENTNL_1 99.1
#define SLAMFM_2 99.6
#define QMUSIC 100.4

#define SKYRADIO 101.0
#define RADIO538 102.2
#define RADIO538_2 102.7
#define RADIOVERONICA_2 103.2
#define RADIOVERONICA_1 103.4

#define SIMONEFM_3 103.6
#define RADIO10_2 103.8
#define RADIONL 104.4
#define RADIO_CONTINU_DRENTHE 104.7
#define OOG 106.6

#define PRONKJEWAIL 145.75

// The frequency unit of the stations in the list mentioned above
#define LIST_UNIT MHZ

// The frequency unit of the stations in the freqs[] array below
#define STATIONS_UNIT KHZ

// To convert from LIST_UNIT to STATIONS_UNIT
#define CONV_UNIT MHZ/KHZ

// Make sure that this array is sorted ascendingly.
// This will be chacked in the sketch. If the order
// is not ascending, or their are doubles frequencies
// a fatal error will be raised.

// Frequencies in KHZ
long freqs[NSTATIONS] = {
  RADIO10_1 * CONV_UNIT,
  RADIO2 * CONV_UNIT,
  _3FM * CONV_UNIT,
  GRUNNFM * CONV_UNIT,
  BNR * CONV_UNIT,
  SUBLIMEFM * CONV_UNIT,
  RADIODRENTHE * CONV_UNIT,
  SIMONEFM_1 * CONV_UNIT,
  RADIO1 * CONV_UNIT,
  RADIOCONTINU * CONV_UNIT,
  SIMONEFM_2 * CONV_UNIT,
  SLAMFM_1 * CONV_UNIT,
  _100PROCENTNL_2 * CONV_UNIT,
  RADIO4 * CONV_UNIT,
  REGIOFM * CONV_UNIT,
  RADIONOORD * CONV_UNIT,
  JOYRADIO * CONV_UNIT,
  _100PROCENTNL_1 * CONV_UNIT,
  SLAMFM_2 * CONV_UNIT,
  QMUSIC * CONV_UNIT,
  SKYRADIO * CONV_UNIT,
  RADIO538 * CONV_UNIT,
  RADIO538_2 * CONV_UNIT,
  RADIOVERONICA_2 * CONV_UNIT,
  RADIOVERONICA_1 * CONV_UNIT,
  SIMONEFM_3 * CONV_UNIT,
  RADIO10_2 * CONV_UNIT,
  RADIONL * CONV_UNIT,
  RADIO_CONTINU_DRENTHE * CONV_UNIT,
  OOG * CONV_UNIT
};

//  PRONKJEWAIL


#endif
