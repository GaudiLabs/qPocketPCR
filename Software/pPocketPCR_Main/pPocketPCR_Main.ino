// qPocketPCR Software
// by Urs Gaudenz GaudiLabs 2025

// Included Libraries
#include "USB.h"
#include "USBMSC.h"
#include "FS.h"
#include "SPIFFS.h"
#include "USB_DRIVE.h"
#include "Parsing.h"

#include "esp_camera.h"
#include "Displays.h"
#include "Keyboard.h"
#include "TLC59108.h"

#include <Adafruit_TLA202x.h>

#include <TFT_eSPI.h> // Display specific library
#include "Free_Fonts.h"
#include "GaudiSans7pt7b.h"
#include "neuropol10pt7b.h"


#include <SPI.h>
#include <Adafruit_FT6206.h>

#include "Buttons.h"
#include "Preferences.h"

#include "esp_task_wdt.h"

#include <SPIFFS.h>


// ==================== CONSTANTS AND CONFIGURATION ====================

// Version and system configuration
#define VERSION_STRING  "V1.1"
#define FORMAT_SPIFFS_IF_FAILED true
#define FILESYSTEM SPIFFS


// Hardware pin definitions

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      7
#define SIOD_GPIO_NUM     17
#define SIOC_GPIO_NUM     16

#define Y9_GPIO_NUM       41
#define Y8_GPIO_NUM       39
#define Y7_GPIO_NUM       40
#define Y6_GPIO_NUM       5
#define Y5_GPIO_NUM       2
#define Y4_GPIO_NUM       4
#define Y3_GPIO_NUM       3
#define Y2_GPIO_NUM       1
#define VSYNC_GPIO_NUM    9
#define HREF_GPIO_NUM     8
#define PCLK_GPIO_NUM     6

#define HEATER_PWM_PIN  21
#define LID_PWM_PIN     42
#define FAN_PWM_PIN     33
#define TFT_BACKLIGHT  18

// PWM channels
#define FAN_CHANNEL 7  // Channel 0 and 1 used by camera?
#define HEATER_CHANNEL 6
#define LID_CHANNEL 5

// ADC channels
#define ADC_BlockTemp 0
#define ADC_LidTemp 1
#define ADC_VBus 2
#define ADC_VADC 3

// SDA/SCL
#define TLC59108_ADDR  0x40       // Address of TLC59108 (default is 0x40)
#define SDA_PIN  17               // SDA pin 
#define SCL_PIN  16               // SCL pin 
#define TLC59108_HWRESET -1       //  TLC59108 reset 


// PCR parameters
#define PIDp 0.4       //0.42
#define PIDi 0.000    //0.004 // 0.001
#define PIDd 0.02     //0.02//0.1
#define TEMPiMax 1500


#define PIDLIDp 0.4       //0.42 0.15 0.18
#define PIDLIDi 0.002    //0.004 // 0.001
#define PIDLIDiRange 10     
#define TEMPLIDiMax 1500

// System parameters
#define ILLUMINATION_TIME 850  //650 min

#define LidTemp 110 //110

#define CurrentMax 1.7
#define LidCurrentMax 1.3
#define BlockCurrentMax 2.1
#define FanCurrentMax 0.3

#define pwmFreqFAN  6000  // PWM frequency in Hz (above audible range)
#define pwmFreqHEATER  1000  // PWM frequency in Hz 
#define pwmFreqLID  1000  // PWM frequency in Hz 

#define pwmResolution 8  // 8-bit resolution (values from 0 to 255)

#define SENS_WIDTH  640
#define SENS_HEIGHT  120

#define MASK_THRESHOLD 40

#define MAX_MEASUREMENTS 80

#define SAFETY_MIN_TEMP 5
#define SAFETY_MAX_TEMP 130


// ==================== ENUMS AND STRUCTURES ====================


enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_TEMP_SENSOR = 1,
    ERROR_WATCHDOG = 2,
};

enum PCRState {
PCR_HEATLID,
PCR_SET,
PCR_TRANSITION,
PCR_TIME,
PCR_END
};

static const uint16_t my_palette[] PROGMEM = {

  TFT_RED,      //  0
  TFT_ORANGE,   //  1
  TFT_BROWN,    //  2
  TFT_DARKGREEN,    //  3
  TFT_BLUE,     //  4
  TFT_PURPLE,   //  5
  TFT_CYAN,     //  6
  TFT_MAGENTA,  //  7
  TFT_WHITE,    //  9
  //TFT_MAROON,   // 12  Darker red colour
  //TFT_DARKGREEN,// 13  Darker green colour
  //TFT_NAVY,     // 14  Darker blue colour
  //TFT_PINK      // 15
};


// ==================== Constants ====================


const int NTC_LID_B = 3435;
const float NTC_TN = 298.15;
const int NTC_RN = 10000;
const float NTC_R0 = 4.7;

float NTC_A = 1.1235e-3;
float NTC_B = 2.3510e-4;
float NTC_C = 8.3301e-8;
float logR;

const int SENS_OFFSET_X = 100; //46
const int SENS_OFFSET_Y = 240; //234 210
const float SENS_SPACING = 65; //73

const float TEMP_TOLLERANCE = 0.5;

const int NUM_SENSORS = 8;

const int MEASUREMENT_INTERVAL_MS = 200;
const int TEMP_CHECK_INTERVAL_MS = 1000;
const int WD_TIMEOUT_S = 5;


// ==================== Variables ====================


ErrorCode errorCode=ERROR_NONE;

int x_pos = 0;

String myFileName="/DATA.TXT";

int sensorValue = 0;        // value read from the sesnor
int outputValue = 0;        // value output to the PWM (heater)
float temperature = 0;
float temperature_lid = 0;

float temperature_mean = 0;
float temperature_lid_mean = 0;


float sensorResistance = 0;
float sensorVoltage = 0;

int PCRpwm = 0;

float TEMPset;
float TEMPdif;
float TEMPi;
float TEMPLIDi;

bool PIDIntegration = false;
float TEMPcontrol;
float TEMPLIDcontrol;
float TEMPdif_a[10];
float TEMPd;
long TEMPclick = 0;
long TIMEclick = 0;
int TIMEcontrol = 0;

float TEMPLIDset;
float TEMPLIDdif;

int ledBrightness = 225; //3.3k Resistor

float fluorescence[NUM_SENSORS][MAX_MEASUREMENTS];
float wellFactor[NUM_SENSORS];

float last_values[NUM_SENSORS];
long sensor_center_x[NUM_SENSORS];
long sensor_center_y[NUM_SENSORS];

int captures=0;
int measurements=0;

unsigned long myTime_measure;
unsigned long myTime_WD;

int measure_interval = 0;


float PCR_Temperatures[5];
int PCR_TIMEs[5];
int PCR_Cycles;

PCRState casePCR = PCR_HEATLID;
int MenuItem = 1;
int PCRstep = 0;
int PCRcycle = 1;

boolean saveReady=false;
boolean cameraOn=false;

boolean touch=false;
boolean saveWellFactors=false;

TS_Point tpoint;

int i;

esp_err_t err=0;

Preferences preferences; // Initialize Preference Manager


TFT_eSPI tft = TFT_eSPI();  // Invoke display library with default width and height
TFT_eSprite camSprite = TFT_eSprite(&tft); // Define Cam Sprite
TFT_eSprite tftbuff = TFT_eSprite(&tft);  // Sprite class


