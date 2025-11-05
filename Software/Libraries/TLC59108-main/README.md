# TLC59108 Library for Arduino Core RP2040 (Raspberry Pi Pico)

This library is a fork of Chrylis's original Arduino library https://github.com/chrylis/tlc59108

It has been modified to work with the new Mbed based ArudinoCore for RP2040.

Has been tested using a Raspberry Pi Pico and Arduino IDE 2.0.0-beta.5.

This is my first fork of a library, apologies for any mistakes!

Main changes:

- Used pointers to pass desired I2C perhipheral interface to the library.
- I2C setup in example sketch using arduino::MbedI2C i2c0(SDA0,SCL0); allowing for multiple I2C interfaces and any compatible pins to be used.
- Used convention to add underscore infront of private variables such as _i2c and _addr
- Changed variable name from EINVAL to ERR due to name clash
- Removed ambiguity error between setAllBrightness(const uint8_t dutyCycle) and setAllBrightness(const uint8_t dutyCycles[]) by changing second function name to setAllBrightnessArray
- Changed name of function getAllBrightness to readAllBrightness to improve ledgibility when used near setAllBrightness
