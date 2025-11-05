#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "Adafruit_MCP23008.h"
#include <SPI.h>
#include <Adafruit_FT6206.h>



#include "Buttons.h"

#define FORMAT_SPIFFS_IF_FAILED true

#define FILESYSTEM SPIFFS
// You only need to format the filesystem once


#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

//
// NOTE: PSRAM IC required for UXGA resolution and high JPEG quality
//       Ensure ESP32 Wrover Module or other board with PSRAM is selected
//       You must select partition scheme from the board menu that has at least 3MB APP space.


#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    32 //25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define EX_LEDS      25 //32
#define HEATER_PWM     4
#define FAN_PWM     12

#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander

  
#define CASE_Main 1
#define CASE_Settings 2
#define CASE_EditSettings 3
#define CASE_Run 4
#define CASE_Done 5

#define PCR_set 1
#define PCR_transition 2
#define PCR_time 3
#define PCR_end 4


#define COLOR_DEPTH 1  // Colour depth (1, 8 or 16 bits per pixel)


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


const int noSen=8;
int x_pos=0;

String fileName="/data";


float fluorescence[noSen];
float last_values[noSen];

unsigned long myTime_display;
unsigned long myTime_measure;

int caseUX = CASE_Main; 
int casePCR = PCR_set;
int MenuItem = 1;
int PCRstate = 0;
int PCRcycle = 1;


// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "GaudiLabs";
const char* password = "versonet";

TFT_eSPI tft = TFT_eSPI();                   // Invoke custom library with default width and height
TFT_eSprite sp = TFT_eSprite(&tft);

TFT_eSprite img = TFT_eSprite(&tft);  // Sprite class
TFT_eSprite img_arrow = TFT_eSprite(&tft);  // Sprite class

const char* host = "esp32fs";
WebServer server(80);
File fsUploadFile;

Adafruit_MCP23008 mcp;
Adafruit_FT6206 ts = Adafruit_FT6206();
TS_Point p;

sensor_t * s;
esp_err_t res = ESP_OK;
uint16_t *spriteBuf ;
  
void startCameraServer();



/// File System
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  File file = FILESYSTEM.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
    File file = FILESYSTEM.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = FILESYSTEM.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  FILESYSTEM.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = FILESYSTEM.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = FILESYSTEM.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.path()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "text/json", output);
}

/// File System



bool PointInRect(TS_Point point, int x, int y, int h, int v)

{
  Serial.print(p.y);
  Serial.print(" ");
    Serial.println(240-p.x);
  return (p.y>x)&(p.y<x+h)&(240-p.x>y)&(240-p.x<y+v);
  }


void setup() {
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Wire.begin(26, 27);

// Set Init Pins Hardware
  mcp.begin();      // use default address 0

  mcp.pinMode(TFT_RST, OUTPUT); 
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


// Init TFT and Sprites
//sp.setAttribute(PSRAM_ENABLE, false);
  tft.init();
  spriteBuf=(uint16_t*)sp.createSprite(320,60);

// Init touch screen
   if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }
  
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  
  
 // config.pixel_format = PIXFORMAT_JPEG; 
 // config.pixel_format = PIXFORMAT_RGB565; 
   config.pixel_format = PIXFORMAT_GRAYSCALE; 
 //  config.pixel_format = PIXFORMAT_YUV422; 
   
    
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  // config.grab_mode = CAMERA_GRAB_LATEST;

    config.jpeg_quality = 12;
    config.fb_count = 1;

     config.frame_size = FRAMESIZE_VGA;
     
  // config.fb_location = CAMERA_FB_IN_DRAM;
     config.fb_location = CAMERA_FB_IN_PSRAM;



 //FRAMESIZE_240X240     FRAMESIZE_SVGA  (800 x 600)      FRAMESIZE_VGA(640 x 480)   FRAMESIZE_QVGA  (320 x 240)


  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

   s = esp_camera_sensor_get();
   Serial.printf("Camera Sensor ID: 0x%x \r\n", s->id.PID);
  Serial.println("Camera Ready!");

  res = s->set_vflip(s, 1); 
  res = s->set_hmirror(s, 1);

  res = s->set_brightness(s, 1); // up the brightness just a bit
  res = s->set_whitebal(s, 0);     // set automatic white balance
  res = s->set_exposure_ctrl(s, 0); // set automatic exposure
  res = s->set_awb_gain(s, 0);  // set automatic white balance gain
  res = s->set_aec2(s, 0);  // aec DSP
  res = s->set_gain_ctrl(s, 0); // set automatic gain control
  
  res = s->set_reg(s, 0x00, 0xFF, 120);     //set gain (0-127?)
  res = s->set_aec_value(s, 512);        //set exposure (0-512?)

 // res = s->set_reg(s, 0x12, 0x3, 3);     //set RAW?