Adafruit_FT6206 ts = Adafruit_FT6206(); // Invoke Touch library
TS_Point p;   //Touch point

Adafruit_TLA202x  tla; // Invoke Analog to Digital Converter

TLC59108 leds(&Wire, TLC59108_ADDR); // Define TLC59108 object for LED Driver using I2C pointer and slave address

sensor_t * cam_sens; // Camera handler
esp_err_t res = ESP_OK; //Camera error handler

uint16_t *camSpriteBuf ;
uint8_t *baseBuf;
boolean *maskBuf;


// ==================== SETUP ====================

void setup() {

// Initialize WDT with 5 second timeout, reset on trigger
    esp_task_wdt_init(WD_TIMEOUT_S, true);

// Add current task to WDT
    esp_task_wdt_add(NULL);

    
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  // Set Init Pins Hardware

  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  pinMode(HEATER_PWM_PIN, OUTPUT);
  digitalWrite(HEATER_PWM_PIN, LOW);

  pinMode(FAN_PWM_PIN, OUTPUT);
  digitalWrite(FAN_PWM_PIN, LOW);

  pinMode(LID_PWM_PIN, OUTPUT);
  digitalWrite(LID_PWM_PIN, LOW);
  

  // Set the PWM properties
  ledcSetup(FAN_CHANNEL, pwmFreqFAN, pwmResolution);  // Channel 0, Frequency, Resolution
  ledcAttachPin(FAN_PWM_PIN, FAN_CHANNEL);  // Attach the PWM signal to the pin

  ledcSetup(HEATER_CHANNEL, pwmFreqHEATER, pwmResolution);  // Channel 0, Frequency, Resolution
  ledcAttachPin(HEATER_PWM_PIN, HEATER_CHANNEL);  // Attach the PWM signal to the pin

  ledcSetup(LID_CHANNEL, pwmFreqLID, pwmResolution);  // Channel 0, Frequency, Resolution
  ledcAttachPin(LID_PWM_PIN, LID_CHANNEL);  // Attach the PWM signal to the pin


  // Init I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  

// Init TFT and Sprites
// camSprite.setAttribute(PSRAM_ENABLE, false);

  tft.init();
  camSpriteBuf = (uint16_t*)camSprite.createSprite(SENS_WIDTH/2, SENS_HEIGHT/2);
  //baseBuf = (uint16_t*)baseSprite.createSprite(640, 120);

  baseBuf = (uint8_t *)malloc(SENS_WIDTH*SENS_HEIGHT * sizeof(uint8_t));
  maskBuf = (boolean *)malloc(SENS_WIDTH*SENS_HEIGHT * sizeof(boolean));

  tftbuff.createSprite(320, 240-SENS_HEIGHT/2);


// draw screen
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  tft.setFreeFont(&neuropol10pt7b);

  tft.setCursor (20, 30);
  tft.print("Check qPocketPCR");

  tft.setCursor (20, 60);
  tft.setFreeFont(&GaudiSans7pt7b);

status_line("Software",false,"Version "+(String)VERSION_STRING);


esp_reset_reason_t reason = esp_reset_reason();

if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_WDT) {
    status_line("Watchdog reset!",true);
    errorCode=ERROR_WATCHDOG;
    emergencyShutdown();
} else {
    status_line("Normal Startup",false);
}


  status_line("Display Initialized",false);
//  status_line("Hardware Pins Initialized",false);


// Mount SPIFFS drive

  status_line("USB Drive Started",!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED));

// Start USB Driver

  Start_USB_Drive();
  addDataToUSB();
  
  // Init LED driver
  leds.setRegister(TLC59108::REGISTER::ALLCALLADR::ADDR, 0x4d);

  leds.init(TLC59108_HWRESET);
  leds.setLedOutputMode(TLC59108::LED_MODE::PWM_IND);

   SetLEDBrightness(false);
 // status_line("LEDs Initialized",false);


  // Init touscreench

   status_line("Touchscreen started",!ts.begin(40));

   status_line("Sensor test",!tla.begin()); //ADC TLA202x found

  tla.setDataRate(TLA202x_RATE_3300_SPS);
  tla.setRange(TLA202x_RANGE_4_096_V);
  tla.setChannel((tla202x_channel_t)ADC_VBus);
  tla.setMode(TLA202x_MODE_CONTINUOUS);
  delay(10);
  

  status_line("USB-C",(tla.readVoltage()*11<4.5),"Voltage: "+String(tla.readVoltage()*11,2));


 // Start Cam
  err=startCam();
  //status_line("Camera init",err != ESP_OK,"error_code: "+(String)err);

  cam_sens = esp_camera_sensor_get();
  status_line("Camera Sensor Ready",false,"ID: "+(String)cam_sens->id.PID);

// Stop Cam
   err=stopCam();


  // Set preferences

  preferences.begin("qPrefs", false);

  if (preferences.getInt("Initialized", 0) != 1)
  {

  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
   wellFactor[sensor] = 1;
   preferences.putFloat("SenFactor"+ sensor, 1.0);
  }

    preferences.putInt("Initialized", 1);
  } else 
  {

  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    sensor_center_x[sensor] = preferences.getInt("SenCentX" + sensor, sensor * SENS_SPACING + SENS_OFFSET_X);
    sensor_center_y[sensor] = preferences.getInt("SenCentY" + sensor, SENS_OFFSET_Y);
    wellFactor[sensor] = preferences.getFloat("SenFactor" + sensor, 1);
    Serial.println(wellFactor[sensor] );
   }
  }

 // status_line("Preferences Initiated",false);


  loadProtocol();
  status_line("Protocol loaded",false);

 if (loadBinFromSPIFFS(baseBuf,SENS_WIDTH*SENS_HEIGHT ,"/base.bin"))
 {
  status_line("Baseline loaded",true);
    for (int i = 0; i < SENS_WIDTH*SENS_HEIGHT; i++) {baseBuf[i]=0;}
 } 
 else   status_line("Baseline loaded",false);

initMask();
if (loadBinFromSPIFFS((uint8_t *)maskBuf,SENS_WIDTH*SENS_HEIGHT ,"/mask.bin"))
 {
  status_line("Mask loaded",true);
    initMask();
 } 
 else   status_line("Mask loaded",false);   

 tft.println(" ");
 tft.println("       Tap to hold...");


  myTime_measure = 0;
  myTime_WD = 0;

  x_pos = 0;

delay(2000);
  while (ts.touched()) 
  {esp_task_wdt_reset();} // Feed the watchdog 
  

  drawMainDisplay();

} // setup

// ==================== MAIN LOOP ====================

