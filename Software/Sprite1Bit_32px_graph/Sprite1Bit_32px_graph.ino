// This sketch draws a rotating Yin and Yang symbol. It illustrates
// the drawing and rendering of simple animated graphics using
// a 1 bit per pixel (1 bpp) Sprite.

// Note:  TFT_BLACK sets the pixel value to 0
// Any other colour sets the pixel value to 1

// A square sprite of side = 2 x RADIUS will be created
// (80 * 80)/8 = 800 bytes needed for 1 bpp sprite
//              6400 bytes for 8 bpp
//             12800 bytes for 16 bpp



#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "Adafruit_MCP23008.h"
#include <SPI.h>
#include <Adafruit_FT6206.h>
#include "Buttons.h"


#define WAIT 0         // Loop delay

// 1bpp Sprites are economical on memory but slower to render
#define COLOR_DEPTH 1  // Colour depth (1, 8 or 16 bits per pixel)

// Rotation angle increment and start angle
#define ANGLE_INC 3


#define EX_LEDS      32
#define HEATER_PWM     4

#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander

int angle = 0;



TFT_eSPI    tft = TFT_eSPI();         // Invoke library

TFT_eSprite img = TFT_eSprite(&tft);  // Sprite class
TFT_eSprite img_arrow = TFT_eSprite(&tft);  // Sprite class

Adafruit_MCP23008 mcp;

Adafruit_FT6206 ts = Adafruit_FT6206();

TS_Point p;

// -------------------------------------------------------------------------
void setup(void)
{
    Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
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
  analogWrite(EX_LEDS,50);   // turn the LED on (HIGH is the voltage level)

  pinMode(HEATER_PWM, OUTPUT); // Excitation LEDs
  digitalWrite(HEATER_PWM, LOW);

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

 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK);
 
  img.setBitmapColor(TFT_BLACK,TFT_WHITE);
  img.setColorDepth(COLOR_DEPTH);
  
  img.createSprite(32, 32);
 img.pushImage(0,0, 32, 32, (uint16_t *)buttonSettings);
 img.pushSprite(320-40, 5);
 img.deleteSprite();

  img_arrow.setBitmapColor(TFT_BLACK,TFT_WHITE);
  img_arrow.setColorDepth(COLOR_DEPTH);
img_arrow.createSprite(40, 40);
 img_arrow.pushImage(0,0, 40, 40, (uint16_t *)buttonArrow);
 img_arrow.pushSprite(205, 70);
 img_arrow.pushSprite(205, 125);

 tft.fillRoundRect(35,70,160,40,7,TFT_BLACK);
 tft.fillRoundRect(35,125,160,40,7,TFT_BLACK);


 img.createSprite(30, 17);
 img.pushImage(0,0, 30, 17, (uint16_t *)buttonLAMP);
 img.pushSprite(155, 80);
 img.deleteSprite();

 img.createSprite(30, 17);
 img.pushImage(0,0, 30, 17, (uint16_t *)buttonPCR);
 img.pushSprite(155, 135);
 img.deleteSprite();


 

 
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (32,29);
  tft.setFreeFont(FSS12); 
  tft.print("Welcome");

  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setCursor (50,98);
  tft.setFreeFont(FSS12); 
  tft.print("qLAMP");
  
  tft.setCursor (50,153);
  tft.setFreeFont(FSS12); 
  tft.print("qPCR");


   Serial.print("Welcome");
delay(1000);
    tft.fillRect(0,0,320,240,TFT_WHITE);


  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (32,29);
  tft.setFreeFont(FSS12); 
  tft.print("Denature Temp. in degC");
  
 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK);

 tft.setTextFont(0); 
 tft.setTextSize(0);

 tft.setFreeFont(FF17); 
 int val=65;
const int tab_height=24;

 //  tft.setTextDatum(MC_DATUM);
     for (int x = 0; x < 5; x++) {

 tft.drawLine(30+x*42,50,30+x*42,220,TFT_BLACK);
 tft.drawLine(31+x*42,50,31+x*42,220,TFT_BLACK);



if (x==2){
 tft.setTextColor(TFT_BLACK,TFT_GREEN);
 tft.fillRect(32+x*42,220-val-tab_height,40,tab_height,TFT_BLACK);
  tft.fillRect(32+x*42,220-val+1-tab_height,40,tab_height-2,TFT_GREEN);

} else
{
   tft.setTextColor(TFT_WHITE,TFT_BLACK);
 tft.fillRect(32+x*42,220-val-tab_height,40,tab_height,TFT_BLACK);
  }
 tft.setFreeFont(FF17); 
 tft.setCursor (40+x*42,220-val+18-tab_height);
 tft.print(val,0);
  tft.setTextFont(0); 
   tft.setCursor (62+x*42,220-val-tab_height+3);
  tft.print("o");
  
if(x==0){
 tft.fillRect(32+x*42,220-val,40,val,TFT_GREEN);
   tft.setTextColor(TFT_BLACK,TFT_GREEN);
} else
{ tft.fillRect(32+x*42,220-val,40,val,TFT_DARKGREY);
  tft.setTextColor(TFT_BLACK,TFT_DARKGREY);
  }
  

 // tft.print(" C");
  tft.setFreeFont(FF17); 
 tft.setCursor (38+x*42,220+16);
 tft.print(70,0);
  tft.print("s");
     }


        
      
 tft.drawLine(30+5*42,50,30+5*42,220,TFT_BLACK);
 tft.drawLine(31+5*42,50,31+5*42,220,TFT_BLACK);

 tft.drawLine(31,50,31+5*42,50,TFT_BLACK);
 tft.drawLine(31,51,31+5*42,51,TFT_BLACK);
 
 tft.drawLine(31,220,31+5*42,220,TFT_BLACK);
 tft.drawLine(31,221,31+5*42,221,TFT_BLACK);

 tft.fillRect(32+1*42,220-tab_height,3*42-2,tab_height,TFT_DARKGREY);
 tft.drawLine(30+1*42,220-tab_height,30+4*42,220-tab_height,TFT_BLACK);

 

 
  tft.setPivot(280, 100);     // Set pivot to middle of TFT screen
  img_arrow.pushRotated(270);
  tft.setPivot(280, 180);     // Set pivot to middle of TFT screen
  img_arrow.pushRotated(90);


  //  img_arrow.setPivot(100,100);            // Show where screen pivot is



}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void loop() {

  delay(WAIT);
}
