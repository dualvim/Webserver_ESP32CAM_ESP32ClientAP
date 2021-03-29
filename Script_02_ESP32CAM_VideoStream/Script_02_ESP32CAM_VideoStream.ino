/*
* Script_02_ESP32CAM_VideoStream.ino     
* 
* --> Template with the structure and functions used ina an ESP32-CAM project
*
*/


/*************************
** Include header files **
*************************/
#include <Arduino.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "esp_http_server.h"
#include <FS.h>            //File System
#include <SPIFFS.h>        // SPIFFS of the ESP32
#include <EEPROM.h>        // read and write from flash memory
#include "SD_MMC.h"        // SD Card ESP32
#include <WiFi.h> // For resources related to Wi-Fi and OTA
#include <WiFiUdp.h>




/*****************************
** Choose your camera model **
*****************************/
// --> Select the camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
//#define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_TTGO_TCAMERA_PLUS

// --> Include the header file with the pins of the boards above
#include "camera_pins.h"




/*************************************************************
** Constants and Global Variables                           **
*************************************************************/
// --> Simbolic constants: ESP32 pins connected to the devices
//#define FLASH_PIN 4 // Pin of the ESP32-CAM which is connected to the camera's flash 
//#define LED_PIN 2


// Wi-Fi mode
#define ESP_WIFI_MODE WIFI_STA // Station mode only (it connects the ESP32 to a Wi-Fi router)
//#define ESP_WIFI_MODE WIFI_AP // Access Point mode only (the ESP32 creates a Wi-Fi network where the devices connects to it)
//#define ESP_WIFI_MODE WIFI_AP_STA // Create an Access Point and connects the ESP32 to a Wi-Fi Network
//#define ESP_WIFI_MODE WIFI_OFF //Turn off the Wi-Fi

// --> Wi-Fi parameters for the station mode 
//#define WIFI_SSID "WiFi_7"
//#define WIFI_PASSWORD "!Gr@z&3dU$"
#define WIFI_SSID "Soft-Access-Point"
#define WIFI_PASSWORD "123456789"

// --> Constants related to the Stream Web Server
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";



// --> Global scope variables
IPAddress WIFI_IP; //IP Address of the ESP32 in the Wi-Fi Network
camera_config_t CONFIG_CAM;  // OV2640 camera configuration
httpd_handle_t stream_httpd = NULL; //Stream handler




/************************************************************ 
** Objects from classes defined in the imported libraries  **   
************************************************************/  




/******************************
** Function prototypes       **
******************************/
void connect_esp32_wifi_network( char* ssid, char* password, IPAddress *ip_addr );
void ov2640_camera_module_configurations( void );
static esp_err_t stream_handler( httpd_req_t *req );
void startCameraServer();




/*****************************
** Set static IP parameters **
*****************************/
// Set your Static IP address
IPAddress local_IP(192, 168, 4, 4); //This modules IP Address

// Set your Gateway IP address
IPAddress gateway(192, 168, 4, 1); //ESP32 with Access Point IP address
IPAddress subnet(255, 255, 0, 0); 
IPAddress primaryDNS(8, 8, 8, 8); //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional




/******************
** setup()       ** 
******************/
void setup() {
      // Disable the 'browout detector'
      WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
      
      // --> Activate the serial conection and waits 5 seconds
      Serial.begin( 115200 ); 
      delay(5000);
      Serial.println("Initializing the ESP32-CAM Video Streamming Web Server...");
      
      // --> Intialize the OV2640 camera module
      ov2640_camera_module_configurations(); 
      
      // --> Wi-Fi Mode:
      WiFi.mode(ESP_WIFI_MODE);

      // Set the static IP parameters
      if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            Serial.println("STA Failed to configure");
      }
      
      //--> Connect the ESP32 to the Wi-Fi network
      connect_esp32_wifi_network(WIFI_SSID, WIFI_PASSWORD, &WIFI_IP);

      // --> Starts the video streamming server
      startCameraServer();
}




/****************
** loop()      **  
****************/
void loop() {
      delay(1);
}




/********************************************
** 'connect_esp32_wifi_network()' function **
********************************************/
//--> connect_esp32_wifi_network(): Conects the ESP32 to the Wi-Fi network
void connect_esp32_wifi_network( char* ssid, char* password, IPAddress *ip_addr ){
      // --> Connect the ESP32 to the Wi-Fi Network:
      // Connect to the Wi-Fi network
      WiFi.begin(ssid, password);
      Serial.print("Connecting to ");
      Serial.print(ssid);
      while( WiFi.status() != WL_CONNECTED ){
            Serial.print(".");
            delay(500);
      }
      Serial.println("Ok\nConected!");
      // Show IP and MAC Address:
      Serial.print("IP Address: ");
      *ip_addr = WiFi.localIP();
      Serial.print(*ip_addr);
      Serial.print("; MAC Address: ");
      Serial.println(WiFi.macAddress());
      Serial.println("------------------------------------------------------------\n");
}




