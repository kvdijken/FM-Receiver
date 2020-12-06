/*
 * Library for the AD9851 DDS chip
 *
 * Connect:
 * SPI CLK (Uno Pin 13) to AD9851 W_CLK
 * SPI MOSI (Uno Pin 11) to AD9851 DATA (D7)
 * Your chosen CS pin to AD9851 FQ_UD (device select)
 * Your chosen Reset pin to AD9851 ResetPin (device reset)
 *
 * To calibrate, use a zero calibration value, set some frequency and
 * measure it as accurately as you can with a frequency counter.
 * Convert that value to parts-per-billion error (positive if the
 * frequency is high, negative if low) and provide that integer to
 * setClock() as the calibration value instead.
 *
 * You must setFrequency() after calling setClock with a new calibration.
 */
#ifndef _AD9851_h_
#define _AD9851_h_

#include "Arduino.h"
#include <SPI.h>

#if ARDUINO < 10600
#error "Arduino 1.6.0 or later (SPI library) is required"
#endif

template <
  uint8_t ResetPin,                     // Reset pin (active = high)
  uint8_t FQ_UDPin,                     // Data Load pin (active = pulse high)
  uint32_t reference_freq = 180L*1000*1000, // 180MHz
  long SPIRate = 2000000,               // 2MBit/s
  uint8_t SPIClkPin = 13,               // Note: do not change the SPI pins, they're currently ignored.
  uint8_t SPIMOSIPin = 11
>
class AD9851 {
  uint32_t      reciprocal;
  uint8_t       shift;                  // (2<<shift) < core_clock <= (2<<shift+1) (27 for 180MHz)

public:
  AD9851()
  {
    pinMode(ResetPin, OUTPUT);          // Ensure we can reset the AD9851
    pinMode(FQ_UDPin, OUTPUT);          // This control signal applies the loaded values
    // Ensure that the SPI device is initialised
    // "setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high"
    SPI.begin();

    reset();
  }

  void reset()
  {
    pulse(ResetPin);                    // (minimum 5 cycles of the 30MHz clock)
    pulse(SPIClkPin);                   // Enable serial loading mode:
    pulse(FQ_UDPin);

    // To avoid possible false configuration settings, immediately set the frequency
    setClock(0);
    setFrequency(1);
  }

  void setClock(int32_t calibration = 0)
  {
    uint32_t core_clock = reference_freq * (1000000000ULL-calibration) / 1000000000ULL;
    uint64_t    scaled = core_clock;
    for (shift = 32; shift > 0 && (scaled&0x100000000ULL) == 0; shift--)
        scaled <<= 1;                   // Ensure that reciprocal fits in 32 bits
    // The AVR gcc implementation has a 32x32->64 widening multiply.
    // This is quite accurate enough, and considerably faster than full 64x64.
    reciprocal = (0x1ULL<<(32+shift)) / core_clock;
  }

  uint32_t frequencyDelta(uint32_t freq) const
  {
    // The reciprocal/16 is a rounding factor determined experimentally
    return ((uint64_t)freq * reciprocal + reciprocal/16) >> shift;
  }

  void setFrequency(uint32_t freq)
  {
    setDelta(frequencyDelta(freq));
  }

  void setDelta(uint32_t tuning_word)
  {
    SPI.beginTransaction(SPISettings(SPIRate, LSBFIRST, SPI_MODE0));
    for (int b = 0; b < 4; b++, tuning_word >>= 8)
            SPI.transfer(tuning_word&0xFF); 
    // The last byte contains configuration settings including phase:
    SPI.transfer(0x01);
    SPI.endTransaction();
    pulse(FQ_UDPin);                    // Transfer the 40-bit control word into the DDS core
  }

protected:
  void pulse(uint8_t pin)
  {
    digitalWrite(pin, HIGH);
    //delay(10);
    digitalWrite(pin, LOW);
  }
};

#endif  /* _AD9851_h_ */
