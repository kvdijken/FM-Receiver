/**
   Runs a Si5351, displays KHZ's on a 8 digit MAX7221/7219 led display.
   Frequency is set using a rotary encode.

   The Si5351 runs on I2c, which uses signals SDA and SCL. It uses the
   Wire library.

   Arduino Uno: SDA is on A4, SCL is on A5
   Arduino Nano:  SDA is on A4, SCL is on A5
*/

// Define DEBUG to send text over serial port.
// #define debug 1

long FWVersion = 58;  // i.e. "5.0"
// date Dec 4, 2020

// In case of an error, stop doing anything. The program loop will be suspended.
bool dontDoAnything = false;


// ==========================================================================
// WHEN STARTING CODING ON A NEW VERSION, FIRST CREATE A NEW ARDUINO PROJECT!
// ==========================================================================

// TODO 6.0 save settings in EEPROM, using EEPROM library.
//          See https://forum.arduino.cc/index.php?topic=45204.0
// TODO 6.0 Implement a way to reset after an error has ocurred
//          https://www.theengineeringprojects.com/2015/11/reset-arduino-programmatically.html
//          Create a new state STATE_ERROR, from which we can leave
//          by pressing DEFAULT button.
// TODO 6.0 implement a fast escape from settings to frequency/station modus
// TODO 6.0 Check if granularity in error detection is still good enough
//          now stateBase() is used.
// TODO 6.0 When in stateBase() an unknown state is encountered still
//          raise an error?
// TODO 6.0 implement left button + rotary function for stepping
//          thru settings. This is getting more important when
//          we add more settings. This prevents
//          us to use the left button many times in succession.
//          This may mean two new events have to be defined:
//          SHIFT_UP and SHIFT_DOWN, where SHIFT_UP is the
//          same as NEXT

// version 5.8
// x update state machine documentation
// x Check order of preprogrammed stations, should be ascending
// x Check for any double preprogrammed stations

// version 5.7
// x remove all stateToXXX() functions. These are only routing through
//          to the stateXXX() functions.
// x many state functions have exactly the same code, only
//          differ in small details. Investigate some inherit mechanism.
//          Seems to work fine in current implementation, but is it worth it?
// x redefine EVENT_UP as EVENT_UP, similar CCW -> DOWN
// x redefine STATE_CHANGE events as EVENT
// x is EVENT_IDLE a STATE or is it an EVENT? -> an EVENT
// x L1 > 0 > default > up --> 2
// x L2 > 0 > default > up --> 2

// version 5.6
// x implement low/high side injection switching
// x implement separate drive level for IF1 and IF2
// x implement drive level = 0 for IF1 and IF2
// x improve performance for rotating rotary button
// x implement full error/result checking, display E when an error has been detected
// x rename BTN_MODE, BTN_NEXT & BTN_DEFAULT, and associated other variables and functions
// x implement UP/DOWN for rotary changes instead  of 1 and -1. Also
//   implement error checking in rotary change functions.

// version 5.5
// x document displayDigit() - what signifies 'digit' exactly
// x display IF1 as 10.70, just as the tuning freq
// x for preprogrammed stations, display "P1." etc for 2 seconds

// version 5.4
// x use SI5351 for LO2
// x change default frequency selection to fast station search
// x add RegioFM at 95.30 MHz
// x change default IF to 10.7MHz
// x change frequency radix from 0.1MHz to 0.05MHz
//   for this we also need to change the frequency display

// x fast station switching using rotation instead of button.
//   set modus using button, then change station using rotation
// x rename debouncerNext to debouncerMode
// x rename btnNext to btnMode
// x rename old_btnNext to old_btnMode
// x EVENT_BTN_MODE_PRESSED to EVENT_BTN_MODE_PRESSED
// x EVENT_BTN_MODE_RELEASED to EVENT_BTN_MODE_RELEASED
// - stateStation needs to be reprogrammed using a speedMode toggle switch
//   to change stations fast or slow
// x change changeStation() to take speedMode into account
// x remove STATE_STATION
// x remove stateStation
// - implement a LED to show slow/fast station selection => now implemented by
//   the LED which was first used for the IF settings. It was a little bit overloaded,
//   and will be more overloaded if we also implement LO drive level setting.
// x rename LED_ID to LED_FAST

// version 5.3
// x check station frequencies (102.2 -> 102.0)
// x disable serial debug info for speed
// x new state to set LO output power
// x extend range of IF2 and change default. It seems that with higher IF2
//    there is less noise

// Version 5.2 will be an attempt to use the AD9851 for LO2
// implement mixer drive, toggle using an extra button, display current drive on screen

// version 5.1
// x displayIF1, displayIF2: display "IF1" and "IF2"



/*
      The current way the state machine is implemented has been to
      generate signals at the changes of button states. When there
      has not been a change in button state, an EVENT_IDLE is generated.

      Another way to do it is as follows. Continuously generate signals
      with the current state of the buttons, and have the state function
      decide if they want to do anything with it. This will give us the
      opportunity to respond to double button presses. So the state
      functions do not register state changes, but just record current
      state, and respond to it (or not).

      This will clearly be for a new FW version.
*/

// define for use with SI5351
#define _SI5351

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LedControl.h>
#include "si5351.h"
#include <Encoder.h>
#include <Bounce2.h>
#include "err.h"
#include "pins.h"
#include "states.h"
#include "stations.h"
#include "frequencies.h"
#include "clocks.h"

