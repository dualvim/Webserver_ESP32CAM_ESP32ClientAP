/* pag_web.h
 * 
 * --> HTML script of the web server hosted on the ESP32
 * 
 */



 const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
      <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">

      <head>
            <title>ESP32 Web Server</title>
      </head>

      <body>
            <h1>Current Date and Time:</h1>
            <p><span id="curr_date">%CURR_DATE%</span></p>
            <p><span id="curr_time">%CURR_TIME%</span></p>
      </body>
      <br>
      <iframe name="stream" id="stream" src="http://192.168.4.4" width=480 height=320></iframe>

      
      <script>
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("curr_date").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/curr_date", true);
                  xhttp.send();
            }, 10000 );

            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("curr_time").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/curr_time", true);
                  xhttp.send();
            }, 10000 );

            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("stream").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/stream", true);
                  xhttp.send();
            }, 10000 );
      </script>
</html>)rawliteral";
