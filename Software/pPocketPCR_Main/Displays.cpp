#include <TFT_eSPI.h> // Hardware-specific library
#include "Displays.h"
#include "Buttons.h"
#include "DesignGrafics.h"
#include "Keyboard.h"
#include "Parsing.h"
#include "USB_DRIVE.h"

#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "neuropol15pt7b.h"
#include "neuropol10pt7b.h"
#include "neuropol11pt7b.h"
#include "neuropol12pt7b.h"
#include "GaudiSans7pt7b.h"

#include "Preferences.h"
#include <Adafruit_FT6206.h>

#define TFT_BUTTONGREY    0x2104      /* 128, 128, 128 */
#define TFT_BUTTONCOLOR    0x3575
#define TFT_BUTTONGREEN    0x75ad
#define TFT_PROTOCOLCOLOR  0x91f8


const int tab_height=24;
int active_x=-1;
int active_y=-1;

int currentFrame=1;


SystemState caseUX = CASE_Main;

extern TFT_eSPI tft;

TFT_eSprite tftbuf = TFT_eSprite(&tft);  // Sprite class
TFT_eSprite icon = TFT_eSprite(&tft);  // Sprite class

extern boolean touched=false;
TS_Point tp;

extern Preferences preferences;
extern Adafruit_FT6206 ts;

extern int PCR_Times[5];
extern float PCR_Temperatures[5];
extern int PCR_Cycles;

bool PointInRect(TS_Point point, int x, int y, int h, int v)

{
  //tft.drawRect(x,y,h,v,TFT_GREEN); //Show Touch Areas

  return (point.y >= x) & (point.y <= x + h) & (240 - point.x >= y) & (240 - point.x <= y + v);
}

 void drawThickLine(int x0, int y0, int x1, int y1, uint32_t color) {
  
        tft.drawLine(x0 , y0 , x1, y1 , color);
        tft.drawLine(x0+1 , y0+1 , x1+1, y1+1 , color);
        tft.drawLine(x0+1 , y0 , x1+1, y1 , color);
        tft.drawLine(x0 , y0+1 , x1, y1+1 , color);
}


/// DISPLAY

void drawBackButton()
{
 tft.setTextColor(TFT_WHITE,TFT_BLACK);
 tft.setFreeFont(&neuropol10pt7b); 
 tft.fillRoundRect(320-52,3,45,33,12,TFT_BUTTONCOLOR);
 tft.setCursor (320-40,25);
 tft.print(char(0x80));
}
 
void drawMainDisplay()
{

 tft.fillRect(0,0,320,240,TFT_WHITE);

 icon.setBitmapColor(TFT_BUTTONGREY,TFT_WHITE);
 icon.setColorDepth(COLOR_DEPTH);
  
 icon.createSprite(253, 159);
 icon.pushImage(0,0, 253, 159, (uint16_t *)mainButtons);
 icon.pushSprite(32, 36);
 icon.deleteSprite();

//  tft.drawRect(32,36,186,92,TFT_GREEN);
//  tft.drawRect(32,138,186,56,TFT_GREEN);
//  tft.drawRect(228,71,57,57,TFT_GREEN);
//  tft.drawRect(228,138,57,57,TFT_GREEN);

   
   Serial.println("Welcome");
}


