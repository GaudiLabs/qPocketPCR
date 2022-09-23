// This sketch draws a rotating Yin and Yang symbol. It illustrates
// the drawing and rendering of simple animated graphics using
// a 1 bit per pixel (1 bpp) Sprite.

// Note:  TFT_BLACK sets the pixel value to 0
// Any other colour sets the pixel value to 1

// A square sprite of side = 2 x RADIUS will be created
// (80 * 80)/8 = 800 bytes needed for 1 bpp sprite
//              6400 bytes for 8 bpp
//             12800 bytes for 16 bpp

#include "ButtonSettings.h"
#define RADIUS 40      // Radius of completed symbol = 40

#define WAIT 0         // Loop delay

// 1bpp Sprites are economical on memory but slower to render
#define COLOR_DEPTH 1  // Colour depth (1, 8 or 16 bits per pixel)

// Rotation angle increment and start angle
#define ANGLE_INC 3
int angle = 0;

#include <TFT_eSPI.h>                 // Hardware-specific library

TFT_eSPI    tft = TFT_eSPI();         // Invoke library

TFT_eSprite img = TFT_eSprite(&tft);  // Sprite class


// -------------------------------------------------------------------------
void setup(void)
{
    tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_GREEN);
  tft.setTextFont(0);        // Select font 0 which is the Adafruit font

  img.setBitmapColor(TFT_BLACK,TFT_GREEN);
  img.setColorDepth(COLOR_DEPTH);
  img.createSprite(100, 100);
  img.fillSprite(TFT_BLACK);

 img.pushImage(0, 0, 100, 100, (uint16_t *)dial);
 img.pushSprite(0,0);

}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
void loop() {

  delay(WAIT);
}