// How long will we stay in any of the  settings states (in miliseconds)
#define SETTINGS_SHOW_TIME 15000

// How long will we display the pre-programmed station number
// when fast selecting stations (in miliseconds).
#define STATION_SHOW_TIME 2000


// Arduino Pin 7 to DIN, 6 to Clk, 5 to LOAD, no.of devices is 1

// LedControl uses SPI, can use any of the pins
// SPI requires a separate slave select pin for any slave

LedControl lc = LedControl( LED_DATA, LED_CLK, LED_SELECT, 1 );


//Bounce bouncerRadix = Bounce( PIN_ENCODER_BTN, 50 );
Encoder encoder( PIN_ENCODER_A, PIN_ENCODER_B );


Bounce debouncerMode = Bounce();
Bounce debouncerNext = Bounce();
Bounce debouncerDefault = Bounce();

// Si5351 pieces
Si5351 si5351;



#define UP 1
#define DOWN -1

// Initial state of the state machine
int state = STATE_FREQUENCY;

#define STARTING_CRYSTAL_CAPACITANCE SI5351_CRYSTAL_LOAD_8PF
//#define STARTING_FREQ_CORRECTION -5700
#define STARTING_FREQ_CORRECTION 10720

// Radix for frequency dialing (in Hz)
long radix = 10000;

long frequency = INITIAL_FREQUENCY;  // in Hz

// intermediate frequency
long iFreq1 = DEFAULT_IF1;   // in Hz
long iFreq2 = DEFAULT_IF2;   // in Hz

long radixIF1 = 10000; // = 10KHZ
long radixIF2 = 10000; // = 10KHZ

// Drive levels for the local oscillators
int driveLevels[NUM_DRIVE_LEVELS] = { DRIVE_LEVEL_0MA,
                                      DRIVE_LEVEL_2MA,
                                      DRIVE_LEVEL_4MA,
                                      DRIVE_LEVEL_6MA,
                                      DRIVE_LEVEL_8MA };
int driveLevelsDisplay[NUM_DRIVE_LEVELS] = { 0, 2, 4, 6, 8 }; // What to display
int driveLevel[2] = { DEFAULT_DRIVELEVEL_MIXER1, DEFAULT_DRIVELEVEL_MIXER2 };        // index into driveLevels[] and driveLevelsDisplay[]

// Frequency selection
// SLOW = let user select exact frequency
// FAST = let user select preprogrammed stations
#define SPEED_MODE_SLOW 0
#define SPEED_MODE_FAST 1
#define DEFAULT_SPEED_MODE SPEED_MODE_FAST
int speedMode = DEFAULT_SPEED_MODE;



/**
  Displays a digit at location 'digit'. Location '0' is the rightmost
  digit. If decimalPoint == true, a decimal point will be displayed
  after the displayed digit. Parameter 'r' determines the digit to
  be displayed;
  r = 0: '0'
  r = 1: '1'
  r = 2: '2'
  r = 3: '3'
  r = 4: '4'
  r = 5: '5'
  r = 6: '6'
  r = 7: '7'
  r = 8: '8'
  r = 9: '9'
  r = 10: ' '
  r = 11: 'A'
  r = 12: 'b'
  r = 13: 'C'
  r = 14: 'd'
  r = 15: 'E'
  r = 16: 'F'
  r = 17: 'h'
  r = 18: 'L'
  r = 19: 'P'
  r = 20: '_'

*/
#define CHAR_0 0
#define CHAR_1 1
#define CHAR_2 2
#define CHAR_3 3
#define CHAR_4 4
#define CHAR_5 5
#define CHAR_6 6
#define CHAR_7 7
#define CHAR_8 8
#define CHAR_9 9
#define CHAR_SPACE 10
#define CHAR_A 11
#define CHAR_B 12
#define CHAR_C 13
#define CHAR_D 14
#define CHAR_E 15
#define CHAR_F 16
#define CHAR_H 17
#define CHAR_L 18
#define CHAR_P 19
#define CHAR__ 20

char c[21] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', 'A', 'b', 'C', 'd', 'E', 'F', 'h', 'L', 'P', '_'  };
bool displayDigit( int digit, byte r, bool decimalPoint ) {
  bool result = true;
  lc.setChar( 0, digit, c[r], decimalPoint );
  return result;
}


#define HISIDE 1
#define LOWSIDE 2
#define DEFAULT_INJECTION LOWSIDE

int injection = DEFAULT_INJECTION;

bool displayInjection() {
  bool result = true;
  // Display HiLo
  lc.setRow( 0, 7, B00110111 );  // H
  lc.setRow( 0, 6, B00001110 );  // L

  // Display either hi or lo
  if( injection == HISIDE ) {
    lc.setRow( 0, 1, B00010111 );  // H
    lc.setRow( 0, 0, B00000100 );  // i
  } else if( injection == LOWSIDE ) {
    lc.setRow( 0, 1, B00110000 );  // L
    lc.setRow( 0, 0, B00011101 );  // o
  } else
    result = false;
  return result;
}


int err = ERR_OK;