/***********************************************
** ov2640_camera_module_configurations()      **  
***********************************************/
//--> 'activate_ota()': Configure and enables the OTA (On The Air)
void ov2640_camera_module_configurations( void ){
      Serial.print("Inicializing the OV2640 camera module on the ESP32-CAM...");
      // --> OV2640 camera configuration
      CONFIG_CAM.ledc_channel = LEDC_CHANNEL_0;
      CONFIG_CAM.ledc_timer = LEDC_TIMER_0;
      CONFIG_CAM.pin_d0 = Y2_GPIO_NUM;
      CONFIG_CAM.pin_d1 = Y3_GPIO_NUM;
      CONFIG_CAM.pin_d2 = Y4_GPIO_NUM;
      CONFIG_CAM.pin_d3 = Y5_GPIO_NUM;
      CONFIG_CAM.pin_d4 = Y6_GPIO_NUM;
      CONFIG_CAM.pin_d5 = Y7_GPIO_NUM;
      CONFIG_CAM.pin_d6 = Y8_GPIO_NUM;
      CONFIG_CAM.pin_d7 = Y9_GPIO_NUM;
      CONFIG_CAM.pin_xclk = XCLK_GPIO_NUM;
      CONFIG_CAM.pin_pclk = PCLK_GPIO_NUM;
      CONFIG_CAM.pin_vsync = VSYNC_GPIO_NUM;
      CONFIG_CAM.pin_href = HREF_GPIO_NUM;
      CONFIG_CAM.pin_sscb_sda = SIOD_GPIO_NUM;
      CONFIG_CAM.pin_sscb_scl = SIOC_GPIO_NUM;
      CONFIG_CAM.pin_pwdn = PWDN_GPIO_NUM;
      CONFIG_CAM.pin_reset = RESET_GPIO_NUM;
      CONFIG_CAM.xclk_freq_hz = 20000000;
      CONFIG_CAM.pixel_format = PIXFORMAT_JPEG;
      // --> Frame size:
      if (psramFound()) {
            CONFIG_CAM.frame_size = FRAMESIZE_HVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
            CONFIG_CAM.jpeg_quality = 10;
            CONFIG_CAM.fb_count = 2;
      } 
      else {
            CONFIG_CAM.frame_size = FRAMESIZE_QVGA;
            CONFIG_CAM.jpeg_quality = 12;
            CONFIG_CAM.fb_count = 1;
      }
      // --> Initialize the camera module
      esp_err_t err = esp_camera_init(&CONFIG_CAM);
      if (err != ESP_OK) {
            Serial.printf("Camera init failed with error 0x%x", err);
            return;
      }
      Serial.println("Ok!");
}




/************************************
** esp_err_t stream_handler()      **  
************************************/
static esp_err_t stream_handler( httpd_req_t *req ){
      camera_fb_t * fb = NULL;
      esp_err_t res = ESP_OK;
      size_t _jpg_buf_len = 0;
      uint8_t * _jpg_buf = NULL;
      char * part_buf[64];
      
      res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
      if(res != ESP_OK){  return res;  }

      while(true){
            fb = esp_camera_fb_get();
            if (!fb) {
                  Serial.println("Camera capture failed");
                  res = ESP_FAIL;
            } 
            else {
                  if(fb->width > 400){
                        if(fb->format != PIXFORMAT_JPEG){
                        
                              bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                              esp_camera_fb_return(fb);
                              fb = NULL;
                        
                              if(!jpeg_converted){
                                    Serial.println("JPEG compression failed");
                                    res = ESP_FAIL;
                              }
                        } 
                        else {
                              _jpg_buf_len = fb->len;
                              _jpg_buf = fb->buf;
                        }
                  }
            }

            if(res == ESP_OK){
                  size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
                  res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
            }
            
            if(res == ESP_OK){
                  res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            }
            
            if(res == ESP_OK){
                  res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            }
    
            if(fb){
                  esp_camera_fb_return(fb);
                  fb = NULL;
                  _jpg_buf = NULL;
            } 
            else if(_jpg_buf){
                  free(_jpg_buf);
                  _jpg_buf = NULL;
            }

    
            if(res != ESP_OK){
                  break;
            }
            //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
      }
      return res;
}




/*****************************
** startCameraServer()      **  
*****************************/
void startCameraServer(){
      httpd_config_t config = HTTPD_DEFAULT_CONFIG();
      config.server_port = 80;
      
      httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = stream_handler,
            .user_ctx  = NULL
      };
      
      //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
      if (httpd_start(&stream_httpd, &config) == ESP_OK) {
            httpd_register_uri_handler(stream_httpd, &index_uri);
      }
}
