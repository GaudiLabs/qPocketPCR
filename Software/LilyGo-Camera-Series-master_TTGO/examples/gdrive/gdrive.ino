#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "base64.h"
#include "esp_camera.h"


const char *ssid = "your network SSID";              // your network SSID
const char *password = "your network password";     // your network password
String myScript = "/macros/s/xxxxxxxxxxxxxxxx/exec"; // Replace with your own url

const char *myDomain = "script.google.com";
String myFilename = "filename=ESP32-CAM.jpg";
String mimeType = "&mimetype=image/jpeg";
String myImage = "&data=";

int waitingTime = 30000; // Wait 30 seconds to google response.

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
// #define CAMERA_MODEL_WROVER_KIT // Has PSRAM
// #define CAMERA_MODEL_ESP_EYE // Has PSRAM
// #define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
// #define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
// #define CAMERA_MODEL_AI_THINKER // Has PSRAM
// #define CAMERA_MODEL_TTGO_T_CAMERA_V05
// #define CAMERA_MODEL_TTGO_T_CAMERA_V16
// #define CAMERA_MODEL_TTGO_T_CAMERA_V162
// #define CAMERA_MODEL_TTGO_T_CAMERA_V17
// #define CAMERA_MODEL_TTGO_T_CAMERA_MINI
// #define CAMERA_MODEL_TTGO_T_CAMERA_PLUS
// #define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// #define CAMERA_MODEL_TTGO_T_CAM_SIM // Has PSRAM
// ** Espressif Internal Boards **
// #define CAMERA_MODEL_ESP32_CAM_BOARD
// #define CAMERA_MODEL_ESP32S2_CAM_BOARD
// #define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

const char * ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFljCCA36gAwIBAgINAgO8U1lrNMcY9QFQZjANBgkqhkiG9w0BAQsFADBHMQsw\n" \
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n" \
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjAwODEzMDAwMDQyWhcNMjcwOTMwMDAw\n" \
"MDQyWjBGMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n" \
"Y2VzIExMQzETMBEGA1UEAxMKR1RTIENBIDFDMzCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggEPADCCAQoCggEBAPWI3+dijB43+DdCkH9sh9D7ZYIl/ejLa6T/belaI+KZ9hzp\n" \
"kgOZE3wJCor6QtZeViSqejOEH9Hpabu5dOxXTGZok3c3VVP+ORBNtzS7XyV3NzsX\n" \
"lOo85Z3VvMO0Q+sup0fvsEQRY9i0QYXdQTBIkxu/t/bgRQIh4JZCF8/ZK2VWNAcm\n" \
"BA2o/X3KLu/qSHw3TT8An4Pf73WELnlXXPxXbhqW//yMmqaZviXZf5YsBvcRKgKA\n" \
"gOtjGDxQSYflispfGStZloEAoPtR28p3CwvJlk/vcEnHXG0g/Zm0tOLKLnf9LdwL\n" \
"tmsTDIwZKxeWmLnwi/agJ7u2441Rj72ux5uxiZ0CAwEAAaOCAYAwggF8MA4GA1Ud\n" \
"DwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0T\n" \
"AQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUinR/r4XN7pXNPZzQ4kYU83E1HScwHwYD\n" \
"VR0jBBgwFoAU5K8rJnEaK0gnhS9SZizv8IkTcT4waAYIKwYBBQUHAQEEXDBaMCYG\n" \
"CCsGAQUFBzABhhpodHRwOi8vb2NzcC5wa2kuZ29vZy9ndHNyMTAwBggrBgEFBQcw\n" \
"AoYkaHR0cDovL3BraS5nb29nL3JlcG8vY2VydHMvZ3RzcjEuZGVyMDQGA1UdHwQt\n" \
"MCswKaAnoCWGI2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzcjEvZ3RzcjEuY3JsMFcG\n" \
"A1UdIARQME4wOAYKKwYBBAHWeQIFAzAqMCgGCCsGAQUFBwIBFhxodHRwczovL3Br\n" \
"aS5nb29nL3JlcG9zaXRvcnkvMAgGBmeBDAECATAIBgZngQwBAgIwDQYJKoZIhvcN\n" \
"AQELBQADggIBAIl9rCBcDDy+mqhXlRu0rvqrpXJxtDaV/d9AEQNMwkYUuxQkq/BQ\n" \
"cSLbrcRuf8/xam/IgxvYzolfh2yHuKkMo5uhYpSTld9brmYZCwKWnvy15xBpPnrL\n" \
"RklfRuFBsdeYTWU0AIAaP0+fbH9JAIFTQaSSIYKCGvGjRFsqUBITTcFTNvNCCK9U\n" \
"+o53UxtkOCcXCb1YyRt8OS1b887U7ZfbFAO/CVMkH8IMBHmYJvJh8VNS/UKMG2Yr\n" \
"PxWhu//2m+OBmgEGcYk1KCTd4b3rGS3hSMs9WYNRtHTGnXzGsYZbr8w0xNPM1IER\n" \
"lQCh9BIiAfq0g3GvjLeMcySsN1PCAJA/Ef5c7TaUEDu9Ka7ixzpiO2xj2YC/WXGs\n" \
"Yye5TBeg2vZzFb8q3o/zpWwygTMD0IZRcZk0upONXbVRWPeyk+gB9lm+cZv9TSjO\n" \
"z23HFtz30dZGm6fKa+l3D/2gthsjgx0QGtkJAITgRNOidSOzNIb2ILCkXhAd4FJG\n" \
"AJ2xDx8hcFH1mt0G/FX0Kw4zd8NLQsLxdxP8c4CU6x+7Nz/OAipmsHMdMqUybDKw\n" \
"juDEI/9bfU1lcKwrmz3O2+BtjjKAvpafkmO8l7tdufThcV4q5O8DIrGKZTqPwJNl\n" \
"1IXNDw9bg1kWRxYtnCQ6yICmJhSFm/Y3m6xv+cXDBlHz4n/FsRC6UfTd\n" \
"-----END CERTIFICATE-----" ;

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(10);

    WiFi.mode(WIFI_STA);

    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8, 8, 4, 4), WiFi.gatewayIP());

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

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
    config.frame_size = FRAMESIZE_VGA; // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
    config.jpeg_quality = 10;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        delay(1000);
        ESP.restart();
    }
}