void drawProtocolDisplay()
{


  const int spacing_left=35;
  const int spacing_top=55;
  const int spacing_step=50;

 int val;
 int step_no;
 char camMark = char(0x82);
 

  tft.fillRect(0,0,320,240,TFT_WHITE);
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setFreeFont(&GaudiSans7pt7b); 
  
  tft.setCursor (spacing_left,20);
  tft.print("Protocol: ");
  tft.print(pcrProtocol.name.substring(0,25));

  tft.setCursor (spacing_left,35);
  tft.print("Date: ");
  tft.print(pcrProtocol.date);
  tft.setTextDatum(MC_DATUM);


 // if(pcrProtocol.stepCount<=(currentFrame-1)*5) currentFrame=pcrProtocol.stepCount/5+1;
int noColloms=pcrProtocol.stepCount-(currentFrame-1)*5;
if (noColloms>5) noColloms=5;


 drawThickLine(spacing_left,30+spacing_top+100-pcrProtocol.steps[(currentFrame-1)*5].temperature,spacing_left+1*30,30+spacing_top+100-pcrProtocol.steps[(currentFrame-1)*5].temperature,TFT_PROTOCOLCOLOR);

for (int colNo = 0; colNo < noColloms; colNo++) {
step_no=(currentFrame-1)*5+colNo;

 tft.drawLine(spacing_left+colNo*spacing_step,spacing_top,spacing_left+colNo*spacing_step,spacing_top+160,TFT_DARKGREY);
 tft.drawLine(spacing_left+colNo*spacing_step,spacing_top,spacing_left+colNo*spacing_step,spacing_top+160,TFT_DARKGREY);
 tft.drawString(String(step_no+1),spacing_left+spacing_step/2+colNo*spacing_step,spacing_top+6);
  
 tft.setCursor (spacing_left+10+colNo*spacing_step,spacing_top+160-6);
 
 if ((pcrProtocol.steps[step_no].duration)<90)
 {tft.drawString(String(pcrProtocol.steps[step_no].duration,0)+"s", spacing_left+spacing_step/2+colNo*spacing_step,spacing_top+160-10);} else
 {tft.drawString(String(pcrProtocol.steps[step_no].duration/60,0)+"m", spacing_left+spacing_step/2+colNo*spacing_step,spacing_top+160-10);}
 

 drawThickLine(spacing_left+colNo*spacing_step+5,30+spacing_top+100-pcrProtocol.steps[step_no].temperature,spacing_left+(colNo+1)*spacing_step,30+spacing_top+100-pcrProtocol.steps[step_no].temperature,TFT_PROTOCOLCOLOR);
 if(colNo<(noColloms-1)) drawThickLine(spacing_left+(colNo+1)*spacing_step,30+spacing_top+100-pcrProtocol.steps[step_no].temperature,spacing_left+(colNo+1)*spacing_step+5,30+spacing_top+100-pcrProtocol.steps[step_no+1].temperature,TFT_PROTOCOLCOLOR);
  
  tft.drawString(String(pcrProtocol.steps[step_no].temperature,1)+"°C",spacing_left+colNo*spacing_step+spacing_step/2+3,spacing_top+118-pcrProtocol.steps[step_no].temperature);

 
  if(pcrProtocol.steps[step_no].capture) 
  { tft.drawString(String(camMark),spacing_left+colNo*spacing_step+spacing_step/2,30+spacing_top+100-pcrProtocol.steps[step_no].temperature-26); }

  
  if ((step_no>=pcrProtocol.repeatStart-1)&&(step_no<=pcrProtocol.repeatEnd-1)) 
  {tft.fillRect(spacing_left+colNo*spacing_step,spacing_top+130-10,spacing_step,20,TFT_PROTOCOLCOLOR);
   tft.setTextColor(TFT_WHITE,TFT_PROTOCOLCOLOR);
   tft.drawString(String(pcrProtocol.cycleCount)+"x",spacing_left+colNo*spacing_step+spacing_step/2,spacing_top+130-1);

   tft.setTextColor(TFT_BLACK,TFT_WHITE);
  }
 
  }

   tft.drawLine(spacing_left+noColloms*spacing_step,spacing_top,spacing_left+noColloms*spacing_step,spacing_top+160,TFT_DARKGREY);
   tft.drawLine(spacing_left,spacing_top+160,spacing_left+noColloms*spacing_step,spacing_top+160,TFT_DARKGREY);



   tft.setTextColor(TFT_WHITE,TFT_BLACK);
   tft.setFreeFont(&neuropol10pt7b); 
   tft.fillRoundRect(320-52,8,45,37,12,TFT_BUTTONCOLOR);
   tft.setCursor (320-40,32);
   tft.print(char(0x80));

 

   tft.setFreeFont(&neuropol15pt7b); 
   tft.setTextColor(TFT_BUTTONCOLOR,TFT_WHITE);

 //tft.fillRect(0, 75, 50, 70,TFT_BLACK);
 //tft.fillRect(270, 75, 50, 70,TFT_BLACK);

if (pcrProtocol.stepCount>currentFrame*5)
  {
 tft.setCursor (320-25,120);
 tft.print(">");
  }
 if (currentFrame>1)
 { tft.setCursor (5,120);
 tft.print("<");
 }



 
} // setup Display

