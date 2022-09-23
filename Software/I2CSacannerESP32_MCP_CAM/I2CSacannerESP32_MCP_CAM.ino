/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include "Adafruit_MCP23008.h"
#include "esp_camera.h"

#include <Wire.h>
 
#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander


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
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
void setup() {
Wire.begin(26, 27);
Serial.begin(115200);
  Serial.println("\nI2C Scanner");

  Adafruit_MCP23008 mcp;
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
  mcp.digitalWrite(LED1, HIGH); // System reset input, active low
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

  esp_err_t err = esp_camera_init(&config);

}
 
void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);          
}