bool displayError( int error ) {
  bool result = true;

  clearDisplay();

  // Display 'Error'
  displayDigit( 7, CHAR_E, 0 ); // E
  lc.setRow( 0, 6, 0x05 );  // r
  lc.setRow( 0, 5, 0x05 );  // r
  lc.setRow( 0, 4, 0x1D );  // o
  lc.setRow( 0, 3, 0x05 );  // r

  // Display the error number
  displayLong( error, 0, false );

  return result;
}


/**
   Displays a long.

   Parameter 'decimals' signifies the number
   of digits after the decimal point.

   Examples:
   52, 1 prints 5.2
   9750, 2 prints 97.50
   8, 1 prints 8

   If parameter wipeLeft == true all characters left
   of the displayed digits will be wiped to spaces
   (but only in the frequency pane. The info pane will
   be left intact).
*/
bool displayLong( long n, int decimals, bool wipeLeft ) {
  bool result = true;
  int i;
  // The increment statement is executed after the body
  // of the for loop structure.
  for ( i=0; i<8 && (n>0 || i==0); i++ ) {
    // Always print a digit if i==0 to print something for n==0;
    int r = n % 10;
    n /= 10;
    // We have already displayed i more digits.
    result = result && displayDigit( i, (byte)r, i == decimals );
  }

  // Fill the remaining digits left of the
  // already printed digits with spaces.
  if ( wipeLeft && n == 0 )
    for ( int j = i; j < 5; j++ )
      // (byte) 10 is a space
      result = result && displayDigit( j, (byte) 10, false );
  return result;
}


/**
   displayFrequency

   Displays the frequency as xx.xx MHz

   frequency (in Hz)
*/
bool displayFrequency()
{
  bool result = true;
  
#ifdef debug
  Serial.print( "displayFrequency " );
  Serial.println( frequency );
#endif

  // only display the 10KHZ part of frequency
  long f = frequency / 10000;
  result = result && displayLong( f, 2, true );
  return result;
}


/**
 * 
*/
bool displayFWVersion() {
  bool result = true;
  result = result && displayLong( FWVersion, 1, true );
  return result;
}


/**
 * 
*/
bool displayIF1()
{
  bool result = true;

  // display the KHZ part of first intermediate frequency
  long f = iFreq1 / 1000;
  result = result && displayLong( f, 3, true );

  // Display which IF we are showing.
  result = result && displayDigit( 7, CHAR_F, false );
  result = result && displayDigit( 6, CHAR_1, false );

  return result;
}


/**
 * 
*/
bool displayIF2()
{
  bool result = true;

  long f = iFreq2 / 1000;

  // Now the frequency is in KHZ
  // Intermediate frequency 2 is always less then 1000 KHZ.
  // We show the KHZ part...
  result = result && displayLong( f, -1, true );

  // ... and also the MHz part (which is 0).
  result = result && displayDigit( 3, CHAR_0, true );

  // Display which IF we are showing.
  result = result && displayDigit( 7, CHAR_F, false );
  result = result && displayDigit( 6, CHAR_2, false );
  return result;
}


/**
   Displays the drive level for the given clock.
   This assumes that the memory variable for the
   LO drive level has been set correctly.

   clock = CLK_IF1 or CLK_IF2
*/
bool displayDriveLevel( int clock )
{
  bool result = true;

  // At the left side of the display we show what LO
  // we are showing the drive level of.
  result = result && displayDigit( 7, CHAR_L, false );
  switch( clock ) {
    case CLK_IF1:
      result = result && displayDigit( 6, CHAR_1, false );
      break;
    case CLK_IF2:
      result = result && displayDigit( 6, CHAR_2, false );
      break;
    default:
      err = ERR_ILLEGAL_CLOCK;
      result = false;
      break;
  }

  // At the right side of the display we show the drive level
  result = result && displayLong( driveLevelsDisplay[driveLevel[clock]], -1, true );

  return result;
}


/**
   This programs the first oscillator for IF1.
   This is dependent on the tuning frequency and the required
   first intermediate frequency.

   It sets the frequency for the first mixer LO. The first mixer should mix
   a signal at the frequency the radio is tuned to down to IF1. So the frequency
   of the LO of the first mixer should be at 'frequency' - LO1.
*/
bool setOSC1() {
  bool result = true;
  uint64_t freqLO1;
  switch( injection ) {
    case LOWSIDE:
      freqLO1 = frequency + FREQUENCY_CORRECTION - iFreq1;
      break;
    case HISIDE:
      freqLO1 = frequency + FREQUENCY_CORRECTION + iFreq1;
      break;
    default:
      err = ERR_ILLEGAL_INJECTION;
      result = false;
  }
#ifdef debug
  Serial.println();
  Serial.println( "setOSC1()" );
  Serial.print( "freqLO1 = " );
  Serial.println( (unsigned long) freqLO1 );
#endif
  if( result )
    si5351.set_freq( freqLO1 * 100ULL, SI5351_CLK0 );

  // Now also the frequency for the second mixer LO needs to be reset again
  // The second LO depends on the IF1 and IF2.
  result = result && setOSC2();

  return result;
}


/**
   This programs the second oscillator for IF2.

   It sets the frequency for the second mixer LO. The second mixer should mix
   a signal at IF1 down to IF2. So the LO should be tuned to IF1 - IF2.
*/
bool setOSC2() {
  bool result = true;
  uint64_t freqLO2 = iFreq1 - iFreq2;
#ifdef debug
  Serial.println();
  Serial.println( "setOSC2()" );
  Serial.print( "freqLO2 = " );
  Serial.println( (unsigned long) freqLO2 );
#endif
  //  dds.setFrequency( freqLO2 );
  si5351.set_freq( freqLO2 * 100ULL, SI5351_CLK1 ); // d = 0 KHZ
  return result;
}