void setPrototcolDisplayFrame(int frame)
{
  currentFrame=frame;
}

void runProtocolDisplay()
{  

TS_Point p;

if(ts.touched()) {touched=true;p=ts.getPoint();if(p.y>0) tp = p;}

  if (!ts.touched()&&touched) {
  touched=false;
  
  
  if (PointInRect(tp, 320-60, 1, 60, 50))  // BACK
  {
  drawMainDisplay();
  caseUX=CASE_Main;
  }

  if (PointInRect(tp, 0, 75, 50, 70)&&(currentFrame>1))  // <
  {
currentFrame--;
drawProtocolDisplay();
  }

  if (PointInRect(tp, 270, 75, 50, 70)&&(pcrProtocol.stepCount>currentFrame*5))  // >
  {
currentFrame++;
drawProtocolDisplay();
  }

  
 }

} // runPCRSetp


void drawSubMenuDisplay()
{
  tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);

  tft.setFreeFont(&neuropol12pt7b); 
  
 tft.fillRoundRect(30,20+0*52,260,45,12,TFT_BUTTONGREY);
 tft.drawString("MEASURE",320/2,40+0*52);

 tft.setFreeFont(&neuropol12pt7b); 
 tft.fillRoundRect(30,20+1*52,260,45,12,TFT_BUTTONGREY);
 tft.drawString("..",320/2,40+1*52);
 
 tft.setFreeFont(&neuropol12pt7b); 
 tft.fillRoundRect(30,20+2*52,260,45,12,TFT_BUTTONGREY);
 tft.setCursor (60,50+2*52);
 tft.drawString("..",320/2,40+2*52);

  tft.setFreeFont(&neuropol10pt7b); 

 tft.setTextColor(TFT_WHITE,TFT_BUTTONCOLOR);
 tft.fillRoundRect(130,20+3*52,60,45,12,TFT_BUTTONCOLOR);
 tft.drawString(String(char(0x80)),320/2,38+3*52);

 
} //drawSubMenuDisplay


void drawSettingsDisplay()
{
  tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);

  tft.setFreeFont(&neuropol12pt7b); 
  
 tft.fillRoundRect(30,20+0*52,260,45,12,TFT_BUTTONGREY);
 tft.drawString("BASELINE",320/2,40+0*52);

 tft.fillRoundRect(30,20+1*52,260,45,12,TFT_BUTTONGREY);
 tft.drawString("ALIGNEMENT",320/2,40+1*52);
  
 tft.fillRoundRect(30,20+2*52,260,45,12,TFT_BUTTONGREY);
 tft.setCursor (60,50+2*52);
 tft.drawString("INITIALIZE USB",320/2,40+2*52);

  tft.setFreeFont(&neuropol10pt7b); 

 tft.setTextColor(TFT_WHITE,TFT_BUTTONCOLOR);
 tft.fillRoundRect(130,20+3*52,60,45,12,TFT_BUTTONCOLOR);
 tft.drawString(String(char(0x80)),320/2,38+3*52);

 
} //drawSettingsDisplay