//s->set_pixformat(s, pf);
// res = s->set_pixformat(s, PIXFORMAT_RGB565);

 


// connect WIFI
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected to IP: ");
  Serial.println(WiFi.localIP());


  Serial.print("For file system open http://");
  Serial.print(host);
  Serial.println(".local/edit");


  fileName=fileName+random(1,10000);;
  fileName=fileName+".txt";
  
    if(!FILESYSTEM.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

     MDNS.begin(host);

    
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from FILESYSTEM
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(0));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();

  
// draw screen    
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextFont(0);        // Select font 0 which is the Adafruit font



    delay(1000);
  

myTime_display=0;
myTime_measure=0;
x_pos=319;

 draw_main_display();
 
} // setup

void loop() {
  //  Everything is done in another task by the web server
  
  server.handleClient();


switch (caseUX) {
  case CASE_Main:
         if (ts.touched()) {  
          p = ts.getPoint();
          if (PointInRect(p,320-40,5,32,32)) {draw_settup_display();caseUX=CASE_EditSettings;}
          if (PointInRect(p,35,70,160,40)) {tft.fillRect(0,0,320,240,TFT_WHITE);caseUX=CASE_Settings;}
          if (PointInRect(p,205,70,40,40)) {tft.fillRect(0,0,320,240,TFT_WHITE);caseUX=CASE_Run;}



   }
   
      break;

      case CASE_Settings:

  //myCamWriteRegister8(0,p.y);
 
      

if (millis()-myTime_measure>200){
MeasureCam();
sp.pushSprite(0,240-60);
  
x_pos=x_pos+1;
if (x_pos>280) {x_pos =0; tft.fillRect(0,0,320,(240-60),TFT_WHITE);

     for (int x = 0; x < 21; x++) {
      tft.drawLine(x*(tft.width()-40)/20+20, (tft.height()-70), x*(tft.width()-40)/20+20,tft.height()-60-140,TFT_LIGHTGREY);
      }
      for (int y = 0; y < 14; y++) {
      tft.drawLine(20,tft.height()-70-10*y,tft.width()-20,tft.height()-70-10*y,TFT_LIGHTGREY);
}
}

 
for (int sensor = 0; sensor < noSen; sensor++){

if (x_pos>1) tft.drawLine(20+x_pos-1, (240-71)-(last_values[sensor]),20+x_pos, (240-71)-(fluorescence[sensor]),my_palette[sensor]);
last_values[sensor]=fluorescence[sensor];

}

myTime_measure=millis();

}

          break; //CASE_Settings

      case CASE_EditSettings:
    
          break;

      case CASE_Run:
    

if (millis()-myTime_measure>1000){
MeasureCam();
sp.pushSprite(0,240-60);
  
x_pos=x_pos+1;
if (x_pos>280) {x_pos =0; tft.fillRect(0,0,320,(240-60),TFT_WHITE);

     for (int x = 0; x < 21; x++) {
      tft.drawLine(x*(tft.width()-40)/20+20, (tft.height()-70), x*(tft.width()-40)/20+20,tft.height()-60-140,TFT_LIGHTGREY);
      }
      for (int y = 0; y < 14; y++) {
      tft.drawLine(20,tft.height()-70-10*y,tft.width()-20,tft.height()-70-10*y,TFT_LIGHTGREY);
}
}

 File file = FILESYSTEM.open(fileName, FILE_APPEND);
   if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    
for (int sensor = 0; sensor < noSen; sensor++){


if (x_pos>1) tft.drawLine(20+x_pos-1, (240-71)-(last_values[sensor]),20+x_pos, (240-71)-(fluorescence[sensor]),my_palette[sensor]);
last_values[sensor]=fluorescence[sensor];
file.print(fluorescence[sensor]);
file.print(", ");  

}

file.println();
    file.close();
myTime_measure=millis();

}
          break; // CASE_Run
          
}
  




 


    //  tft.fillRect(260, 0,80, 240, TFT_WHITE);
   //  p = ts.getPoint();
  //      tft.setCursor (260, 25);
 //   tft.print(240-p.x);
  //    tft.fillRect(260, 240-p.x-25,50, 50, TFT_BLACK);
   // };


}