/*
   Use this function if a new frequency has been set and needs to be used.

   It displays the frequency and sets the local oscillator(s).

*/
bool newFrequency() {
  bool result = true;
  result = result && displayFrequency();
  
#ifdef debug
  Serial.println();
  Serial.println( "newFrequency()" );
  Serial.print( "frequency = " );
  Serial.println( frequency );
#endif

  // Radio is tuned by setting the first oscillator.
  result = result && setOSC1();
  return result;
}


/*
   Change the frequency the radio is tuned to.

   dir == 1:  increment by radix
   dir == -1: decrement by radix
*/
bool changeFrequency( short dir )
{
  bool result = true;
  switch( dir ) {
    case UP:
      frequency += radix;
      if (frequency > MAX_FREQUENCY)
        frequency = MAX_FREQUENCY;
      break;
    case DOWN:
      if ( frequency > MIN_FREQUENCY + radix )
        frequency -= radix;
      else
        frequency = MIN_FREQUENCY;
      break;
    default:
      result = false;
  }
  result = result && newFrequency();
  return result;
}


bool displayingStation = false;

/*
   Displays the station number as ' 1.' or '12.'
   Sets a timer. After a certain (preprogrammed) time,
   the station number will disappear again.
*/
bool displayStation( int i ) {
  bool result = true;

  // in any case we will have to update the station number.
  if ( i < 10 ) {
    // One digit
    result = result && displayDigit( 7, CHAR_SPACE, false );
    result = result && displayDigit( 6, i, true );
  } else {
    // Two digits
    result = result && displayDigit( 7, i / 10, false );
    result = result && displayDigit( 6, i % 10, true );
  }

  displayingStation = true;
  result = result && startStationTimer();

  return result;
}


/**
   Empties the info pane.
   The info pane are the two leftmost
   characters in the display.
*/
bool emptyInfoPane() {
  bool result = true;
  result = result && displayDigit( 7, CHAR_SPACE, false );
  result = result && displayDigit( 6, CHAR_SPACE, false );
  return result;
}


/**
   Clears the entire display.
*/
bool clearDisplay() {
  bool result = true;
  for ( int i = 0; i < 8 && result; i++ )
    result = result && displayDigit( i, CHAR_SPACE, false );
  return result;
}


/*
   Cleans up the area taken up by the station indication.
   Erases the first two characters in the display.
*/
bool hideStation() {
  bool result = true;
  if ( !displayingStation ) {
    // Not displaying the station number.
    // This must be a programming error.
    err = ERR_ILLEGAL_STATION;
    result = false;
  } else {
    result = result && emptyInfoPane();
    displayingStation = false;
  }
  return result;
}


/**
   Changes frequency to the frequency of the
   previous preprogrammed station. If there is no
   previous station, go to the last station.
*/
bool prevStation()
{
  bool result = false;
  int i = NSTATIONS - 1;

  // Find the last frequency in freqs[] which is smaller than 'frequency'.
  // freqs[] unit is STATIONS_UNIT, frequency is in Hz
  for ( ; i >= 0 && !result; i-- )
    if ( freqs[i] * STATIONS_UNIT < frequency )
      result = true;
  // after the body of the loop is completed, the dec statement
  // will be executed. See https://www.tutorialspoint.com/cprogramming/c_for_loop.htm
  // So i contains the station number (zero-based) before the tested station.

  int s = i + 1; // freqs[s] is the station to be selected.
  if ( result ) {
    // freqs[i+1] * STATIONS_UNIT < frequency
    // i+1 the the first station with frequency
    // lower than the current frequency
    // freqs[s] * STATIONS_UNIT < frequency
  }

  if( !result ) {
    // wrap around, select the last station
    s = NSTATIONS - 1;
    result = true;
  }

  // i contains the station to be selected now
  frequency = freqs[s] * STATIONS_UNIT;

  result = result && newFrequency();
  result = result && displayStation( s + 1 ); // station list is zero-based

  return result;
}


/**
   Changes frequency to the frequency of the
   next preprogrammed station. If there is no
   next station, go to the first station.
*/
bool nextStation()
{
  bool result = false;
  int i = 0;

  // Find the first frequency in freqs[] which is larger than 'frequency'.
  // freqs[] unit is STATIONS_UNIT, frequency is in Hz
  for ( ; i < NSTATIONS && !result; i++ )
    if ( freqs[i] * STATIONS_UNIT > frequency )
      result = true;
  // after the body of the loop is completed, the inc statement
  // will be executed. See https://www.tutorialspoint.com/cprogramming/c_for_loop.htm
  // So i contains the station number (zero-based) after the tested station.

  int s = i - 1; // freqs[s] is the station to be selected.
  if ( result ) {
    // freqs[i-1] * STATIONS_UNIT < frequency
    // i+1 the the first station with frequency
    // lower than the current frequency
    // freqs[s] * STATIONS_UNIT < frequency
  }

  if ( !result ) {
    // wrap around, select the first station
    s = 0;
    result = true;
  }

  // i contains the station to be selected now
  frequency = freqs[s] * STATIONS_UNIT;

  result = result && newFrequency();
  result = result && displayStation( s + 1 ); // remember the list of stations is zero-based

  return result;
}