void loop() {
  
Service_USB();
if (newConfigAvailable) {loadProtocol();newConfigAvailable=false;if (caseUX==CASE_RunPotocolDisplay) drawProtocolDisplay();}

if (millis() - myTime_WD > TEMP_CHECK_INTERVAL_MS) {readTemperatures();}


switch (caseUX) {
  
 case CASE_Main:


      if (ts.touched()) {
        p = ts.getPoint();

        if (PointInRect(p, 0, 0, 218, 128)) // RUN
        {

          tla.setChannel((tla202x_channel_t)ADC_VBus);
          captures=countCaptures();

          if (captures>MAX_MEASUREMENTS) {}
          
          drawInitRunDisplay(tla.readVoltage()*11);
          caseUX = CASE_InitRun;
        }
        
        if (PointInRect(p, 0, 138, 218, 100)) // PROTOCOL
        {
          setPrototcolDisplayFrame(1);
          drawProtocolDisplay();
          caseUX = CASE_RunPotocolDisplay;         
        }
        
        if (PointInRect(p, 228, 50, 100, 78)) // SUB MENU
        {

          drawSubMenuDisplay();
          caseUX = CASE_RunSubMenuDisplay;
          
        }
        
        if (PointInRect(p, 228, 138, 100, 100))  // SETTINGS
        {
          drawSettingsDisplay();
          caseUX = CASE_RunSettingsDisplay;
        }

      

      }
break; // case Main


case CASE_RunSubMenuDisplay:
runSubMenuDisplay();
break; //RunSettingsDisplay


case CASE_RunSettingsDisplay:
runSettingsDisplay();
break; //RunSettingsDisplay


case CASE_InitMeasurement:


x_pos = 0;
measurements=0;
leds.setAllBrightness(ledBrightness);
//SetLEDBrightness();
err=startCam();  // Start Cam
caseUX = CASE_RunMeasurement;
break; //CASE_InitMeasurement


case CASE_RunPotocolDisplay:
runProtocolDisplay();
break; //CASE_RunPotocolDisplay


case CASE_RunMeasurement:

      //myCamWriteRegister8(0,p.y);

      if (ts.touched()) {
        p = ts.getPoint();
        if (PointInRect(p, 320 - 70, 0, 70, 40)) {
          setHeaters(temperature_mean, 0,0);
          drawMainDisplay();
          caseUX = CASE_Main;
          leds.setAllBrightness(0);
          stopCam(); // Stop Cam
          break;  //CASE_RunMeasurement
        }
      }

    if (millis() - myTime_measure > MEASUREMENT_INTERVAL_MS) {

        MeasureCam();
        camSprite.pushSprite(0, 240 - 60);

        x_pos = x_pos + 1;
        if (x_pos > 280) {
          x_pos = 0;
          drawMeasurementDisplay();
        }

        for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
          if (x_pos > 1) tft.drawLine(20 + x_pos - 1, (240 - 71) - (last_values[sensor] / 2), 20 + x_pos, (240 - 71) - (fluorescence[sensor][0] / 2), my_palette[sensor]);
          last_values[sensor] = fluorescence[sensor][0];
        }

        myTime_measure = millis();
      }
      
break; //CASE_RunMeasurement


case CASE_RunComplete:

if (ts.touched()) {
        caseUX = CASE_Main;
        drawMainDisplay();
        }
break; //CASE_RunComplete

case CASE_InitBaseline:
leds.setAllBrightness(ledBrightness);

err=startCam();  // Start Cam
caseUX = CASE_RunBaseline;

break; //CASE_InitBaseline


case CASE_InitCalibration:
leds.setAllBrightness(ledBrightness);

for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
       sensor_center_x[sensor] = sensor * SENS_SPACING + SENS_OFFSET_X;
       sensor_center_y[sensor] = SENS_OFFSET_Y;
}
          
err=startCam();  // Start Cam
caseUX = CASE_RunCalibration;

break; //CASE_InitCalibration


  
case CASE_RunInitUSB:

if(ts.touched()) {touch=true;p=ts.getPoint();if(p.y>0) tpoint = p;}

  if (!ts.touched()&&touch) {
  touch=false;

  
      if (PointInRect(tpoint, 20, 160, 130, 80)) {  //CANCEL
        drawMainDisplay();
        caseUX = CASE_Main;
        break;
      }

      if (PointInRect(tpoint, 170, 160, 130, 80)) { //FORMAT
         tft.println();
         tft.println("       FORMATTING...");

            esp_task_wdt_delete(NULL); // Stop Watchdog for this
        InitializeUSB();
            esp_task_wdt_add(NULL);// Start Watchdog again
        
        newConfigAvailable=true;
        // Reset Preferences
          for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
            wellFactor[sensor] = 1;
           preferences.putFloat("SenFactor"+ sensor, 1.0);
          }
  
        caseUX = CASE_Main;
        drawMainDisplay();

        break;
      }
  }
      
break; // CASE_RunInitUSB


case CASE_RunBaseline:

if(ts.touched()) {touch=true;p=ts.getPoint();if(p.y>0) tpoint = p;}

  if (!ts.touched()&&touch) {
  touch=false;
  
      if (PointInRect(tpoint, 20, 160, 130, 80)) { //CANCEL
        drawMainDisplay();
        stopCam(); // Stop Cam
        leds.setAllBrightness(0);
        caseUX = CASE_Main;

        break;
      }

      if (PointInRect(tpoint, 170, 160, 130, 80)) { //SAVE BASELINE
         SeeCam(true);
        stopCam(); // Stop Cam
        leds.setAllBrightness(0);

        saveBinToSPIFFS(baseBuf,SENS_WIDTH*SENS_HEIGHT ,"/base.bin");
        
        drawMainDisplay();
        caseUX = CASE_Main;

          
        break;
      }
  }
      SeeCam(false);
      camSprite.pushSprite(0, 80);
      
break; // CASE_RunBaseline

case CASE_RunCalibration:

if(ts.touched()) {touch=true;p=ts.getPoint();if(p.y>0) tpoint = p;}

  if (!ts.touched()&&touch) {
  touch=false;

      if (PointInRect(tpoint,20, 160, 130, 80)) { //CANCEL
        drawMainDisplay();
        stopCam(); // Stop Cam
        leds.setAllBrightness(0);

        for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
        sensor_center_x[sensor] = preferences.getInt("SenCentX" + sensor, sensor * SENS_SPACING + SENS_OFFSET_X);
        sensor_center_y[sensor] = preferences.getInt("SenCentY" + sensor, SENS_OFFSET_Y);
  }

  
        caseUX = CASE_Main;
        break;
      }

      if (PointInRect(tpoint, 170, 160, 130, 80)) { //SAVE ALIGNEMENT

         AlignCam(true);

  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
   wellFactor[sensor] = 1;
  }
         measurements=0;
       if (saveWellFactors)  MeasureCam();

        stopCam(); // Stop Cam
        leds.setAllBrightness(0);
        drawMainDisplay();
        
  float maxFluorescence=0;
  
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
   if (fluorescence[sensor][0]>maxFluorescence) maxFluorescence=fluorescence[sensor][0];
  }


     for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
      if ((fluorescence[sensor][measurements]!=0)&&(saveWellFactors))
      wellFactor[sensor] = maxFluorescence/fluorescence[sensor][measurements];
   
      preferences.putInt("SenCentX" + sensor, sensor_center_x[sensor] );
      preferences.putInt("SenCentY" + sensor, sensor_center_y[sensor] );
      preferences.putFloat("SenFactor"+ sensor, wellFactor[sensor]);

       }

        saveBinToSPIFFS((uint8_t *)maskBuf,SENS_WIDTH*SENS_HEIGHT * sizeof(boolean) ,"/mask.bin");

        caseUX = CASE_Main;
        break;
      } //SAVE ALIGNEMENT

     if (PointInRect(tpoint, 30, 65, 160, 155)) { //Toggle Well Factor
     saveWellFactors=!saveWellFactors;
  }
}
      AlignCam(false);
      camSprite.pushSprite(0, 65);
      tft.setCursor (65,156);
      tft.setFreeFont(&neuropol10pt7b); 
      if (saveWellFactors) tft.print("\x82"); else {tft.print("\x81");tft.fillRect(68,144,12,12,TFT_WHITE);}

