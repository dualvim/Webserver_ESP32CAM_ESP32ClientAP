# Webserver using ESP32-CAM and ESP32 boards       
 - This project is formed by two ESP32 Projects:      
	- **Project 1**: **`Script_01_ESP32_STA_AP_WebServer`**      
		- This board will create an Access Point called **`Soft-Access-Point`**, with password **`123456789`**.     
		- Also, the ESP32 board connects to the local Wi-Fi network.       
		- This sketch creates a web server which shows the current date and time (taken from the NTP server) and a frame wich connects with the ESP32-CAM.      
		- Files:    
			- **`Script_01_ESP32_STA_AP_WebServer.ino`**: Arduino sketch loaded to the ESP32.      
			- **`pag_web.h`**: Contais the definition of the constant **`index_html`**, which holds the HTML script of the web server.         
	- **Project 2**: **`Script_02_ESP32CAM_VideoStream`**       
		- This project contais the sketch sent to the ESP32-CAM.      
		- It creaes a PHP server wich shows the video stream.     
		 