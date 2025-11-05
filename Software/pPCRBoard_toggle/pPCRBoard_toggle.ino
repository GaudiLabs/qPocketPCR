#include <Wire.h>
#include "Adafruit_MCP23008.h"

// Basic toggle test for i/o expansion. flips pin #0 of a MCP23008 i2c
// pin expander up and down. Public domain

// Connect pin #1 of the expander to Analog 5 (i2c clock)
// Connect pin #2 of the expander to Analog 4 (i2c data)
// Connect pins #3, 4 and 5 of the expander to ground (address selection)
// Connect pin #6 and 18 of the expander to 5V (power and reset disable)
// Connect pin #9 of the expander to ground (common ground)

// Output #0 is on pin 10 so connect an LED or whatever from that to ground

Adafruit_MCP23008 mcp;

#define EX_LEDS      32
#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander



  int i=0;

  
void setup() {  
  Wire.begin(26, 27);
  
  mcp.begin();      // use default address 0

  mcp.pinMode(CAM_PWR, OUTPUT); 
  mcp.digitalWrite(CAM_PWR, LOW); // LOW = POWER ON

  mcp.pinMode(TFT_BACKLIGHT, OUTPUT); 
  mcp.digitalWrite(TFT_BACKLIGHT, HIGH); //   POWER ON

  mcp.pinMode(LED1, OUTPUT); // LED1
  pinMode(EX_LEDS, OUTPUT); // Excitation LEDs

}


// flip the pin #0 up and down

void loop() {
  delay(10);
  if (i>128) mcp.digitalWrite(LED1, HIGH); else
  mcp.digitalWrite(LED1, LOW); 


  analogWrite(EX_LEDS, i);   // turn the LED on (HIGH is the voltage level)
  delay(10);                       // wait for a second
i++;
if (i>255) i=0;
}