break; // CASE_RunCalibration

case CASE_InitRun:

    p = ts.getPoint();
      if (PointInRect(p, 20, 160, 130, 80)) {
        drawMainDisplay();
        caseUX = CASE_Main;
        break;
      }

      if (PointInRect(p, 170, 160, 130, 80)) {
         
        caseUX = CASE_InitQPCR; 
        break;
      }
      
 
break; // CASE_InitRun

case CASE_InitQPCR:

measurements=0;
casePCR = PCR_HEATLID;
PCRstep = 0;
PCRcycle = 1;

TEMPLIDset=LidTemp;
TEMPset = pcrProtocol.steps[PCRstep].temperature;
TIMEcontrol = pcrProtocol.steps[PCRstep].duration*1000;

 tft.fillRect(0,0,320,240,TFT_WHITE);
 drawGrid();

  leds.setAllBrightness(ledBrightness);
  err=startCam();  // Start Cam
  delay(ILLUMINATION_TIME);
  MeasureCam();
  camSprite.pushSprite(0, 240 - 60);
  stopCam();
  leds.setAllBrightness(0);

openNewProtocolFile();
myTime_measure = millis();
      
caseUX = CASE_RunQPCR;
           
break; //CASE_InitQPCR



case CASE_AbortRunQPCR:

      if (ts.touched()) {
        p = ts.getPoint();
        if (PointInRect(p, 15,100,80,50)) {
          setHeaters(temperature_mean, 0,0);
          stopCam(); // Stop Cam
          addDataToUSB();
          drawMainDisplay();
          caseUX = CASE_Main;
          break;
        }

         if (PointInRect(p, 117, 100, 110, 50)) {
          caseUX = CASE_RunQPCR;

         }
      }
   
case CASE_RunQPCR:

      if (ts.touched()) {
        p = ts.getPoint();
        if (PointInRect(p, 320 - 60, 0, 60, 50)) {
          if (casePCR == PCR_END) 
          { drawMainDisplay();
          caseUX = CASE_Main;} else
           caseUX = CASE_AbortRunQPCR;
        }
      }

    readTemperatures();
    
      temperature_mean = (temperature_mean * 3 + temperature) / 4;
      temperature_lid_mean = (temperature_lid_mean * 3 + temperature_lid) / 4;



      PIDIntegration = false;

      TEMPdif = TEMPset - temperature_mean;
      if(abs(TEMPdif<3)) {TEMPi = TEMPi + (TEMPset - temperature_mean);} else TEMPi=0;
      if (TEMPi > TEMPiMax) TEMPi = TEMPiMax;
      if (TEMPi < -TEMPiMax) TEMPi = -TEMPiMax;

      TEMPLIDdif = TEMPLIDset - temperature_lid_mean;
      if(abs(TEMPLIDdif<PIDLIDiRange)) {TEMPLIDi = TEMPLIDi + (TEMPLIDset - temperature_lid_mean);} else TEMPLIDi=0;
      if (TEMPLIDi > TEMPLIDiMax) TEMPLIDi = TEMPLIDiMax;
      if (TEMPLIDi < -TEMPLIDiMax) TEMPLIDi = -TEMPLIDiMax;
      //Serial.print("Temp: ");
      //Serial.println(temperature_mean);
      //Serial.print("Dif: ");
      //Serial.println(TEMPdif);



// Draw Grid (42ms)
 drawGrid();


      if (millis() - TEMPclick > 200) {
        TEMPclick = millis();
        TEMPd = TEMPdif_a[4] - TEMPdif;
        TEMPdif_a[4] = TEMPdif_a[3];
        TEMPdif_a[3] = TEMPdif_a[2];
        TEMPdif_a[2] = TEMPdif_a[1];
        TEMPdif_a[1] = TEMPdif_a[0];
        TEMPdif_a[0] = TEMPdif;
        //  Serial.println (TEMPd);


       // tft.drawLine(x_pos, 230 - ((int)(temperature_mean * 10)) % 180, x_pos, 230 - ((int)(temperature_mean * 10)) % 180, my_palette[4]);

       // x_pos++;
      //  if (x_pos > 300)x_pos = 0;
      }



      switch (casePCR) {


        case PCR_HEATLID:

        runPID();

        if (TEMPLIDdif<1) casePCR = PCR_SET;

         break;

                  
        case PCR_SET:
          TEMPset = pcrProtocol.steps[PCRstep].temperature;
          TIMEcontrol = pcrProtocol.steps[PCRstep].duration*1000;
          PIDIntegration = false;
          casePCR = PCR_TRANSITION;
          break;




        
        case PCR_TRANSITION:
          runPID();
          //draw_run_display();
          if (abs(TEMPset - temperature_mean) < TEMP_TOLLERANCE) {
            PIDIntegration = true;
            TIMEclick = millis();
            casePCR = PCR_TIME;
          }
          break;


        case PCR_TIME:
          runPID();
          TIMEcontrol = pcrProtocol.steps[PCRstep].duration*1000 - (millis() - TIMEclick) ;
          // draw_run_display();
          //Serial.println(PCRstep);
          // Serial.println(pcrProtocol.repeatEnd);

          if (pcrProtocol.steps[PCRstep].capture&&(TIMEcontrol <= ILLUMINATION_TIME)&&!saveReady) {initSaveMeasurement(); saveReady=true;};

          if (TIMEcontrol <= 0) {
              
            if (pcrProtocol.steps[PCRstep].capture)   {measurements++;SaveMeasurement(8);saveReady=false;}
            
            if (PCRstep == pcrProtocol.repeatEnd-1)
            {PCRcycle++;
             PCRstep = pcrProtocol.repeatStart-1;
             if (PCRcycle>pcrProtocol.cycleCount) 
             {PCRstep=pcrProtocol.repeatEnd;PCRcycle=1;}
            }
             else 
             {PCRstep++;}
            
            casePCR = PCR_SET;
            
            if (PCRstep >= pcrProtocol.stepCount)  
            { setHeaters(temperature_mean, 0,0);
              stopCam(); // Stop Camcase
              addDataToUSB();
              casePCR = PCR_END;
              }
         
            
          }
          break;

        case PCR_END:

         break;

         
        default:
          // Statement(s)
          break;


      }//PCR switch
      break; // CASE_RunQPCR




 
case CASE_EditSettings:

     

break; // case CASE_EditSettings

  

}  // SWITCH


}


void initSaveMeasurement()
{
      Serial.println("initSaveMeasurement");

  leds.setAllBrightness(ledBrightness);
  err=startCam();  // Start Cam
  
}