void MeasureCam()
{

const int sens_offset_x=46;
const int sens_offset_y=234;
const int sens_size_x=50;
const int sens_size_y=50;
const float sens_spacing=73;
const float disp_sens_spacing = 36;

long intensity;
long center_x[noSen];
long center_y[noSen];

camera_fb_t * image_fb = NULL;
image_fb = esp_camera_fb_get();

if (!image_fb)
      { ESP_LOGE("my", "Camera capture failed"); }

  long i=0;
       for (int sensor = 0; sensor < noSen; sensor++){
        intensity=0;
        center_x[sensor]=0; 
        center_y[sensor]=0;
        for (int y = 0; y < sens_size_y; y++){
          for (int x = 0; x < sens_size_x; x++) {

    i=sensor*sens_spacing+x+sens_offset_x+(y+sens_offset_y)*(image_fb->width);

intensity=intensity+image_fb->buf[i];
center_x[sensor]=center_x[sensor]+intensity*x;
center_y[sensor]=center_y[sensor]+intensity*y;

image_fb->buf[i]=image_fb->buf[i]|0x1f;
     
     } //for x
   // Serial.println();
  } //for y

   center_x[sensor]=center_x[sensor]/intensity;
   center_y[sensor]=center_y[sensor]/intensity;
   fluorescence[sensor]=(float)intensity/sens_size_x/sens_size_y;
       }//for Sensors

for (int y = 0; y < sp.height(); y++){
     for (int x = 0; x < (sp.width()); x++) {
//image_fb->width 320,120
uint8_t intensity=(image_fb->buf[x*2+(y*2+sens_offset_y-40)*image_fb->width]);
spriteBuf[x+y*sp.width()]=((intensity&0x1c)<<11)|((intensity&0xe0)>>5)|((intensity&0x07)<<8);

     }
}

 sp.setTextFont(0); 
 sp.setTextSize(1);
 sp.setTextColor(TFT_GREEN,TFT_BLACK);
 
 for (int sensor = 0; sensor < 8; sensor++){
    
 sp.setCursor (160+14+(sensor-4)*disp_sens_spacing,7);
 sp.print(fluorescence[sensor],0);

   
/*
Serial.print("intensity ");
Serial.print(sensor);
Serial.print(" : ");
Serial.println(fluorescence[sensor]);
*/
    

  } // for draw

// sp.pushImage(-200, -200, 640, 480, (uint16_t *)image_fb->buf);

esp_camera_fb_return(image_fb);

  
} // MeasureCam


void myCamWriteRegister8(uint8_t reg, uint8_t val) {
  // use i2c
  Wire.beginTransmission(0x21);
  Wire.write((byte)reg);
  Wire.write((byte)val);
  Wire.endTransmission();
}





/// DISPLAY


void draw_main_display()
{

  
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


   Serial.println("Welcome");
}

void draw_settup_display()
{
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
}
