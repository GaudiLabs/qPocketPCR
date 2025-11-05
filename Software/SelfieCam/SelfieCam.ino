#include <ESPAsyncWebServer.h>
#include "esp_camera.h"
#include "camera_index.h"
#include "Arduino.h"
//#include "fd_forward.h"

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include <TFT_eFEX.h>
TFT_eFEX  fex = TFT_eFEX(&tft);

AsyncWebServer webserver(80);
AsyncWebSocket ws("/ws");

const char* ssid = "GaudiLabs";
const char* password = "versonet";

String filelist;
camera_fb_t * fb = NULL;
String incoming;
long current_millis;
long last_capture_millis = 0;
static esp_err_t cam_err;
static esp_err_t card_err;
char strftime_buf[64];
long file_number = 0;

#define PWDN_GPIO_NUM     32
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
  Serial.begin(115200);
  
  pinMode(4, OUTPUT);// initialize io4 as an output for LED flash.
  digitalWrite(4, LOW); // flash off/
  
  init_wifi();

  tft.begin();
  tft.setRotation(3);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(35,55);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println(WiFi.localIP());
  delay(5000);

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
  config.pixel_format = PIXFORMAT_JPEG;
  
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  cam_err = esp_camera_init(&config);
  if (cam_err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", cam_err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QQVGA);
  s->set_vflip(s, 1);

  ws.onEvent(onEvent);
  webserver.addHandler(&ws);

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.print("Sending interface...");
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_ov2640_html_gz, sizeof(index_ov2640_html_gz));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webserver.on("/image", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("requesting image from SPIFFS");
    if (request->hasParam("id")) {
      AsyncWebParameter* p = request->getParam("id");
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      String imagefile = p->value();
      imagefile = imagefile.substring(4); // remove img_
      request->send(SPIFFS, "/" + imagefile);
    }
  });

  webserver.begin();

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    SPIFFS.begin(true);// Formats SPIFFS - could lose data https://github.com/espressif/arduino-esp32/issues/638
  }
  Serial.println("\r\nInitialisation done.");

  fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS
}

bool init_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  return true;
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  // String incoming = String((char *)data); No idea why.. gave extra characters in data for short names.
  // so ....
  for (size_t i = 0; i < len; i++) {
    incoming += (char)(data[i]);
  }
  Serial.println(incoming);

  if (incoming.substring(0, 7) == "delete:") {
    String deletefile = incoming.substring(7);
    incoming = "";
    int fromUnderscore = deletefile.lastIndexOf('_') + 1;
    int untilDot = deletefile.lastIndexOf('.');
    String fileId = deletefile.substring(fromUnderscore, untilDot);
    Serial.println(fileId);
    Serial.println("image delete");
    SPIFFS.remove("/selfie_t_" + fileId + ".jpg");
    SPIFFS.remove("/selfie_f_" + fileId + ".jpg");
    client->text("removed:" + deletefile); // file deleted. request browser update
    
  } else {
    Serial.println("sending list");
    client->text(filelist_spiffs());
  }
}

String filelist_spiffs()
{

  filelist = "";
  fs::File root = SPIFFS.open("/");

  fs::File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    // Serial.println(fileName);
    filelist = filelist + fileName;
    file = root.openNextFile();
  }
  Serial.println(filelist);
  return filelist;
}

void latestFileSPIFFS()
{
  fs::File root = SPIFFS.open("/");

  fs::File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    Serial.println(fileName);
    int fromUnderscore = fileName.lastIndexOf('_') + 1;
    int untilDot = fileName.lastIndexOf('.');
    String fileId = fileName.substring(fromUnderscore, untilDot);
    Serial.println(fileId);
    file_number = max(file_number, fileId.toInt()); // replace filenumber if fileId is higher
    file = root.openNextFile();
  }
}



static esp_err_t take_photo()
{
  latestFileSPIFFS(); // next file number
  file_number++;
  Serial.println(file_number);
  Serial.println("Starting thumb capture: ");
  fb = esp_camera_fb_get();
  fex.drawJpg((const uint8_t*)fb->buf, fb->len, 0, 6);
  //save thumb
  char *thumb_filename = (char*)malloc(23 + sizeof(file_number));
  sprintf(thumb_filename, "/spiffs/selfie_t_%d.jpg", file_number);
  Serial.println("Opening file: ");
  FILE *thumbnail = fopen(thumb_filename, "w");
  if (thumbnail != NULL)  {
    size_t err = fwrite(fb->buf, 1, fb->len, thumbnail);
    Serial.printf("File saved: %s\n", thumb_filename);
  }  else  {
    Serial.println("Could not open file");
  }
  fclose(thumbnail);
  esp_camera_fb_return(fb);
  fb = NULL;
  free(thumb_filename);

  Serial.println("Starting main capture: ");
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SVGA);
  delay(500);
  fb = esp_camera_fb_get();
  char *full_filename = (char*)malloc(23 + sizeof(file_number));
  sprintf(full_filename, "/spiffs/selfie_f_%d.jpg", file_number);
  Serial.println("Opening file: ");
  FILE *fullres = fopen(full_filename, "w");
  if (fullres != NULL)  {
    size_t err = fwrite(fb->buf, 1, fb->len, fullres);
    Serial.printf("File saved: %s\n", full_filename);
  }  else  {
    Serial.println("Could not open file");
  }
  fclose(fullres);
  esp_camera_fb_return(fb);
  fb = NULL;
  free(full_filename);

  s->set_framesize(s, FRAMESIZE_QQVGA);
  delay(500);
  char *addtobrowser = (char*)malloc(24 + sizeof(file_number));
  sprintf(addtobrowser, "added:selfie_t_%d.jpg", file_number);
  ws.textAll((char*)addtobrowser);// file added. request browser update
}


void loop()
{
 
  {
    fb = esp_camera_fb_get();
    fex.drawJpg((const uint8_t*)fb->buf, fb->len, 0, 6);
    esp_camera_fb_return(fb);
    fb = NULL;
  }
}