void runSubMenuDisplay()
{

TS_Point p;

if(ts.touched()) {touched=true;p=ts.getPoint();if(p.y>0) tp = p;}

  if (!ts.touched()&&touched) {
  touched=false;
  
  if (PointInRect(tp, 30,0*52,260,65))  // MEASURE
  {
   tft.fillRect(0,0,320,240,TFT_WHITE);
   drawMeasurementDisplay();
   caseUX = CASE_InitMeasurement;
  }

  if (PointInRect(tp, 30,20+1*52,260,45))  // MELTING CURVE
  {       
  
  }

  if (PointInRect(tp, 30,20+2*52,260,45))  // ..
  {
 

  }

if (PointInRect(tp, 110,20+3*52,100,64))  // BACK
  {
  drawMainDisplay();
  caseUX=CASE_Main;
  }


  }
} // runSubMenuDisplay


void runSettingsDisplay()
{
TS_Point p;

if(ts.touched()) {touched=true;p=ts.getPoint();if(p.y>0) tp = p;}

  if (!ts.touched()&&touched) {
  touched=false;
  
  if (PointInRect(tp, 30,0*52,260,65))  // BASELINE
  {
   drawBaselineDisplay();
   caseUX = CASE_InitBaseline;
  }

  if (PointInRect(tp, 30,20+1*52,260,45))  // CALIBRATE
  {       
   drawCalibrationDisplay();
   caseUX = CASE_InitCalibration;
  }

  if (PointInRect(tp, 30,20+2*52,260,45))  // INITIALIZE USB
  {

  drawInitUSBDisplay();
  caseUX = CASE_RunInitUSB;

 // InitializeUSB();

  }
  
  if (PointInRect(tp, 110,20+3*52,100,64))  // BACK
  {
  drawMainDisplay();
  caseUX=CASE_Main;
  }


  }
} // runSettingslDisplay
void draw_qPCR_display()
{
  
  }


void drawCancelDisplay()
{
  tft.fillRect(0,0,320,240,TFT_WHITE);
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setFreeFont(&neuropol12pt7b); 
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CANCEL RUN?",320/2,50);

  tft.setFreeFont(&neuropol10pt7b); 
  tft.setTextColor(TFT_WHITE,TFT_BUTTONCOLOR);
  tft.fillRoundRect(130,20+3*52,60,45,12,TFT_BUTTONCOLOR);
  tft.drawString(String(char(0x80)),320/2,38+3*52);



  caseUX = CASE_RunComplete;

}



void drawInitUSBDisplay()