/**
 * 
*/
bool changeIF1( short dir ) {
  bool result = true;
  switch( dir ) {
    case UP:
      iFreq1 += radixIF1;
      if ( iFreq1 > MAX_IF1 )
        iFreq1 = MAX_IF1;
      break;
    case DOWN:
      if ( iFreq1 > MIN_IF1 + radixIF1 )
        iFreq1 -= radixIF1;
      else
        iFreq1 = MIN_IF1;
      break;
    default: 
      result = false;
      break;
  }
  result = result && displayIF1();
  result = result && setOSC1();
  return result;
}


/**
 * 
*/
bool changeIF2( short dir ) {
  bool result = true;
  switch( dir ) {
    case UP:
      iFreq2 += radixIF2;
      if ( iFreq2 > MAX_IF2 )
        iFreq2 = MAX_IF2;
      break;
    case DOWN:
      if ( iFreq2 > MIN_IF2 + radixIF2 )
        iFreq2 -= radixIF2;
      else
        iFreq2 = MIN_IF2;
      break;
    default: 
      result = false;
      break;
  }
  result = result && displayIF2();
  result = result && setOSC2();
  return result;
}


/**
 * 
*/
bool changeDriveLevel( int clock, short dir ) {
  bool result = true;
  switch( dir ) {
    case UP:
      driveLevel[clock] += RADIX_DRIVELEVEL;
      if ( driveLevel[clock] > MAX_DRIVELEVEL )
        driveLevel[clock] = MAX_DRIVELEVEL;
      break;
    case DOWN:
      if ( driveLevel[clock] > MIN_DRIVELEVEL + RADIX_DRIVELEVEL)
        driveLevel[clock] -= RADIX_DRIVELEVEL;
      else
        driveLevel[clock] = MIN_DRIVELEVEL;
      break;
    default:
      result = false;
      break;
  }
  result = result && displayDriveLevel( clock );
  result = result && setSi5351DriveLevel( clock );
  return result;
}


/**

*/
bool changeInjection( short dir ) {
  bool result = true;
  int old = injection;
  switch( dir ) {
    case UP:
      injection = HISIDE;
      break;
    case DOWN:
      injection = LOWSIDE;
      break;
    default:
      result = false;
  }
  if( injection != old ) {
    // TODO: Check if these display instructions are not done twice.
    result = result && displayInjection();
    result = result && setOSC1();
  }
  return result;
}


// Time at which the counter for how long to stay in any
// other state than stateFrequency started counting.
// May be be reset any time the countdown should be reset to zero.
// This is in miliseconds, as taken from the Arduino millis() timer.
unsigned long timerSettings;

unsigned long timerStation;

unsigned long timerRotary;

/**
   (Re)sets the time for any of the settings states.
*/
bool startSettingsTimer() {
  bool result = true;
  timerSettings = millis();
  return result;
}


/**
   (Re)sets the time for any of the settings states.
*/
bool startRotaryTimer() {
  bool result = true;
  timerRotary = millis();
  return result;
}


/**
   (Re)sets the time for the period we show station number.
*/
bool startStationTimer() {
  bool result = true;
  timerStation = millis();
  return result;
}


/**
   Returns true if the number of seconds
   since the statetimer has been started is larger
   than the preprogrammed amount (SETTINGS_SHOW_TIME miliseconds).
*/
bool checkSettingsTimer() {
  unsigned long m = (millis() - timerSettings);
  return m >= SETTINGS_SHOW_TIME;
}

#define ROTARY_TIME 500

bool checkRotaryTimer() {
  unsigned long m = (millis() - timerRotary);
  return m >= ROTARY_TIME;
}


/**
   Returns true if the number of seconds
   since the stationtimer has been started is larger
   than the preprogrammed amount (STATION_SHOW_TIME miliseconds)
*/
bool checkStationTimer() {
  unsigned long m = (millis() - timerStation);
  return m >= STATION_SHOW_TIME;
}


/**
 * 
 */
bool ledOff() {
  bool result = true;
  digitalWrite( LED_FAST, LOW );
  return result;
}


/*
 * 
 */
bool ledOn() {
  bool result = true;
  digitalWrite( LED_FAST, HIGH );
  return result;
}


/*
 * Let the LED display if the radio is in
 * station select mode (led = on) or or 
 * frequency select mode (led = off).
*/
bool ledSpeedMode() {
  bool result = true;
  if ( speedMode == SPEED_MODE_FAST )
    result = result && ledOn();
  else if ( speedMode == SPEED_MODE_SLOW )
    result = result && ledOff();
  else {
    err = ERR_ILLEGAL_SPEEDMODE_LED;
    result = false;
  }
  return result;
}


