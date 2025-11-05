#include "esp_camera.h"
#include <WiFi.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h" // Include the header file attached to this sketch

#include <SPI.h>
#include <Adafruit_FT6206.h>

TFT_eSPI tft = TFT_eSPI();                   // Invoke custom library with default width and height

TFT_eSprite sp = TFT_eSprite(&tft);


Adafruit_FT6206 ts = Adafruit_FT6206();

 TS_Point p;


//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15 
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "GaudiLabs";
const char* password = "versonet";

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  tft.init();

  //sp.createSprite(128,128);

   if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }
  
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
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
  
  
 // config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  // config.grab_mode = CAMERA_GRAB_LATEST;

    config.jpeg_quality = 12;
    config.fb_count = 2;

     config.frame_size = FRAMESIZE_VGA;
     
  // config.fb_location = CAMERA_FB_IN_DRAM;
     config.fb_location = CAMERA_FB_IN_PSRAM;


 //FRAMESIZE_240X240     FRAMESIZE_SVGA       FRAMESIZE_VGA   FRAMESIZE_QVGA 



#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  
    Serial.printf("Camera Sensor ID: 0x%x \r\n", s->id.PID);


  // initial sensors are flipped vertically and colors are a bit saturated
 /* if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }*/
  
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
    tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
    tft.setTextFont(0);        // Select font 0 which is the Adafruit font

      tft.setTextColor(TFT_BLACK, TFT_BLUE);
 tft.fillRect(0,0,20,240,0);
 
      tft.setTextSize(1);
      
          tft.setCursor (60, 25);
 tft.setFreeFont(FF18); 
    tft.print("qPocketPCR");

          tft.setCursor (60, 45);
 tft.setFreeFont(FSS9); 
    tft.print("qPocketPCR");
    
    tft.setCursor (60, 65);
     tft.setFreeFont(FSS12); 
    tft.print("SETUP");


        tft.setCursor (60, 95);
     tft.setFreeFont(FSSB12); 
    tft.print("SETUP");

//sp.fillRect(0,0,20,240,0);
    //  sp.pushSprite(0, 0);

int button_width = 120;
int button_height = 40;




  tft.fillRoundRect(160-button_width/2, 120-button_height/2,button_width, button_height,10, TFT_BLACK);
  tft.fillRoundRect(160-button_width/2+3, 120-button_height/2+3, button_width-6, button_height-6,10, TFT_WHITE);
            tft.setCursor (160-33, 120+8);
            tft.print("STOP");
                        tft.println();

}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(100);
   if (ts.touched()) {  

      tft.fillRect(260, 0,80, 240, TFT_WHITE);

     p = ts.getPoint();
        tft.setCursor (260, 25);
    tft.print(240-p.x);
      tft.fillRect(260, 240-p.x-25,50, 50, TFT_BLACK);
    };

}