{
 tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);
  
 // drawBackButton();
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (15,29);
  tft.setFreeFont(&neuropol10pt7b); 

  tft.print("Initialize USB");

 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK); 

 tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);

 tft.fillRoundRect(20,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("CANCEL",85,200);

 tft.fillRoundRect(170,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("FORMAT",235,200);

 tft.setTextColor(TFT_BLACK, TFT_WHITE);
 tft.setFreeFont(&GaudiSans7pt7b); 
 tft.setCursor (0,70);

 uint32_t SPIFFS_freeBytes = (SPIFFS.totalBytes() - SPIFFS.usedBytes());

 tft.print("       Total Drive: ");
 tft.print(SPIFFS.totalBytes()/1000);
 tft.println(" kB");

 tft.print("       Used Space: ");
 tft.print(SPIFFS.usedBytes()/1000);
 tft.println(" kB");
 
 tft.print("       Free Space: ");
 tft.print(SPIFFS_freeBytes/1000);
 tft.println(" kB");

 
  }


void drawBaselineDisplay()

{
 tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);
  
 // drawBackButton();
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (15,29);
  tft.setFreeFont(&neuropol10pt7b); 

  tft.print("Read Baseline");

 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK); 

 tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);

 tft.fillRoundRect(20,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("CANCEL",85,200);

 tft.fillRoundRect(170,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("SAVE",235,200);
 

 
  }


void drawInitRunDisplay(float voltage)
{


   tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);
  
 // drawBackButton();
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (15,29);
  tft.setFreeFont(&neuropol10pt7b); 

  tft.print("Start Run");

 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK); 


 tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
 tft.fillRoundRect(20,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("CANCEL",85,200);

 tft.setTextColor(TFT_BLACK, TFT_WHITE);
 tft.setFreeFont(&GaudiSans7pt7b); 
 tft.setCursor (0,70);


  tft.setCursor (0,60);
  tft.print("    Protocol: ");
  tft.println(pcrProtocol.name.substring(0,25));
   
 tft.print("    USB-C Voltage: ");
 tft.print(String(voltage));
 tft.println(" V");

if (voltage<6){

    tft.setCursor (00,120);
 tft.setTextColor(TFT_RED, TFT_WHITE);
 tft.println("    \x87 USB-C Voltage too low to run.");
 tft.println("    Connect using a Power Delivery USB plug (PD)");

 tft.setFreeFont(&neuropol10pt7b); 
 tft.setTextColor(TFT_WHITE,0xad55);
 tft.fillRoundRect(170,180,130,45,12,0xad55);
 tft.drawString("RUN",235,200);
 
  } else 

  {    tft.setCursor (00,120);
   tft.setTextColor(TFT_DARKGREEN, TFT_WHITE);

     tft.println("    \x87 Starting a new run will delet existing");
     tft.print("    data on the device.");
     
 tft.setFreeFont(&neuropol10pt7b); 
 tft.setTextColor(TFT_WHITE,TFT_DARKGREEN);
 tft.fillRoundRect(170,180,130,45,12,TFT_DARKGREEN);
 tft.drawString("RUN",235,200);
    }
 
  }


  
void drawCalibrationDisplay()

{
 tft.fillRect(0,0,320,240,TFT_WHITE);

  tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);
  tft.setTextDatum(MC_DATUM);
  
 // drawBackButton();
  
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor (15,29);
  tft.setFreeFont(&neuropol10pt7b); 

  tft.print("Camera Alignement");

 tft.drawLine(0,40,320,40,TFT_BLACK);
 tft.drawLine(0,41,320,41,TFT_BLACK); 

 tft.setTextColor(TFT_WHITE,TFT_BUTTONGREY);

 tft.fillRoundRect(20,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("CANCEL",85,200);

 tft.fillRoundRect(170,180,130,45,12,TFT_BUTTONGREY);
 tft.drawString("SAVE",235,200);
 
 tft.setTextColor(TFT_BLACK, TFT_WHITE);
 tft.setFreeFont(&GaudiSans7pt7b); 
 tft.setCursor (90,154);
 tft.print("Record well factors");

 
  }

void drawMeasurementDisplay()
{
 tft.fillRect(0,0,320,(240-60),TFT_WHITE);

 drawBackButton();
     
  //draw grid
   for (int x = 0; x < 21; x++) {
      tft.drawLine(x*(tft.width()-40)/20+20, (tft.height()-70), x*(tft.width()-40)/20+20,tft.height()-60-140,TFT_LIGHTGREY);
      }
      for (int y = 0; y < 14; y++) {
      tft.drawLine(20,tft.height()-70-10*y,tft.width()-20,tft.height()-70-10*y,TFT_LIGHTGREY);
    }

}


void status_line(String printtext, bool status_flag, String extra_text)

{
  
const int line_height=14;
const int margin_left=20;

char checkMark = char(0x80);
char errorMark = char(0x81);

  tft.setCursor (margin_left,tft.getCursorY());

if (status_flag){
 tft.setTextColor(TFT_RED, TFT_WHITE);
 tft.print(" ");
 tft.print(char(errorMark));
 tft.print(" ");
  } else
  {
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
 tft.print(" ");
 tft.print(char(errorMark));
 tft.print(" ");
    }
  
   tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.print(" ");
  tft.print(printtext);

  if (extra_text!=""){
  tft.print(", ");
  tft.print(extra_text);
  }
  tft.setCursor (margin_left,tft.getCursorY()+line_height);
  }
