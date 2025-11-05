// This project is done by Teach Me Something

#include <TFT_eSPI.h> // Hardware-specific library
#include "Keyboard.h"
#include "Buttons.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "Roboto_Medium7pt7b.h"

#include "Preferences.h"
#include <Adafruit_FT6206.h>

extern TFT_eSPI tft;

extern Preferences preferences;

extern Adafruit_FT6206 ts;

/*______End of Libraries_______*/

/*______Assign names to colors */
#define BLACK 0x0000
#define BROWN 0x7980
#define RED 0xF800
#define ORANGE 0xFBE0
#define YELLOW 0xFFE0
#define GREEN 0x07E0
#define BLUE 0x001F
#define VIOLET 0xA81F
#define GREY 0xC618
#define WHITE 0xFFFF
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define LGREEN 0xAFE0

/*_______Assigned______*/

// Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //Start communication with LCD

boolean Caps = false;
String symbol[3][4][10] = {
    {{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
     {"Q", "W", "E", "R", "T", "Z", "U", "I", "O", "P"},
     {"A", "S", "D", "F", "G", "H", "J", "K", "L", "x"},
     {"x", "Y", "X", "C", "V", "B", "N", "M", ".", "x"}},
    {{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
     {"q", "w", "e", "r", "t", "z", "u", "i", "o", "p"},
     {"a", "s", "d", "f", "g", "h", "j", "k", "l", "x"},
     {"x", "y", "x", "c", "v", "b", "n", "m", ",", "x"}},
    {{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
     {"~", "!", "@", "#", "$", "%", "^", "&", "*", "\'"},
     {"-", "_", "=", "+", "_", "{", "}", "(", ")", "x"},
     {"x", "\\", "/", "<", ">", "?", "\"", ";", ":", "x"}}};

int X, Y;
int symbolSet = 0;

String InputText = "";

TS_Point waitTouch()
{
  TS_Point p;

  while (!ts.touched())
    ;
  p = ts.getPoint();

  // X = map(p.y, 0, 320, 0, 320);
  // Y = map(p.x, 0, 240, 240, 0);

  X = (p.y - 1) / 32;
  Y = (240 - p.x - 66) / 35;
  // tft.fillRoundRect((i*32)+1, j*35+101, 31, 34, 3,WHITE);

  Serial.print(X);
  Serial.print(',');
  Serial.println(Y); // + " " + Y);
  return p;
}

void draw_BoxNButtons()
{
  int x = 0;
  int y = 0;
  int c = 0;

  tft.fillScreen(WHITE);
  tft.fillRect(0, 65, 320, 175, BLACK);
  tft.setFreeFont(FF17);

  tft.setTextSize(0);
  tft.setTextColor(BLACK);
  for (y = 0; y < 4; y++)
  {
    for (x = 0; x < 10; x++)
    {
      tft.fillRoundRect((x * 32) + 1, y * 35 + 66, 31, 34, 3, WHITE);
      tft.setCursor(x * 32 + 9, y * 35 + 88);
      tft.print(symbol[symbolSet][y][x]);
    }
  }
  if (symbolSet == 1)
    tft.fillRoundRect(1, 171, 31, 34, 3, GREEN);
  else
    tft.fillRoundRect(1, 171, 31, 34, 3, GREY);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 195);
  tft.print("^");

  if (symbolSet == 2)
    tft.fillRoundRect(289, 171, 31, 34, 3, GREEN);
  else
    tft.fillRoundRect(289, 171, 31, 34, 3, GREY);
  tft.setTextColor(BLACK);
  tft.setTextSize(0);
  tft.setCursor(290, 185);
  tft.print("!?");

  tft.fillRoundRect(289, 136, 31, 34, 3, GREY);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.setCursor(294, 150);
  tft.print("<");

  tft.fillRoundRect(1, 206, 223, 34, 3, WHITE);
  tft.setTextColor(BLACK);
  tft.setTextSize(0);
  tft.setCursor(55, 228);
  tft.print("Space Bar");

  tft.fillRoundRect(225, 206, 95, 34, 3, GREY);
  tft.setTextColor(BLACK);
  tft.setTextSize(0);
  tft.setCursor(247, 228);
  tft.print("ENTER");
}

boolean DetectButtons()
{

  if (Y >= 0)
    if (X > 6 && Y == 4)
      return true;
    else if (X == 9 && Y == 2)
      InputText.remove(InputText.length() - 1, 1);
    else if (X == 0 && Y == 3)
    {
      if (symbolSet == 0)
        symbolSet = 1;
      else
        symbolSet = 0;
      draw_BoxNButtons();
    }
    else if (X == 9 && Y == 3)
    {
      if (symbolSet == 2)
        symbolSet = 0;
      else
        symbolSet = 2;
      draw_BoxNButtons();
    }
    else if (tft.getCursorX() < 300)
      if (X < 7 && Y == 4)
        InputText = InputText + " ";
      else if (X >= 0 && X <= 9 && Y >= 0 && Y <= 4)
        InputText.concat(symbol[symbolSet][Y][X]);

  tft.fillRect(0, 0, 320, 65, WHITE);
  tft.setCursor(10, 30);
  tft.print(InputText + "_");
  return false;
}

String keyboard()
{

  int i = 0;
  int j = 0;
  int a = 0;
  int b = 0;
  boolean enter = false;

  InputText = "";
  draw_BoxNButtons();
  tft.setTextSize(0);
  tft.setTextColor(BLACK);
  tft.setCursor(10, 30);
  tft.print(InputText + "_");

  // loop
  while (!enter)
  {
    TS_Point p = waitTouch();
    // tft.setCursor (a,b);

    enter = DetectButtons();
    delay(200);
  }
  return InputText;
}
