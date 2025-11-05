#include "Wire.h"
#include "TLC59108.h"

#define ADDR  0x47            // Address of TLC59108 (default is 0x40)
#define SDA0  4               // SDA pin of RPi Pico
#define SCL0  5               // SCL pin of RPi Pico
#define TLC59108_HWRESET 15   // Pin wired to TLC59108 reset

arduino::MbedI2C I2C0(SDA0,SCL0);

TLC59108 leds(&I2C0, ADDR); // Define TLC59108 object using I2C pointer and slave address

void setup() {

  I2C0.begin(); // Begin chosen interface

  leds.init(TLC59108_HWRESET);
  leds.setLedOutputMode(TLC59108::LED_MODE::PWM_IND);

}

void loop(){
  blink();
}

void blink() {

  // Blink each channel on and off one by one
  for (byte i=0; i<8; i++)
  {
    leds.setBrightness(i-1,0);
    leds.setBrightness(i,100);
    delay(200);
  }
  
  leds.setAllBrightness(0);

}
