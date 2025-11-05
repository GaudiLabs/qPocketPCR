/*
  Analog input, analog output, serial output

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  The circuit:
  - potentiometer connected to analog pin 0.
    Center pin of the potentiometer goes to the analog pin.
    side pins of the potentiometer go to +5V and ground
  - LED connected from digital pin 9 to ground through 220 ohm resistor

  created 29 Dec. 2008
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInOutSerial
*/
#include <TFT_eSPI.h> // Hardware-specific library
#include "Adafruit_MCP23008.h"
#include <SPI.h>
#include <Adafruit_FT6206.h>


#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander


#define EX_LEDS      32
#define HEATER_PWM     4
#define FAN_PWM     12

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = 33;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

const int NTC_B = 3435;
const float NTC_TN = 298.15;
const int NTC_RN = 10000;
const float NTC_R0 = 4.7;
float temperature =0;
float sensorResistance =0;

float sensorVoltage =0;

int x=0;
TFT_eSPI    tft = TFT_eSPI();         // Invoke library

TFT_eSprite img = TFT_eSprite(&tft);  // Sprite class
TFT_eSprite img_arrow = TFT_eSprite(&tft);  // Sprite class

Adafruit_MCP23008 mcp;

Adafruit_FT6206 ts = Adafruit_FT6206();

TS_Point p;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
    analogReadResolution(12);

     Wire.begin(26, 27);

// Set Init Pins Hardware
  mcp.begin();      // use default address 0
 mcp.pinMode(TFT_RST, OUTPUT); 
    mcp.digitalWrite(TFT_RST, LOW); // TFT RESET,  LOW = RESET

  mcp.digitalWrite(TFT_RST, HIGH); // TFT RESET,  LOW = RESET

  mcp.pinMode(TFT_BACKLIGHT, OUTPUT); 
  mcp.digitalWrite(TFT_BACKLIGHT, HIGH); // HIGH = BACKLIGHT ON

  mcp.pinMode(CAM_PWR, OUTPUT); 
  mcp.digitalWrite(CAM_PWR, LOW); // LOW = POWER ON

  mcp.pinMode(CAM_PWDN, OUTPUT); 
  mcp.digitalWrite(CAM_PWDN, 0); // 0: Normal mode / 1: Power down mode

  mcp.pinMode(CAM_RST, OUTPUT); 
  mcp.digitalWrite(CAM_RST, HIGH); // System reset input, active low

  mcp.pinMode(LED1, OUTPUT); // LED1

  pinMode(EX_LEDS, OUTPUT); // Excitation LEDs

   
  analogWrite(EX_LEDS,0);   // turn the LED on (HIGH is the voltage level)

  pinMode(HEATER_PWM, OUTPUT); // Excitation LEDs
  digitalWrite(HEATER_PWM, HIGH);

  pinMode(FAN_PWM, OUTPUT); // Excitation LEDs
  digitalWrite(FAN_PWM, LOW);

   if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }
  tft.init();

  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextFont(0);        // Select font 0 which is the Adafruit font


 
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:

 sensorVoltage=3.3*sensorValue/4096;
sensorResistance=((sensorVoltage*NTC_R0)/(3.3-sensorVoltage));
temperature=  1/(log(sensorResistance*1000/NTC_RN)/NTC_B+1/NTC_TN)-273.15 ;


  // print the results to the Serial Monitor:
  Serial.print("sensor = ");
  Serial.println(temperature);
 tft.drawLine(x,240-temperature*2,x,240-temperature*2,TFT_BLACK);

x=x+1;
if (x>320) {tft.fillRect(0,0,320,240,TFT_WHITE);x=0;
  digitalWrite(HEATER_PWM, LOW);
    digitalWrite(FAN_PWM, HIGH);

}

delay(100);
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2);
}
