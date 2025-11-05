#ifndef TLC59108_h
#define TLC5910_h

#include "Arduino.h"
#include "Wire.h"

class TLC59108
{
  public:
    
    // CONSTRUCTORS
    TLC59108(byte addr);                // Constructor (uses default Wire*)
    TLC59108(TwoWire *i2c, byte addr);  // Constructor (Wire explicitly specified for microcontrollers with multiple I2C peripherals)


    // FUNCTIONS
    uint8_t init(const uint8_t hwResetPin = 0);                                                     // Initializes the driver by performing a hardware reset (if pin is specified) and enabling the oscillator
    uint8_t setLedOutputMode(const uint8_t outputMode);                                             // Sets all channels to the output modes listed in REGISTER::LED_MODE
    uint8_t readRegister(const uint8_t reg) const;                                                  // Reads a single register; returns -1 on error	
    uint8_t readRegisters(uint8_t *dest, const uint8_t startReg, const uint8_t num) const;          // Reads multiple registers into the specified array and returns the number of bytes successfully read
    bool    readAllBrightness(uint8_t dutyCycles[]) const;                                          // Reads brightness values from chip. Requires an array of size 8
    uint8_t setRegister(const uint8_t reg, const uint8_t value);                                    // Writes a value into a single register
    uint8_t setRegisters(const uint8_t startReg, const uint8_t values[], const uint8_t numValues);  // writes values into multiple registers
    uint8_t setBrightness(const uint8_t pwmChannel, const uint8_t dutyCycle);                       // Set brightness for specified channel
    uint8_t setAllBrightness(const uint8_t dutyCycle);                                              // Set brightness for all channels with one value
    uint8_t setAllBrightnessArray(const uint8_t dutyCycles[]);                                      // Set brightness for all channels with descrete values using an array of size 8


    // STATIC VARIABLES
	  static const byte NUM_CHANNELS = 8;

    // I2C addresses
    struct I2C_ADDR
    {
      static const byte BASE      = 0x40;
      static const byte SWRESET   = 0x4b;
      static const byte ALLCALL   = 0x48;
      static const byte SUB1      = 0x49;
      static const byte SUB2      = 0x4a;
      static const byte SUB3      = 0x4c;
    };

    // Register auto-increment modes for setting multiple registers
    struct AUTO_INCREMENT
    {
      static const byte ALL       = 0x80; // Increment through all registers (for initial setup)
      static const byte IND       = 0xa0; // Increment through individual brightness registers
      static const byte GLOBAL    = 0xc0; // Increment through global control registers
      static const byte INDGLOBAL = 0xe0; // Increment through individual and global registers
    };

    struct LED_MODE
    {
      static const byte OFF         = 0;
      static const byte FULL_ON     = 1;
      static const byte PWM_IND     = 2;
      static const byte PWM_INDGRP  = 3;
    };

    // Register names
    struct REGISTER
    {
    public:
      struct MODE1
      {
        static const byte ADDR      = 0x00;
        static const byte OSC_OFF   = 0x10;
        static const byte SUB1      = 0x08;
        static const byte SUB2      = 0x04;
        static const byte SUB3      = 0x02;
        static const byte ALLCALL   = 0x01;
      };

      struct MODE2
      {
        static const byte ADDR      = 0x01;
        static const byte EFCLR     = 0x80;
        static const byte DMBLNK    = 0x20;
        static const byte OCH       = 0x08;
      };

      struct PWM0       { static const byte ADDR = 0x02; };
      struct PWM1       { static const byte ADDR = 0x03; };
      struct PWM2       { static const byte ADDR = 0x04; };
      struct PWM3       { static const byte ADDR = 0x05; };
      struct PWM4       { static const byte ADDR = 0x06; };
      struct PWM5       { static const byte ADDR = 0x07; };
      struct PWM6       { static const byte ADDR = 0x08; };
      struct PWM7       { static const byte ADDR = 0x09; };

      struct GRPPWM     { static const byte ADDR = 0x0a; };
      struct GRPFREQ    { static const byte ADDR = 0x0b; };

      struct LEDOUT0    { static const byte ADDR = 0x0c; };
      struct LEDOUT1    { static const byte ADDR = 0x0d; };

      struct SUBADR1    { static const byte ADDR = 0x0e; };
      struct SUBADR2    { static const byte ADDR = 0x0f; };
      struct SUBADR3    { static const byte ADDR = 0x10; };

      struct ALLCALLADR { static const byte ADDR = 0x11; };

      struct IREF
      {
        static const byte ADDR = 0x12;
        static const byte CM = 0x80; // current multiplier
        static const byte HC = 0x40; // subcurrent
      };

      struct EFLAG      { static const byte ADDR = 0x13; };

    };

    struct ERROR        { static const byte ERR = 2; };


  private:
    uint8_t _addr;
    TwoWire *_i2c;

};

#endif
