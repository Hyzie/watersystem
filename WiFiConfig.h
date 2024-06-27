#include <WiFi.h>
#include "Arduino.h"
// #include "StringEEPROM.h"

/*----------SSID AND PASSWORD OF SOFT AP WIFI--------------*/
const char* AP_ssid = "Water System WiFi Config";
const char* AP_password = "";
/*---------------------------------------------------------*/


/*------SSID AND PASSWORD OF WIFI FROM EEPROM--------------*/
// const char* ssid = "";
// const char* pass = "";
/*---------------------------------------------------------*/


WiFiServer server(80);

String header;



unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void WiFi_Config(String *inputSSID, String *inputPASS)
{
  Serial.println("Setting up AP...");
  WiFi.softAP(AP_ssid, AP_password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  server.begin();
  bool received = false;
  while(1)
  {
    WiFiClient client = server.available(); 

    if (client) {  
      for(int i=0; i<2; i++)
      {  
        digitalWrite(18, HIGH);
        digitalWrite(19, HIGH);
        delay(500);
        digitalWrite(18, LOW);
        digitalWrite(19, LOW);
        delay(500);
      }

      currentTime = millis();
      previousTime = currentTime;
      Serial.println("SSID and Password from web server:");          
      String currentLine = "";                
      while (client.connected() && currentTime - previousTime <= timeoutTime) { 
        currentTime = millis();
        if (client.available()) {             
          char c = client.read();             
          header += c;
          if (c == '\n') {                    
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              if (header.indexOf("GET /?input1=") >= 0) {
                int input1Index = header.indexOf("input1=") + 7;
                int input1EndIndex = header.indexOf("&", input1Index);
                *inputSSID = header.substring(input1Index, input1EndIndex);

                int input2Index = header.indexOf("input2=") + 7;
                int input2EndIndex = header.indexOf(" ", input2Index);
                *inputPASS = header.substring(input2Index, input2EndIndex);


                Serial.print("- SSID: "); 
                Serial.println(*inputSSID);
                Serial.print("- Password: ");
                Serial.println(*inputPASS);

                received = true;
              }

              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #00FFFF;}</style>");
              client.println("<script>");
              client.println("function hideMessage() {");
              client.println("  var message = document.getElementById('confirmation');");
              client.println("  if (message) {");
              client.println("    setTimeout(function() { message.style.display = 'none'; }, 2000);");
              client.println("  }");
              client.println("}");
              client.println("</script>");
              client.println("</head>");
              client.println("<body onload=\"hideMessage()\"><h1>Enter WIFI SSID and Password:</h1>");
              client.println("<form action=\"/\" method=\"GET\">");
              client.println("SSID: <input type=\"text\" name=\"input1\"><br><br>");
              client.println("Password: <input type=\"text\" name=\"input2\"><br><br>");
              client.println("<input type=\"submit\" value=\"Submit\" class=\"button\"></form>");
              if (received) {
                // client.println("<p id=\"confirmation\">Complete sending SSID and Password to ESP32.</p>");
                client.println("<div id=\"confirmation-box\" style=\"border: 1px solid #ccc; background-color: #f2f2f2; padding: 10px; margin-top: 10px;\">");
                client.println("<p id=\"confirmation\">Complete sending SSID and Password to ESP32.</p>");
                client.println("</div>");
              }
              client.println("<script>");
              client.println("function hideMessage() {");
              client.println("  var confirmationBox = document.getElementById('confirmation-box');");
              client.println("  if (confirmationBox) {");
              client.println("    setTimeout(function() { confirmationBox.style.display = 'none'; }, 2000);");
              client.println("  }");
              client.println("}");
              client.println("</script>");
              
              client.println("</body></html>");

              client.println();
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;      
          }
        }
      }
      header = "";
    }
    if(received == true)
    {
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
      break;
    }
  }
}
