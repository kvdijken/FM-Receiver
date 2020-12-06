#ifndef PINS_H
#define PINS_H

// Purple labels in Arduino Nano pinout diagram.

// Pins for rotary encode

// D7
#define PIN_ENCODER_A    7

// D8
#define PIN_ENCODER_B    8



// LED Display pins

// D9
#define LED_DATA 9

// D10
#define LED_SELECT 10

// A0
#define LED_CLK 14

// IF pins

// D2
#define LED_FAST 2

// D4
#define BTN_MODE 4

// A3
#define BTN_NEXT 17

// D3
#define BTN_DEFAULT 3



// Pins for AD9851

// D11
#define SPI_DATA 11

// D13
#define SPI_CLK 13

// A1
#define AD9851_RESET_PIN  15

// A2
#define AD9851_FQ_UD_PIN  16



// Pins for Si5351

// A4
#define I2C_SDA 18

// A5
#define I2C_CLS 19



/*
   Pins in use:

   2 LED_FAST
   3 BTN_NEXT_DEFAULT
   4 BTN_MODE
   7 PIN_ENCODER_A
   8 PIN_ENCODER_B
   9 LED_DATA
   10 LED_SELECT
   11 SPI_DATA
   13 SPI_CLK
   14 LED_CLK
   15 AD9851_RESET_PIN
   16 AD9851_FQ_UD_PIN
   17 BTN_NEXT
   18 I2C_SDA
   19 I2C_CLS

   Purple labels in Arduino Nano pinout diagram.
*/

#endif