void SaveMeasurement(int step_size)
{
     Serial.println("SaveMeasurement");
  setHeaters(0,0, 0);

  //  analogWrite(EX_LEDS, 20);  // turn the LED on (HIGH is the voltage level)  <<<<<<<<<<<<<<<<<<<<
  //vTaskDelay(1500 / portTICK_PERIOD_MS); // The LED needs to be turned on ~150ms before the call to esp_camera_fb_get()
  MeasureCam();
  stopCam();
  camSprite.pushSprite(0, 240 - 60);

  // analogWrite(EX_LEDS, 0);  // turn the LED on (HIGH is the voltage level) <<<<<<<<<<<<<<<<<<<<
  leds.setAllBrightness(0);

  // pinMode(EX_LEDS, OUTPUT); // Excitation LEDs
  // digitalWrite(EX_LEDS,LOW);   // turn the LED on (HIGH is the voltage level)

  Serial.print(myFileName);
  File file = SPIFFS.open(myFileName, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    /*  bool formatted = SPIFFS.format();
      if(formatted){
      Serial.println("\n\nSuccess formatting");
      }else{
      Serial.println("\n\nError formatting");
      }
    */


  }

  file.print(PCRcycle);
  file.print(", ");
  file.print(int((millis() - myTime_measure)/1000),0);

  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    file.print(", ");
    file.print(fluorescence[sensor][measurements]);
  }
  file.println();
  file.close();
  Serial.println(" ok");
} // SaveMeasurment



void MeasureCam()
{
  const int sensDispSizeX = 50;
  const int sensDispSizeY = 50;
  const int sens_size_x = 20;
  const int sens_size_y = 20;

  const float disp_SENS_SPACING = 36;

  long intensity_sum;
  int intensityCount;
  int intensity;
 

  camera_fb_t * image_fb = NULL;
  image_fb = esp_camera_fb_get();
  
  if (!image_fb)
  {
    ESP_LOGE("my", "Camera capture failed");
  } else
  {

    camSprite.fillRect(0, 0, camSprite.width(), camSprite.height(), TFT_BLACK);
    
    long i = 0;
    long ibuf = 0;
    

 //  // Measure Sensor Intensities + Draw Sensor Images
    for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
      intensity_sum = 0;
      intensityCount=0;
      for (int y = 0; y < sensDispSizeY; y++) {
        for (int x = 0; x < sensDispSizeX; x++) {
          
          i = sensor_center_x[sensor] - sensDispSizeX / 2 + x + (y + sensor_center_y[sensor] - sensDispSizeY / 2) * (image_fb->width);
          ibuf = i -(SENS_OFFSET_Y - camSprite.height()) *image_fb->width;
          
         if (baseBuf[ibuf]<255) intensity = static_cast<int>((image_fb->buf[i] - baseBuf[ibuf]) * (255/(255.0 - baseBuf[ibuf]))); else intensity=255;

                     if (maskBuf[ibuf]) {intensity_sum = intensity_sum + intensity; intensityCount++;}


          if (intensity<0) intensity=0;
            camSpriteBuf[(int)(sensor * disp_SENS_SPACING) + x / 2 + 21 + (y / 2 + 20)*camSprite.width()] = ((intensity & 0x1c) << 11) | ((intensity & 0xe0) >> 5) | ((intensity & 0x07) << 8);
         // camSpriteBuf[(int)(sensor * disp_SENS_SPACING) + x / 2 + 21 + (y / 2 + 20)*camSprite.width()] = maskBuf[ibuf]*0xff;

        } //for x
      } //for y
            fluorescence[sensor][measurements] = (float)intensity_sum*wellFactor[sensor]/ intensityCount;

    }//for Sensors




    /*
      for (int y = 0; y < camSprite.height(); y++) {
        for (int x = 0; x < (camSprite.width()); x++) {
          //image_fb->width 320,120
          uint8_t intensity = (image_fb->buf[x * 2 + (y * 2 + SENS_OFFSET_Y - 40) * image_fb->width]);
          camSpriteBuf[x + y * camSprite.width()] = ((intensity & 0x1c) << 11) | ((intensity & 0xe0) >> 5) | ((intensity & 0x07) << 8);

        }
      }
    */
    camSprite.setFreeFont(&GaudiSans7pt7b); 
    camSprite.setTextColor(TFT_GREEN, TFT_BLACK);

    for (int sensor = 0; sensor < 8; sensor++) {

      camSprite.setTextDatum(MC_DATUM);
      camSprite.setTextColor(my_palette[sensor], TFT_BLACK);
      camSprite.drawString(String(fluorescence[sensor][measurements],0), 160 +17+ (sensor - 4)*disp_SENS_SPACING,51);
      camSprite.drawString(String(sensor+1),160 +17+ (sensor - 4)*disp_SENS_SPACING,11);
      camSprite.fillRect(160 + (sensor - 4)*disp_SENS_SPACING,0,disp_SENS_SPACING,4,  my_palette[sensor]);

      /*
        Serial.print("intensity ");
        Serial.print(sensor);
        Serial.print(" : ");
        Serial.println(fluorescence[sensor]);
      */

    } // for draw



    // camSprite.pushImage(-200, -200, 640, 480, (uint16_t *)image_fb->buf);

    esp_camera_fb_return(image_fb);

  }
} // MeasureCam

void SetLEDBrightness(boolean state)   // Not currently used
{
  const uint8_t LEDBrightness[]
  {225,225,225,225,225,225,225,225};
  
   if (state) for (byte i = 0; i < NUM_SENSORS; i++)
  {
    //  leds.setBrightness(i-1,0);
    leds.setBrightness(i, LEDBrightness[i]);
  } else 
    leds.setAllBrightness(0);

  
  }

void SeeCam(boolean saveBase)
{
  camera_fb_t * image_fb = NULL;
  image_fb = esp_camera_fb_get();

  if (!image_fb)
  {
    ESP_LOGE("my", "Camera capture failed");
  } else {

    for (int y = 0; y < camSprite.height(); y++) {
      for (int x = 0; x < (camSprite.width()); x++) {
        //image_fb->width 320,120
        uint8_t intensity = (image_fb->buf[x * 2 + (y * 2 + SENS_OFFSET_Y - camSprite.height()) * image_fb->width]);
        camSpriteBuf[x + y * camSprite.width()] = ((intensity & 0x1c) << 11) | ((intensity & 0xe0) >> 5) | ((intensity & 0x07) << 8);

      }
    }

 if (saveBase){
       for (int y = 0; y < SENS_HEIGHT; y++) {
      for (int x = 0; x < (SENS_WIDTH); x++) {
        
        uint8_t intensity = (image_fb->buf[x  + (y  + SENS_OFFSET_Y - camSprite.height()) *image_fb->width]);
        baseBuf[x + y * SENS_WIDTH] = intensity ;

      }
    }
 } // saveBase
 


    // camSprite.pushImage(-200, -200, 640, 480, (uint16_t *)image_fb->buf);
    esp_camera_fb_return(image_fb);
  }
} // See Cam