/**
   Program actions which should be performed
   when entering a state.
*/
bool enterState( int to ) {
  bool result = true;

  // As long as all states show their own info in the infoPane
  // we should empty the info pane whenever we change state. If
  // not all states show info, only leaving states which show
  // info should empty the info pane.
  // Or, whenever we enter a state which does not show info,
  // we should empt the info pane.

  switch( to ) {
    case STATE_FREQUENCY:
        // This state does not always show info.
        // So, empty the info pane.
        result = result && emptyInfoPane();
        result = result && displayFrequency();
        break;
    case STATE_IF1:
        result = result && displayIF1();
        result = result && startSettingsTimer();
        break;
    case STATE_IF2:
        result = result && displayIF2();
        result = result && startSettingsTimer();
        break;
    case STATE_DRIVELVL_1:
        result = result && displayDriveLevel( CLK_IF1 );
        result = result && startSettingsTimer();
        break;
    case STATE_DRIVELVL_2:
        result = result && displayDriveLevel( CLK_IF2 );
        result = result && startSettingsTimer();
        break;
    case STATE_INJECTION:
      result = result && displayInjection();
      result = result && startSettingsTimer();
      break;
    default:
        // programming error
        err = ERR_ILLEGAL_STATE_ENTERSTATE;
        result = false;
        break;
  }
  if( result )
    state = to;
  return result;
}


/**

*/
bool toggleSpeed() {
  bool result = true;
  switch( speedMode ) {
    case SPEED_MODE_SLOW:
      speedMode = SPEED_MODE_FAST;
      break;
    case SPEED_MODE_FAST:
      speedMode = SPEED_MODE_SLOW;
      break;
    default:
      // programming error
      err = ERR_ILLEGAL_SPEEDMODE_TOGGLE;
      result = false;
      break;
  }
  result = result && ledSpeedMode();
  return result;
}


/**
 * 
 */
bool stateBase( int change, int nextState ) {
  bool result = true;
  switch( change ) {
    case EVENT_DOWN: 
        result = result && startSettingsTimer();
        break;
    case EVENT_UP:
        result = result && startSettingsTimer();
        break;
    case EVENT_BTN_NEXT_PRESSED:
        // Change state to the next state.
        result = result && enterState( nextState );
        break;
    case EVENT_BTN_DEFAULT_PRESSED:
        // TODO next line is not present in the implementation
        // as used until now. Should the timer not be reset
        // upon pressing the DEFAULT button?
//        result = result && startSettingsTimer();
        break;
    case EVENT_BTN_DEFAULT_RELEASED:
        result = result && startSettingsTimer();
        break;
    case EVENT_BTN_MODE_PRESSED:
        result = result && toggleSpeed();
        break;
    case EVENT_BTN_MODE_RELEASED:
      break;
    case EVENT_IDLE:
        // If we stay in this state 10 seconds, we return to
        // the frequency state, where the frequency can be changed.
        if( checkSettingsTimer() )
          result = result && enterState( STATE_FREQUENCY );
        break;
    default:
        // programming error
        err = ERR_ILLEGAL_CHANGE_STATEINJECTION;
        result = false;
        break;
  }
  return result;
}


/**

*/
bool stateFrequency( int change ) {
  bool result = true;
  switch( change ) {
    case EVENT_DOWN:
        if ( speedMode == SPEED_MODE_SLOW )
          result = result && changeFrequency( DOWN );
        else if ( speedMode == SPEED_MODE_FAST )
          result = result && prevStation();
        else
          // programming error
          err = ERR_ILLEGAL_SPEEDMODE_STATEFREQUENCY;
          result = false;
        break;
    case EVENT_UP:
        if ( speedMode == SPEED_MODE_SLOW )
          result = result && changeFrequency( UP );
        else if ( speedMode == SPEED_MODE_FAST )
          result = result && nextStation();
        else
          // programming error
          err = ERR_ILLEGAL_SPEEDMODE_STATEFREQUENCY;
          result = false;
        break;
    case EVENT_BTN_MODE_PRESSED:
        result = result && toggleSpeed();
        break;
    case EVENT_BTN_MODE_RELEASED:
      break;
    case EVENT_BTN_NEXT_PRESSED:
        result = result && enterState( STATE_IF1 );
        break;
    case EVENT_IDLE:
        if ( displayingStation && checkStationTimer() )
          result = result && hideStation();
        break;
    default:
        // programming error
        err = ERR_ILLEGAL_CHANGE_STATEFREQUENCY;
        result = false;
        break;
  }
  return result;
}


/**

*/
bool stateIF1( int change ) {
  bool result = true;
  switch ( change ) {
    case EVENT_DOWN:
      // Decrease first intermediate frequency
      result = result && changeIF1( DOWN );
      break;
    case EVENT_UP:
      // Increase first intermediate frequency
      result = result && changeIF1( UP );
      break;
    case EVENT_BTN_DEFAULT_PRESSED:
      // Setting the intermediate frequency to its default value is done
      // on pressing the 'default' button.
      iFreq1 = DEFAULT_IF1;
      result = result && displayIF1();
      result = result && setOSC1();
      break;
    default:
      break;
  }
  result = result && stateBase( change, STATE_IF2 );
  return result;
}


/**
 * 
 */
bool stateIF2( int change ) {
  bool result = true;
  switch ( change ) {
    case EVENT_DOWN:
        result = result && changeIF2( DOWN );
        break;
    case EVENT_UP:
        result = result && changeIF2( UP );
        break;
    case EVENT_BTN_DEFAULT_PRESSED:
        // Setting the intermediate frequency to its default value is done
        // on pressing the 'default' button.
        iFreq2 = DEFAULT_IF2;
        result = result && displayIF2();
        result = result && setOSC2();
        break;
    default:
      // Hopefully all not-handled case will be handled by
      // the default state handler. If not, the defaults
      // handler will raise an error.
      break;
  }
  // Perform the default handling for all states.
  result = result && stateBase( change, STATE_DRIVELVL_1 );
  return result;
}