void loop()
{
    saveCapturedImage();
    delay(60 * 1000);
}


void saveCapturedImage()
{
    Serial.println("Connect to " + String(myDomain));
    WiFiClientSecure client;
    // IPAddress ip(142, 250, 188, 14);

    client.setCACert(ca);

    if (client.connect(myDomain, 443))
    {
        Serial.println("Connection successful");

        camera_fb_t *fb = NULL;
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            delay(1000);
            ESP.restart();
            return;
        }

        // char *input = (char *)fb->buf;
        // char output[base64_enc_len(3)];
        // String imageFile = "";
        // for (int i = 0; i < fb->len; i++)
        // {
        //     base64_encode(output, (input++), 3);
        //     if (i % 3 == 0)
        //         imageFile += urlencode(String(output));
        // }
        String imageFile = urlencode(base64::encode(fb->buf, fb->len));
        String Data = myFilename + mimeType + myImage;

        esp_camera_fb_return(fb);

        Serial.println("Send a captured image to Google Drive.");

        client.println("POST " + myScript + " HTTP/1.1");
        client.println("Host: " + String(myDomain));
        client.println("Content-Length: " + String(Data.length() + imageFile.length()));
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.println();

        client.print(Data);
        int Index;
        for (Index = 0; Index < imageFile.length(); Index = Index + 1000)
        {
            client.print(imageFile.substring(Index, Index + 1000));
        }

        Serial.println("Waiting for response.");
        long int StartTime = millis();
        while (!client.available())
        {
            Serial.print(".");
            delay(100);
            if ((StartTime + waitingTime) < millis())
            {
                Serial.println();
                Serial.println("No response.");
                // If you have no response, maybe need a greater value of waitingTime
                break;
            }
        }
        Serial.println();
        while (client.available())
        {
            Serial.print(char(client.read()));
        }
    }
    else
    {
        Serial.println("Connected to " + String(myDomain) + " failed.");
    }
    client.stop();
}

// https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            code2 = '\0';
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
            // encodedString+=code2;
        }
        yield();
    }
    return encodedString;
}