void AlignCam(boolean saveMask)
{
  const int sens_size_x = 60;
  const int sens_size_y = 60;

  const float disp_SENS_SPACING = 36;

  long intensity;
  long center_x;
  long center_y;

  camera_fb_t * image_fb = NULL;
  image_fb = esp_camera_fb_get();

  if (!image_fb)
  {
    ESP_LOGE("my", "Camera capture failed");
  } else {

    long i = 0;
    long ibuf = 0;


//Find Center
    for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
      intensity = 0;
      center_x = 0;
      center_y = 0;
        
      for (int y = 0; y < sens_size_y; y++) {
        for (int x = 0; x < sens_size_x; x++) {

        i = sensor_center_x[sensor] - sens_size_x / 2 + x + (y + sensor_center_y[sensor] - sens_size_y / 2) * (image_fb->width);
        ibuf= i -(SENS_OFFSET_Y - camSprite.height()) *image_fb->width;


        if ((i<307200)&&(i>0)&&(ibuf>0)&&(ibuf<(SENS_WIDTH*SENS_HEIGHT)))
        {
          intensity = intensity + image_fb->buf[i]-baseBuf[ibuf];
          center_x = center_x + (image_fb->buf[i]-baseBuf[ibuf]) * x;
          center_y = center_y + (image_fb->buf[i]-baseBuf[ibuf]) * y;
         // if (image_fb->buf[i]>baseBuf[ibuf])
         // image_fb->buf[i] = (image_fb->buf[i]-baseBuf[ibuf]) | 0x1f; else image_fb->buf[i] =0;
         }
         
        } //for x
      } //for y
 
      sensor_center_x[sensor] = sensor_center_x[sensor] + center_x / intensity - sens_size_x / 2;
      sensor_center_y[sensor] = sensor_center_y[sensor] + center_y / intensity - sens_size_y / 2;

       if (sensor_center_y[sensor] < SENS_OFFSET_Y-SENS_SPACING/3) sensor_center_y[sensor]= SENS_OFFSET_Y-SENS_SPACING/3;
       if (sensor_center_y[sensor] > SENS_OFFSET_Y+SENS_SPACING/3) sensor_center_y[sensor]= SENS_OFFSET_Y+SENS_SPACING/3;
      
       if (sensor_center_x[sensor] < sensor * SENS_SPACING-SENS_SPACING/3 + SENS_OFFSET_X) sensor_center_x[sensor]=sensor * SENS_SPACING-SENS_SPACING/3 + SENS_OFFSET_X;
       if (sensor_center_x[sensor] > sensor * SENS_SPACING+SENS_SPACING/3 + SENS_OFFSET_X) sensor_center_x[sensor]=sensor * SENS_SPACING+SENS_SPACING/3 + SENS_OFFSET_X;

       
     // fluorescence[sensor] = (float)intensity / sens_size_x / sens_size_y;
    
    }//for Sensors

//Extract Sensor Mask

       for (int y = 0; y < SENS_HEIGHT; y++) {
      for (int x = 0; x < (SENS_WIDTH); x++) {
        
        int intensity = (image_fb->buf[x  + (y  + SENS_OFFSET_Y - camSprite.height()) *image_fb->width]-baseBuf[x + y * SENS_WIDTH]);
        if (intensity<0) intensity=0;
       if (saveMask) maskBuf[x + y * SENS_WIDTH] = intensity>MASK_THRESHOLD ;
      // camSpriteBuf[x/2 + y/2 * camSprite.width()] = ((intensity & 0x1c) << 11) | ((intensity & 0xe0) >> 5) | ((intensity & 0x07) << 8);
       camSpriteBuf[x/2 + y/2 * camSprite.width()] = 0x1ce007 *(intensity>MASK_THRESHOLD);

      }
    }

 // Draw Center Cross  
    for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
      camSprite.drawLine(sensor_center_x[sensor] / 2, (sensor_center_y[sensor] - SENS_OFFSET_Y + camSprite.height()) / 2 - 5, sensor_center_x[sensor] / 2, (sensor_center_y[sensor] - SENS_OFFSET_Y + camSprite.height()) / 2 + 5, TFT_RED);
      camSprite.drawLine((sensor_center_x[sensor]) / 2 - 5, (sensor_center_y[sensor] - SENS_OFFSET_Y + camSprite.height()) / 2, (sensor_center_x[sensor]) / 2 + 5, (sensor_center_y[sensor] - SENS_OFFSET_Y + camSprite.height()) / 2, TFT_RED);
    }

    esp_camera_fb_return(image_fb);

    // camSprite.pushImage(-200, -200, 640, 480, (uint16_t *)image_fb->buf);

  }


} // Calibrate Cam





void myCamWriteRegister8(uint8_t reg, uint8_t val) {
  // use i2c
  Wire.beginTransmission(0x21);
  Wire.write((byte)reg);
  Wire.write((byte)val);
  Wire.endTransmission();
}

//// PID

void readTemperatures()
{
  float ADCVoltage = 0;


    tla.setChannel((tla202x_channel_t)ADC_VADC);
      delay(10);
      ADCVoltage = tla.readVoltage();
      
      tla.setChannel((tla202x_channel_t)ADC_BlockTemp);
      delay(10);
      sensorVoltage = tla.readVoltage();

      sensorResistance = ((sensorVoltage * NTC_R0) / (ADCVoltage - sensorVoltage));
      logR = log(sensorResistance * 1000);
      temperature = 1 / (NTC_A + NTC_B * logR + NTC_C * pow(logR, 3))- 273.15;
     // temperature =  1 / (log(sensorResistance * 1000 / NTC_RN) / NTC_B + 1 / NTC_TN) - 273.15 ;
     

      tla.setChannel((tla202x_channel_t)ADC_LidTemp);
      delay(10);
      sensorVoltage = tla.readVoltage();

      sensorResistance = ((sensorVoltage * NTC_R0) / (ADCVoltage - sensorVoltage));
      temperature_lid =  1 / (log(sensorResistance * 1000 / NTC_RN) / NTC_LID_B + 1 / NTC_TN) - 273.15 ;

if (!isTemperatureSafe(temperature)) {errorCode=ERROR_TEMP_SENSOR;emergencyShutdown();}
if (!isTemperatureSafe(temperature_lid)) {errorCode=ERROR_TEMP_SENSOR;emergencyShutdown();}

myTime_WD=millis();
esp_task_wdt_reset(); // Feed the watchdog  
  }

int power_heating(float temperature, float power)
{
  float power_return;
#define FA_c -0.000016878
#define FA_d -0.00285
#define FA_e 0.004473
#define FA_f 0.06775

  power_return =(power-FA_d*temperature-FA_f)/(FA_c*temperature+FA_e);

  if (power_return > 255) power_return = 255;
  if (power_return < 0) power_return = 0;

  return (int)power_return;

}

int power_cooling(float temperature, float power)
{
  float power_return;

#define FA_g -0.2275
#define FA_h 0.589
#define FA_cTl 25
#define FA_cTh 60


  power_return = pow(2.7, ((power * (FA_cTh - FA_cTl) / (temperature - FA_cTl)) - FA_h) / FA_g);

  if (power_return > 255) power_return = 255;
  if (power_return < 0) power_return = 0;

  return (int)power_return;
}

int power_lid_heating(float power)
{
  float power_return;

#define FA_g 0.0
#define FA_h 0.0156

power_return = (power + FA_g)/FA_h;

  if (power_return > 255) power_return = 255;
  if (power_return < 0) power_return = 0;

  return (int)power_return;

}


void runPID()
{

    TEMPcontrol = PIDp * TEMPdif + PIDd * TEMPd +  PIDi * TEMPi;
    TEMPLIDcontrol = PIDLIDp * TEMPLIDdif+  PIDLIDi * TEMPLIDi;
    setHeaters(temperature_mean, TEMPcontrol,TEMPLIDcontrol); 


} //runPID

