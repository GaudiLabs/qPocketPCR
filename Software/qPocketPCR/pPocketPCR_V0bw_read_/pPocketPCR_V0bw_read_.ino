#include "esp_camera.h"
#include <WiFi.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "Adafruit_MCP23008.h"
#include <SPI.h>
#include <Adafruit_FT6206.h>


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
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define EX_LEDS      32
#define HEATER_PWM     4

#define LED1      6 // on MCP23008 Port Expander
#define CAM_PWR   5 // on MCP23008 Port Expander
#define CAM_PWDN   4 // on MCP23008 Port Expander
#define TFT_RST   3 // on MCP23008 Port Expander
#define TOUCH_RST   2 // on MCP23008 Port Expander
#define CAM_RST   1 // on MCP23008 Port Expander
#define TFT_BACKLIGHT   0 // on MCP23008 Port Expander


int x_pos=0;

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "GaudiLabs";
const char* password = "versonet";

TFT_eSPI tft = TFT_eSPI();                   // Invoke custom library with default width and height
TFT_eSprite sp = TFT_eSprite(&tft);

Adafruit_MCP23008 mcp;
Adafruit_FT6206 ts = Adafruit_FT6206();
TS_Point p;

sensor_t * s;
esp_err_t res = ESP_OK;
uint16_t *spriteBuf ;
  
void startCameraServer();

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

  pinMode(EX_LEDS, OUTPUT); // Excitation LEDs
  analogWrite(EX_LEDS,50);   // turn the LED on (HIGH is the voltage level)

  pinMode(HEATER_PWM, OUTPUT); // Excitation LEDs
  digitalWrite(HEATER_PWM, LOW);


// Init TFT and Sprites
//sp.setAttribute(PSRAM_ENABLE, false);
  tft.init();
  spriteBuf=(uint16_t*)sp.createSprite(320,50);

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

 
unsigned long myTime;

myTime=millis();

 

Serial.print("wb ");
Serial.println(millis()-myTime);


// connect WIFI
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
    
// draw screen    
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextFont(0);        // Select font 0 which is the Adafruit font

 /* tft.setTextColor(TFT_BLACK, TFT_BLUE);
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
*/
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(500);
  // if (ts.touched()) {  

unsigned long myTime;

    myTime=millis();
    camera_fb_t * image_fb = NULL;
    image_fb = esp_camera_fb_get();


    if (!image_fb)
        {
            ESP_LOGE("my", "Camera capture failed");
        }
     
  //  (uint16_t *)fb->buf
  //esp_camera_fb_return(image_fb);

const int sens_offset_x=46;
const int sens_offset_y=234;
const int sens_size_x=50;
const int sens_size_y=50;
const float sens_spacing=73;

uint8_t red;
uint8_t green;
uint8_t blue;
long intensity[8];
long center_x[8];
long center_y[8];

  long i=0;
       for (int sensor = 0; sensor < 8; sensor++){
        intensity[sensor]=0;
        center_x[sensor]=0; 
        center_y[sensor]=0;
      for (int y = 0; y < sens_size_y; y++){
     for (int x = 0; x < sens_size_x; x++) {


 i=sensor*sens_spacing+x+sens_offset_x+(y+sens_offset_y)*(image_fb->width);

intensity[sensor]=intensity[sensor]+image_fb->buf[i];
center_x[sensor]=center_x[sensor]+intensity[sensor]*x;
center_y[sensor]=center_y[sensor]+intensity[sensor]*y;

image_fb->buf[i]=image_fb->buf[i]|0x1f;
     
      //  Serial.write (',');
     //   Serial.print (fb->buf[i]);
    //     i++;
    //     if ((x>100)&&(x<120)&&(y>50)&&(y<170)) image_fb->buf[i*2]=0xF8;
   //       if ((x>200)&&(x<220)&&(y>50)&&(y<170)) image_fb->buf[i*2+1]=0x1F;

        //   if ((x>200)&&(x<220)&&(y>50)&&(y<170)) image_fb->buf[i]=image_fb->buf[i]/2;
    }
   // Serial.println();
}

   center_x[sensor]=center_x[sensor]/intensity[sensor];
   center_y[sensor]=center_y[sensor]/intensity[sensor];

       }

for (int y = 0; y < sp.height(); y++){
     for (int x = 0; x < (sp.width()); x++) {
//image_fb->width 320,120
uint8_t intensity=(image_fb->buf[x*2+(y*2+sens_offset_y-20)*image_fb->width]);
spriteBuf[x+y*sp.width()]=((intensity&0x1c)<<11)|((intensity&0xe0)>>5)|((intensity&0x07)<<8);

     }
}
     
   // sp.pushImage(-200, -200, 640, 480, (uint16_t *)image_fb->buf);
  // sp.pushImage(0, -80, 320, 240, (uint16_t *)image_fb->buf);
// Serial.println( sp.frameBuffer(1));




   sp.pushSprite(0,240-50);


x_pos=x_pos+1;
if (x_pos>320) {x_pos =0; tft.fillRect(0,0,320,(240-55),TFT_WHITE);}
tft.drawLine(x_pos, (240-55)-(intensity[1]/sens_size_x/sens_size_y), x_pos, (240-55),TFT_GREEN);

 tft.setTextFont(0); 
   tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
 
  for (int sensor = 0; sensor < 8; sensor++){
/*Serial.print("intensity ");
Serial.print(sensor);
Serial.print(" : ");
   Serial.println(intensity[sensor]*3/sens_size_x/sens_size_y);
*/
    
 //   tft.drawLine(sensor*sens_spacing+sens_offset_x,sens_offset_y+20+center_y[sensor],sensor*sens_spacing+sens_offset_x+sens_size_x, sens_offset_y+20+center_y[sensor], TFT_BLUE);
//    tft.drawLine((sensor*sens_spacing+center_x[sensor]+sens_offset_x), 100, (sensor*sens_spacing+center_x[sensor]+sens_offset_x), 240,TFT_BLUE);
   tft.setCursor (20+sensor*25, 20);
 // tft.setFreeFont(FSS9); 
  tft.print(intensity[sensor]/sens_size_x/sens_size_y);
  tft.print("  ");
  
  }


   esp_camera_fb_return(image_fb);

    //  tft.fillRect(260, 0,80, 240, TFT_WHITE);

   //  p = ts.getPoint();
  //      tft.setCursor (260, 25);
 //   tft.print(240-p.x);
  //    tft.fillRect(260, 240-p.x-25,50, 50, TFT_BLACK);


   // };


}