/**

*/
bool stateDriveLVL1( int change ) {
  bool result = true;
  switch ( change ) {
    case EVENT_DOWN:
        result = result && changeDriveLevel( CLK_IF1, DOWN );
        break;
    case EVENT_UP:
        result = result && changeDriveLevel( CLK_IF1, UP );
        break;
    case EVENT_BTN_DEFAULT_PRESSED:
        // Setting the drivelevel to its default value is done
        // on pressing the 'default' button.
        driveLevel[CLK_IF1] = DEFAULT_DRIVELEVEL_MIXER1;
        result = result && displayDriveLevel( CLK_IF1 );
        result = result && setSi5351DriveLevel( CLK_IF1 );
        break;
    default:
        break;
  }
  // Perform the default handling for all states.
  result = result && stateBase( change, STATE_DRIVELVL_2 );
  return result;
}


/**

*/
bool stateDriveLVL2( int change ) {
  bool result = true;
  switch ( change ) {
    case EVENT_DOWN:
        result = result && changeDriveLevel( CLK_IF2, DOWN );
        break;
    case EVENT_UP:
        result = result && changeDriveLevel( CLK_IF2, UP );
        break;
    case EVENT_BTN_DEFAULT_PRESSED:
        // Setting the drivelevel to its default value is done
        // on pressing the 'default' button.
        driveLevel[CLK_IF2] = DEFAULT_DRIVELEVEL_MIXER2;
        result = result && displayDriveLevel( CLK_IF2 );
        result = result && setSi5351DriveLevel( CLK_IF2 );
        break;
    default:
        break;
  }
  // Perform the default handling for all states.
  result = result && stateBase( change, STATE_INJECTION );
  return result;
}


/**
 * 
 */
bool stateInjection( int change ) {
  bool result = true;
  // Only handle the cases which are special to this state.
  // All general cases will be handled later by stateBase().
  switch( change ) {
    case EVENT_DOWN:
        result = result && changeInjection( DOWN );
        break;
    case EVENT_UP:
        result = result && changeInjection( UP );
        break;
    case EVENT_BTN_DEFAULT_PRESSED:
        // Setting the injection-side to its default value 
        // is done on pressing the 'default' button.
        injection = DEFAULT_INJECTION;
        result = result && displayInjection();
        result = result && setOSC1();
        break;
    default:
      // Hopefully all not-handled case will be handled by
      // the default state handler. If not, the defaults
      // handler will raise an error.
      break;
  }
  // Perform the default handling for all states.
  result = result && stateBase( change, STATE_FREQUENCY );
  return result;
}


/**

*/
bool stateChange( int change ) {
  int result = true;

#ifdef debug
//  Serial.println( "stateChange" );
#endif

  // Every state has got its own change of state method.
  // Check in which state we are now, and then delegate
  // the change to the appropriate state method
  switch( state ) {
    case STATE_FREQUENCY:
        result = result && stateFrequency( change );
        break;
    case STATE_IF1:
        result = result && stateIF1( change );
        break;
    case STATE_IF2:
        result = result && stateIF2( change );
        break;
    case STATE_DRIVELVL_1:
        result = result && stateDriveLVL1( change );
        break;
    case STATE_DRIVELVL_2:
        result = result && stateDriveLVL2( change );
        break;
    case STATE_INJECTION:
        result = result && stateInjection( change );
        break;
    default:
        // programming error
        err = ERR_ILLEGAL_CHANGE_STATECHANGE;
        result = false;
        break;
  }
  return result;
}


/*
   This sets the drive level for the mixers.
   This sets the Si5351 oscillator frequency.
   It depends on the driveLevel[] variable
   to have the right value.

   clock = 1 or 2
*/
bool setSi5351DriveLevel( int clock ) {
  bool result = true;
  int si_clock, si_level, level = driveLevel[clock];

  // decide which clock to set
  if ( clock == CLK_IF1 )
    si_clock = SI5351_CLK0;
  else if ( clock == CLK_IF2 )
    si_clock = SI5351_CLK1;
  else {
    err = ERROR_ILLEGAL_CLOCK;
    result = false;
  }

  // decide which level to set it to
  if ( level == DRIVE_LEVEL_0MA ) {
    // Switch it off entirely
    if ( result )
      si5351.output_enable( si_clock, 0 );  // void result
  }
  else {
    switch ( level ) {
      case DRIVE_LEVEL_2MA:
        si_level = SI5351_CLK_DRIVE_STRENGTH_2MA;
        break;
      case DRIVE_LEVEL_4MA:
        si_level = SI5351_CLK_DRIVE_STRENGTH_4MA;
        break;
      case DRIVE_LEVEL_6MA:
        si_level = SI5351_CLK_DRIVE_STRENGTH_6MA;
        break;
      case DRIVE_LEVEL_8MA:
        si_level = SI5351_CLK_DRIVE_STRENGTH_8MA;
        break;
      default:
        err = ERR_ILLEGAL_DRIVELEVEL;
        result = false;
    }
    if ( result ) {
      // set the required level
      si5351.drive_strength( si_clock, si_level );  // void result
      // it may previously have been disabled, so enable it, just in case
      si5351.output_enable( si_clock, 1 ); // void result
    }
  }
  return result;
}