void setHeaters(float temperature, float blockPower,float lidPower)
{
  int blockPWM=0;
  int fanPWM=0;
  int lidPWM=0;




lidPWM = power_lid_heating(lidPower);

      //  tft.drawLine(x_pos, 170 - lidPWM/2, x_pos, 170 - lidPWM/2, my_palette[1]);


    if (casePCR != PCR_HEATLID)
    if (blockPower > 0)
    {
      blockPWM = power_heating(temperature, blockPower);
      fanPWM=0;

      
      if (LidCurrentMax/255*lidPWM + BlockCurrentMax/255*blockPWM > CurrentMax) blockPWM= (CurrentMax-LidCurrentMax/255*lidPWM)/BlockCurrentMax*255;
      
      if (blockPWM > 255) blockPWM = 255;
      if (blockPWM < 0) blockPWM = 0;

  
    }
    else
    {
      fanPWM = power_cooling(temperature, blockPower);
      blockPWM=0;

      if (LidCurrentMax/255*lidPWM + BlockCurrentMax/255*fanPWM > CurrentMax) fanPWM= (CurrentMax-LidCurrentMax/255*lidPWM)/FanCurrentMax*255;
      
      if (fanPWM > 255) fanPWM = 255;
      if (fanPWM < 0) fanPWM = 0;
    }


//Serial.print("fanPWM: ");
//Serial.println(fanPWM);

//Serial.print("blockPWM: ");
//Serial.println(blockPWM);

//Serial.print("lidPWM: ");
//Serial.println(lidPWM);
//Serial.println("");


  ledcWrite(FAN_CHANNEL, fanPWM);  // Set the PWM duty cycle for FAN 
  ledcWrite(HEATER_CHANNEL, blockPWM);  // Set the PWM duty cycle for HEATER 
  ledcWrite(LID_CHANNEL, lidPWM);  // Set the PWM duty cycle for HEATER 

}

esp_err_t startCam()
{
  
  // Init camera
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0; //?
  config.ledc_timer = LEDC_TIMER_0;   //?
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; //20000000

  config.pin_sccb_sda = -1;
  config.sccb_i2c_port =0;


  // config.pixel_format = PIXFORMAT_JPEG;
  // config.pixel_format = PIXFORMAT_RGB565;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  //  config.pixel_format = PIXFORMAT_YUV422;


  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;

  config.jpeg_quality = 12;
  config.fb_count = 2;

  config.frame_size = FRAMESIZE_VGA;

  // config.fb_location = CAMERA_FB_IN_DRAM;
  config.fb_location = CAMERA_FB_IN_PSRAM;


  //FRAMESIZE_240X240     FRAMESIZE_SVGA  (800 x 600)      FRAMESIZE_VGA(640 x 480)   FRAMESIZE_QVGA  (320 x 240)

  esp_err_t err = esp_camera_init(&config);

  cam_sens = esp_camera_sensor_get();
 
  res = cam_sens->set_vflip(cam_sens, 1);
  res = cam_sens->set_hmirror(cam_sens, 1);

  res = cam_sens->set_brightness(cam_sens, 1); // up the brightness just a bit
  res = cam_sens->set_whitebal(cam_sens, 0);     // set automatic white balance
  res = cam_sens->set_exposure_ctrl(cam_sens, 0); // set automatic exposure
  res = cam_sens->set_awb_gain(cam_sens, 0);  // set automatic white balance gain
  res = cam_sens->set_aec2(cam_sens, 0);  // aec DSP
  res = cam_sens->set_gain_ctrl(cam_sens, 0); // set automatic gain control

  res = cam_sens->set_reg(cam_sens, 0x00, 0xFF, 16);     //set gain (0-127?) Gain=(GAIN[7]+1)×(GAIN[6]+1)×(GAIN[5]+1)×(GAIN[4]+1)×(GAIN[3:0]/16+1)
  res = cam_sens->set_aec_value(cam_sens, 512);        //set exposure (0-512?)

  res = cam_sens->set_reg(cam_sens, 0x46, 0x07, 1);  // set Lens correction    
  //res = cam_sens->set_reg(cam_sens, 0x47, 0xFF, 15);  // set Lens center 
  res = cam_sens->set_reg(cam_sens, 0x49, 0xFF, 150);    // set Lens correction factor  200
  res = cam_sens->set_reg(cam_sens, 0x4A, 0xFF, 10);    // set Lens correction mid Radius 15

  // res = cam_sens->set_reg(cam_sens, 0x12, 0x3, 3);     //set RAW?
  //cam_sens->set_pixformat(cam_sens, pf);
  // res = cam_sens->set_pixformat(cam_sens, PIXFORMAT_RGB565);

  cameraOn=true;
return err;
}

esp_err_t stopCam()
{
    esp_err_t err = esp_camera_deinit();
    cameraOn=false;
   return err;
  }



  
void openNewProtocolFile()
{

  File file = SPIFFS.open(myFileName, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for appending");
      bool formatted = SPIFFS.format();
      if(formatted){
      Serial.println("\n\nSuccess formatting");
      }else{
      Serial.println("\n\nError formatting");
      }
    
  }
file.print("Protocol name: ");
file.println(pcrProtocol.name);
file.println("Cycle, Time, Sensor1, Sensor2, Sensor3, Sensor4, Sensor5, Sensor6, Sensor7, Sensor8");

}


void addDataToUSB()
{
  InitializeUSBFiles();
  addFileToFAT(SPIFFS,myFileName);

  Serial.println("InitializeUSBFiles");
}

void initMask()
{
  
   for (int i = 0; i < SENS_WIDTH*SENS_HEIGHT; i++) maskBuf[i]=false;  


  const int sens_size_x = 20;
  const int sens_size_y = 20;

 // Measure Sensor Intensities
    long i = 0;

    for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
      for (int y = 0; y < sens_size_y; y++) {
        for (int x = 0; x < sens_size_x; x++) {

          //  i=sensor*SENS_SPACING+x+SENS_OFFSET_X+(y+SENS_OFFSET_Y)*(image_fb->width);
          i = sensor_center_x[sensor] - sens_size_x / 2 + x + (y-SENS_OFFSET_Y+camSprite.height() + sensor_center_y[sensor] - sens_size_y / 2) * (SENS_WIDTH);
           maskBuf[i]=true;
        } //for x
      } //for y
    }//for Sensors


}

int countCaptures()
{int captureCounter=0;

for (int stepCounter = 0; stepCounter < pcrProtocol.stepCount; stepCounter++) {

if (pcrProtocol.steps[stepCounter].capture)
{if ((stepCounter<=pcrProtocol.repeatEnd-1)&&(stepCounter>=pcrProtocol.repeatStart-1))
captureCounter=captureCounter+pcrProtocol.cycleCount; else
captureCounter=captureCounter+1;
  
  }

}
  return captureCounter;
}




void  drawGrid()

