/*
* Script_01_ESP32_STA_AP_WebServer.ino     
*
* --> Sketch sent an ESP32 module
* --> The wi-fi mode here works in the AP (Access Point) and STA (station) modes simultaneously  
*     
*     
*/



/*************************
** Include header files **
*************************/
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESPAsyncWebServer.h> // --> Async web server
#include "pag_web.h"


/*****************************************
** Global scope constants and variables **
*****************************************/
/* --> Time-Zone adjustment (in seconds):
   --> Difference, in seconds, between the hour in this time-zone and the time in Greenwich
   --> Brasilia TZ (GMT -3) = -3600 * 3 = -10800 */
#define TZ_ADJUST -10800

// Soft AP constants
#define SSID_AP "Soft-Access-Point"
#define PASSWORD_AP "123456789"
#define CHAN_AP 2 // Wi-Fi Channel (1 to 13)

// Wi-Fi constants
#define WIFI_SSID "YOUR WI-FI NETWORK SSID"
#define WIFI_PASSWORD "YOUR WI-FI NETWORK PASSWORD"


// One second interval
#define T_MS_1_SEC 1000 // One second interval
#define T_MS_15_SEC 15000 // 15 second interval
// --> Global Variables
unsigned long previousMillis = 0, previousMillis2 = 0;
unsigned long currentMillis;
String formattedDate;
String dayStamp;
String timeStamp;
IPAddress AP_IP; //IP 
IPAddress WIFI_IP; //IP Address of the ESP32 in the Wi-Fi Network



/************************************************************ 
** Objects from classes defined in the imported libraries  **   
************************************************************/  
AsyncWebServer server(80); // Asynchronous web server on port '8080'
// NTP Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); //NTP client
    





/******************************
** Function prototypes       **
******************************/
void create_esp32_soft_ap( char* soft_ap_ssid, char* soft_ap_pass, int chan_ap, IPAddress *ip_addr );
void connect_esp32_wifi_network( char* ssid, char* password, IPAddress *ip_addr );
String processor( const String& var );
void getTimeStamp( void );



/*****************************
** Set static IP parameters **
*****************************/
// Set your Static IP address
IPAddress local_IP(192, 168, 100, 133); //This modules IP Address

// Set your Gateway IP address
IPAddress gateway(192, 168, 100, 1); //Router address
IPAddress subnet(255, 255, 0, 0); 
IPAddress primaryDNS(8, 8, 8, 8); //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional


/******************
** setup()       ** 
******************/
void setup(){
      Serial.begin(115200);
      delay(3000);

      
      // --> Create the Soft Access Point
      create_esp32_soft_ap( SSID_AP, PASSWORD_AP, CHAN_AP, &AP_IP );

      // Set the static IP parameters
      if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            Serial.println("STA Failed to configure");
      }
      //--> Connects the ESP32 to the Wi-Fi network
      connect_esp32_wifi_network(WIFI_SSID, WIFI_PASSWORD, &WIFI_IP);

      
      //--> NTP server
      Serial.println("Initialing the NTP server");
      timeClient.begin();
      timeClient.setTimeOffset(TZ_ADJUST);
      getTimeStamp();
      
      
      // --> Configure the asynchronous web server
      // Route for root / web page
      server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/html", index_html, processor); } ); //

      // Date:
      server.on("/curr_date", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", dayStamp.c_str()); });

      // Hour:
      server.on("/curr_time", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", timeStamp.c_str()); });

      // Video Stream
      server.on("/stream", HTTP_GET, [](AsyncWebServerRequest * request) { request->send_P(200, "text/plain", String("").c_str()); });
      // --> Start the asynchronous web server
      Serial.println("Starting the Web Server");
      server.begin();
}




/****************
** loop()      **  
****************/
void loop(){
      //Current timestamp
      currentMillis = millis();
      
      if( (currentMillis - previousMillis) >= T_MS_1_SEC ){
            previousMillis = currentMillis;
            getTimeStamp();
      }

      if( (currentMillis - previousMillis2) >= T_MS_15_SEC ){
            previousMillis2 = currentMillis;
            Serial.print(dayStamp); Serial.print(" - ");
            Serial.print(timeStamp); Serial.print("\t");
            //Serial.print("Connected devices: ");
            //Serial.println(WiFi.softAPgetStationNum());
      }
}




/******************************
** Function definitions      **
******************************/
// --> create_esp32_soft_ap()
void create_esp32_soft_ap( char* soft_ap_ssid, char* soft_ap_pass, int chan_ap, IPAddress *ip_addr ){
      // --> Create the Soft Access Point
      Serial.println("------------------------------------------------------------");
      Serial.print("Creating the soft-AP...");
      WiFi.softAP(soft_ap_ssid, soft_ap_pass, chan_ap, false, 8);
      *ip_addr = WiFi.softAPIP();
      Serial.print("IP do AP ");
      Serial.print(soft_ap_ssid);
      Serial.print(": ");
      Serial.println(*ip_addr);
      Serial.print("MAC Address do Soft AP: ");
      Serial.println(WiFi.softAPmacAddress());
      Serial.println("------------------------------------------------------------");
}


// --> 'connect_esp32_wifi_ap_sta()'
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




// --> getTimeStamp()
void getTimeStamp( void ) {
      while (!timeClient.update()) {
            timeClient.forceUpdate();
      }
      
      // --> Date format in 'formattedDate': 2018-05-28T16:00:13Z
      // --> Its needed to extract the date and the time
      formattedDate = timeClient.getFormattedDate();
      //Serial.println(formattedDate);

      // --> Extract the date
      int splitT = formattedDate.indexOf("T"); //Position of the character 'T' in the string
      dayStamp = formattedDate.substring(0, splitT);
      //Serial.println(dayStamp);

      // --> Extract the hour
      timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
      //Serial.println(timeStamp);
}




// --> 'processor()
String processor( const String& var ){
       //-->  If 'var' is HOUR_DATA:
      if (var == "CURR_DATE") { return String(dayStamp); }
      //--> If 'var' is HOUR_PHOTO:
      else if (var == "CURR_TIME") { return String(timeStamp); }
      // --> Else, return empty string
      return String();
}