/**

*/
#ifdef _SI5351
void setupSi5351()
{
  // Setup for the Si5351
  //  si5351.init( STARTING_CRYSTAL_CAPACITANCE, 0, 16764 );
  si5351.init( STARTING_CRYSTAL_CAPACITANCE, 0, STARTING_FREQ_CORRECTION );
  // Kvd 15.ii.2018 met 25MHz kristal voor AdaFruit Si5351, 8pF en 18700 calibratie levert hij de beste resultaten
  // Kvd 17.ii.2018 met 25MHz kristal voor AdaFruit Si5351, 10pF en -5700 calibratie levert hij de beste resultaten

  // Initial drive levels
  setSi5351DriveLevel( CLK_IF1 );
  setSi5351DriveLevel( CLK_IF2 );
  setOSC1();
  setOSC2();
}
#endif


/*
  // Sanity check for list of preprogrammed stations
  // Ascending, and no doubles
 */
bool checkStations() {
  bool result = true;
  long last = 0;
  for( int i=0; i<NSTATIONS && result; i++ ) {
    result = (freqs[i] > last);
    last = freqs[i];
  }
  return result;
}


/*

*/
void setup() {
#ifdef debug
  Serial.begin( 9600 );
  Serial.println();
  Serial.println( "setup()" );
#endif

  // Initialize the MAX7219 device
  lc.shutdown(0, false);  // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register

  displayFWVersion();
  delay( 1000 );

  // Sanity check for list of preprogrammed stations
  // Ascending, and no doubles
  if( !checkStations() ) {
    displayError( ERR_STATION_LIST );
    dontDoAnything = true;
    return;
  }

#ifdef _SI5351
  // Setup the I2C comm to the Si5351
  // Do this at first, so maybe at startup we will get sound
  // from the radio which is better than noise.
  setupSi5351();
#endif

  pinMode( LED_FAST, OUTPUT );
  ledSpeedMode();

  pinMode( BTN_MODE, INPUT_PULLUP );
  pinMode( BTN_NEXT, INPUT_PULLUP );
  pinMode( BTN_DEFAULT, INPUT_PULLUP );

  debouncerMode.attach( BTN_MODE );
  debouncerMode.interval( 25 ); // interval in ms
  debouncerNext.attach( BTN_NEXT );
  debouncerNext.interval( 25 ); // interval in ms
  debouncerDefault.attach( BTN_DEFAULT );
  debouncerDefault.interval( 25 ); // interval in ms

  // Setup for the rotary encoder
  // temp comment
  //  encoder.write( 0 );

  //  pinMode( PIN_ENCODER_BTN, INPUT_PULLUP );

  displayFrequency();

  // TODO This is already done in setupSi5351()
  setOSC1();
}


int old_e = 0;

/**
 * 
 */
bool checkRotary( bool *change ) {
  bool result = true;
  *change = false;
  int value = encoder.read();
  if( value % 2 == 0 && value != old_e ) {
    // There has been a change in encoder value.
    if( value > old_e )
      result = result && stateChange( EVENT_UP );
    else
      result = result && stateChange( EVENT_DOWN );
    old_e = value;
    *change = true;
  }
  return result;
}


int old_btnMode = HIGH;
int btnMode = 0;
int old_btnNext = HIGH;
int btnNext = 0;
int old_btnDefault = HIGH;
int btnDefault = 0;

/**
 * 
 */
bool checkButtons() {
  bool result = true;

  // NEXT button
  debouncerNext.update();
  btnNext = debouncerNext.read();
  if ( btnNext != old_btnNext )
  {
    if ( btnNext == LOW )
      result = result && stateChange( EVENT_BTN_NEXT_PRESSED );
//    else
//      result = result && stateChange( EVENT_BTN_NEXT_RELEASED );
    old_btnNext = btnNext;
  }

  // MODE button
  debouncerMode.update();
  btnMode = debouncerMode.read();
  if ( btnMode != old_btnMode )
  {
    if ( btnMode == LOW )
      result = result && stateChange( EVENT_BTN_MODE_PRESSED );
    else
      result = result && stateChange( EVENT_BTN_MODE_RELEASED );
    old_btnMode = btnMode;
  }

  // DEFAULT button
  debouncerDefault.update();
  btnDefault = debouncerDefault.read();
  if ( btnDefault != old_btnDefault )
  {
    if ( btnDefault == LOW )
      result = result && stateChange( EVENT_BTN_DEFAULT_PRESSED );
    else
      result = result && stateChange( EVENT_BTN_DEFAULT_RELEASED );
    old_btnDefault = btnDefault;
  }
  return result;
}


/**
 * 
 */
bool doLoopBody() {
  bool result = true;
  bool rotary;
  // Check the inputs for the state machine.

  // Check encoder inputs
  result = result && checkRotary( &rotary );

  if( rotary )
    result = result && startRotaryTimer();
  else 
    // no changes on the rotary encoder this loop
    if( checkRotaryTimer() ) {
      // No encoder change in the last ROTARY_TIME msecs. 
      // Check button inputs.
      result = result && checkButtons();
      result = result && stateChange( EVENT_IDLE );
    }

  if ( err != ERR_OK ) {
    // result should be false
    displayError( err );
    // From now on, don't do anything anymore.
    dontDoAnything = true;
  }
  
  return result;
}


/**
 * 
*/
void loop() {
  if ( !dontDoAnything )
    doLoopBody();
}