{

  #define TFT_BUTTONCOLOR    0x3575


const int minSpacing = 30;       // Minimum spacing in pixels
const int grid_w = 230;
const int grid_h = 128;
const int margin_left = 8;
const int margin_top = 30;

// Clear Buffer
 tftbuff.fillRect(0, 0, 320, 180, TFT_WHITE);

// Draw Back Button
 tftbuff.setTextColor(TFT_WHITE,TFT_BLACK);
 tftbuff.setFreeFont(&neuropol10pt7b); 
 tftbuff.fillRoundRect(320-52,3,45,33,12,TFT_BUTTONCOLOR);
 tftbuff.setCursor (320-40,25);
 tftbuff.print(char(0x80));
 tftbuff.setTextColor(TFT_BLACK,TFT_WHITE);


  int division=1;
  int xSpacing=grid_w/captures;
  if (xSpacing<minSpacing) {division=ceil(minSpacing*captures/grid_w);xSpacing=division*grid_w/captures;}

  int yMax=INT_MIN;
  int yMin=INT_MAX;

for (int x = 0; x <= measurements; x++)
for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
int value = fluorescence[sensor][x];
      
      if (value > yMax) {yMax = value;}
      if (value < yMin) {yMin = value;}
}

  int yStep=10*ceil((yMax-yMin)/70.0);
  if (yMax-yMin<35) yStep=5;
  int yMinLine=yStep*floor((float)yMin/yStep);
  int ySpacing = grid_h/8;
  int yLines = grid_h / ySpacing;



  tftbuff.setTextDatum(MC_DATUM);
  tftbuff.setFreeFont(&GaudiSans7pt7b); 

  // Draw vertical lines and labels (captures)
  for (int i = 0; i <= (captures/division); i++) {
    int x = i * xSpacing;
    tftbuff.drawLine(x+margin_left, margin_top, x+margin_left, margin_top+grid_h, TFT_LIGHTGREY);
     String label= String(i*division);
    tftbuff.drawString(label, x + margin_left, grid_h+margin_top+8);
  }
    tftbuff.drawLine(grid_w+margin_left, margin_top, grid_w+margin_left, margin_top+grid_h, TFT_LIGHTGREY);


  // Draw horizontal lines and labels (ranges)
  for (int j = 0; j <= yLines; j++) {
    int y = j * ySpacing;
    tftbuff.drawLine(margin_left, y+margin_top, margin_left+grid_w, y+margin_top, TFT_LIGHTGREY);
  //  tftbuff.setCursor(0, y + 2);
  //  tftbuff.print("R");
 //   tftbuff.print(j);
  }
    tftbuff.drawLine(margin_left, grid_h+margin_top, margin_left+grid_w, grid_h+margin_top, TFT_LIGHTGREY);

if (((yMinLine+8*yStep)>=0)&&(yMinLine<=0))
    tftbuff.drawLine(margin_left, grid_h+margin_top+yMinLine/yStep*ySpacing, margin_left+grid_w, grid_h+margin_top+yMinLine/yStep*ySpacing, TFT_DARKGREY);


// Draw Measurements

for (int x = 0; x < measurements; x++)
for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {

tftbuff.drawLine(margin_left+x*((float)xSpacing/division),grid_h+margin_top+(yMinLine-(fluorescence[sensor][x]))/yStep*ySpacing,margin_left+(x+1)*((float)xSpacing/division),grid_h+margin_top+(yMinLine-(fluorescence[sensor][x+1]))/yStep*ySpacing,my_palette[sensor]);


  }


  
 // Write Step
      
      tftbuff.setTextColor(TFT_BLACK, TFT_WHITE);
      tftbuff.setFreeFont(&GaudiSans7pt7b);

      tftbuff.setCursor (20, 18);
      if (casePCR == PCR_HEATLID){
        tftbuff.print("Heating Lid");}
      else 
      if (casePCR == PCR_END){
        tftbuff.print("== RUN COMPLETE ==");}
      else
      {
      tftbuff.print("Step ");
      tftbuff.print(PCRstep+1);
      tftbuff.print(": ");
      tftbuff.print(pcrProtocol.steps[PCRstep].name); 
      }


  // Write Status
      tftbuff.setCursor (255, 60);
      tftbuff.print(char(0x83)); // Temp
      tftbuff.print(temperature_mean, 1);
      tftbuff.print("°C"); 

      tftbuff.setCursor (255, 75);
      tftbuff.print(char(0x84)); // Set Temp
      tftbuff.print(TEMPset,1);
      tftbuff.print("°C"); 

      tftbuff.setCursor (255, 90);
      tftbuff.print(char(0x86));
      tftbuff.print(TIMEcontrol/1000,0);
      tftbuff.print("s"); 

      tftbuff.setCursor (255, 105);
      tftbuff.print(char(0x85));
      tftbuff.print(PCRcycle); 
      tftbuff.print(" / "); 
      tftbuff.print(pcrProtocol.cycleCount);

      tftbuff.setCursor (255, 120);
      tftbuff.print("L ");
      tftbuff.print(temperature_lid_mean,0); 
      tftbuff.print("°C"); 




      // Draw Cancle Frame

      if (caseUX == CASE_AbortRunQPCR)
      {
  tftbuff.fillRect(15, 40, 215, 110, TFT_WHITE);
  tftbuff.drawRect(15, 40, 215, 110, TFT_DARKGREY);
  tftbuff.setFreeFont(&neuropol10pt7b); 
  tftbuff.setCursor (20,65);
  tftbuff.println("Abort PCR run?");
  tftbuff.setCursor (35,85);
  tftbuff.setFreeFont(&GaudiSans7pt7b);
  tftbuff.println("Current data will be safed.");

  tftbuff.setTextColor(TFT_WHITE,TFT_DARKGREY);
  tftbuff.fillRoundRect(25,100,80,40,12,TFT_DARKGREY);
  tftbuff.drawString("ABORT",64,119);
  tftbuff.fillRoundRect(117,100,100,40,12,TFT_DARKGREY);
  tftbuff.drawString("RESUME",168,119);
      }
      
      tftbuff.pushSprite(0, 0);

      

}

bool isTemperatureSafe(float temp) {
    return (temp >= SAFETY_MIN_TEMP && 
            temp <= SAFETY_MAX_TEMP);
}


void emergencyShutdown() {

// Immediately shut off all heaters

  pinMode(HEATER_PWM_PIN, OUTPUT);
  digitalWrite(HEATER_PWM_PIN, LOW);

  pinMode(LID_PWM_PIN, OUTPUT);
  digitalWrite(LID_PWM_PIN, LOW);

 // ledcWrite(HEATER_CHANNEL, 0);
 // ledcWrite(LID_CHANNEL, 0);
 
// Full fan power for cooling
  pinMode(FAN_PWM_PIN, OUTPUT);
  digitalWrite(FAN_PWM_PIN, HIGH);

  //  ledcWrite(FAN_CHANNEL, 255); // Full fan power for cooling

    
    // Turn off LEDs
    leds.setAllBrightness(0);


 
    // Display error on screen
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(&neuropol10pt7b);
    tft.setCursor(35, 50);
    tft.println("SYSTEM ERROR !");
    tft.setCursor(35, 80);
    tft.setFreeFont(&GaudiSans7pt7b);
    if (errorCode==ERROR_TEMP_SENSOR) tft.println("TEMPERATURE OUT OF RANGE.");
    if (errorCode==ERROR_WATCHDOG) tft.println("CODE ERROR.");


  while (true) //!ts.touched()
  {esp_task_wdt_reset();} // Feed the watchdog 
  
}
