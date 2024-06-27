#include<WiFi.h>
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "WiFiConfig.h"
#include "StringEEPROM.h"
#include "DHT.h"

#define API_KEY "AIzaSyARfDnGN5GlyQmGEUu0nxwcEXyTm8IagXE"
#define DATABASE_URL "https://water-control-5fade-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define LEDRED 18
#define LEDYELLOW 19
#define MOISTURE_SENSOR 21
#define TEMP_SENSOR 4
#define BUTTON 23

#define VALVE01 33
#define VALVE02 25
#define VALVE03 26
#define VALVE04 27
#define PUMP 32

#define DHTTYPE DHT11
DHT dht(TEMP_SENSOR, DHTTYPE);

FirebaseData fdbo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

int ValveCount = 0;


void WiFiConfig()
{
  pinMode(BUTTON, INPUT_PULLUP);
  int Wait = 50;
  int mode = 0;
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDYELLOW, OUTPUT);
  while(Wait != 0)
  {
    digitalWrite(LEDRED, HIGH);
    digitalWrite(LEDYELLOW, LOW);
    // Serial.println(digitalRead(BUTTON));
    if(digitalRead(BUTTON) == 0)
    {
      mode = 1;
      digitalWrite(LEDRED, LOW);
      digitalWrite(LEDYELLOW, LOW);
      break;
    }
    delay(100);
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDYELLOW, HIGH);
    delay(100);
    Wait--;
  }

  digitalWrite(LEDRED, LOW);
  digitalWrite(LEDYELLOW, LOW);

  if(mode == 1)
  {
    String inputSSID;
    String inputPASS;

    WiFi_Config(&inputSSID, &inputPASS);

    Serial.println(inputSSID);
    Serial.println(inputPASS);

    if (!EEPROM.begin(EEPROM_SIZE)) 
    {
      Serial.println("Failed to initialise EEPROM");
      return;
    }

    Serial.println("Writing SSID and Password into EEPROM...");

    WriteStringToEEPROM(0, inputSSID);
    WriteStringToEEPROM(20, inputPASS);

    Serial.println("Done configuration.");
  }
}

void WiFiConnect()
{
  // Read WiFi SSID and Password from EEPROM
  if (!EEPROM.begin(EEPROM_SIZE)) 
  {
    Serial.println("Failed to initialise EEPROM");
    return;
  }
    
  char* ssid = "";
  String dataRead = ReadStringFromEEPROM(0);
  ssid = stringToCharArray(dataRead);
  Serial.println(ssid);

  char* pass = "";
  String dataRead1 = ReadStringFromEEPROM(20);
  pass = stringToCharArray(dataRead1);
  Serial.println(pass);

  // Connect to WiFi
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to network.");
} 

void FirebaseConnect()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Sign Up OK");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void OpenValve(int *count, int pin)
{
  if(*count == 0)
  {
    digitalWrite(PUMP, HIGH);
  }

  digitalWrite(pin, HIGH);
  *count++;
}

void CloseValve(int *count, int pin)
{
  digitalWrite(pin, LOW);
  *count--;

  if(*count == 0)
  {
    digitalWrite(PUMP, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(VALVE01, OUTPUT);
  pinMode(VALVE02, OUTPUT);
  pinMode(VALVE03, OUTPUT);
  pinMode(VALVE04, OUTPUT);
  pinMode(PUMP, OUTPUT);

  pinMode(TEMP_SENSOR, INPUT);
  pinMode(MOISTURE_SENSOR, INPUT_PULLUP);

  WiFiConfig();
  WiFiConnect();
  FirebaseConnect();
}

  

void loop() {
  // SEND SENSOR DATA TO FIREBASE
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    
    float humid = dht.readHumidity();
    float temp = dht.readTemperature();
    float f = dht.readTemperature(true);

    String moist = "Dry";

    if(digitalRead(MOISTURE_SENSOR) == 0)
    {
      moist = "Wet";
    }

    if (isnan(humid) || isnan(temp) || isnan(f)) {
      humid = -1;
      temp = -1;
      // return;
    }

    // HUMIDITY
    if(Firebase.RTDB.setFloat(&fdbo, "Sensor/HUMID", humid))
    {
      Serial.println();
      Serial.print(humid);
      Serial.print(" - successfully saved to: " + fdbo.dataPath());
      Serial.println(" (" + fdbo.dataType() + ")");
    }
    else
      Serial.println("FAILED: " + fdbo.errorReason());

    // TEMPERATURE
    if(Firebase.RTDB.setFloat(&fdbo, "Sensor/TEMP", temp))
    {
      Serial.println();
      Serial.print(temp);
      Serial.print(" - successfully saved to: " + fdbo.dataPath());
      Serial.println(" (" + fdbo.dataType() + ")");
    }
    else
      Serial.println("FAILED: " + fdbo.errorReason());

    // MOISTURE STATUS
    if(Firebase.RTDB.setString(&fdbo, "Sensor/SOILSTATUS", moist))
    {
      Serial.println();
      Serial.print(moist);
      Serial.print(" - successfully saved to: " + fdbo.dataPath());
      Serial.println(" (" + fdbo.dataType() + ")");
    }
    else
      Serial.println("FAILED: " + fdbo.errorReason());

    // CONTROL PUMP
    if(Firebase.RTDB.getBool(&fdbo, "CONTROL/PUMP"))
    {
      if(fdbo.dataType() = "boolean")
      {
        if(fdbo.boolData() == true)
        {
          digitalWrite(PUMP, HIGH);
        }
        else
        {
          digitalWrite(PUMP, LOW);
        }
      }
    }
    else
    {
      Serial.println("FAILED: " + fdbo.errorReason());
    }

    // CONTROL VALVE01
    if(Firebase.RTDB.getBool(&fdbo, "CONTROL/VALVE01"))
    {
      if(fdbo.dataType() = "boolean")
      {
        if(fdbo.boolData() == true)
        {
          digitalWrite(VALVE01, HIGH);
        }
        else
        {
          digitalWrite(VALVE01, LOW);
        }
      }
    }
    else
    {
      Serial.println("FAILED: " + fdbo.errorReason());
    }

    // CONTROL VALVE02
    if(Firebase.RTDB.getBool(&fdbo, "CONTROL/VALVE02"))
    {
      if(fdbo.dataType() = "boolean")
      {
        if(fdbo.boolData() == true)
        {
          digitalWrite(VALVE02, HIGH);
        }
        else
        {
          digitalWrite(VALVE02, LOW);
        }
      }
    }
    else
    {
      Serial.println("FAILED: " + fdbo.errorReason());
    }

    // CONTROL VALVE03
    if(Firebase.RTDB.getBool(&fdbo, "CONTROL/VALVE03"))
    {
      if(fdbo.dataType() = "boolean")
      {
        if(fdbo.boolData() == true)
        {
          digitalWrite(VALVE03, HIGH);
        }
        else
        {
          digitalWrite(VALVE03, LOW);
        }
      }
    }
    else
    {
      Serial.println("FAILED: " + fdbo.errorReason());
    }

    // CONTROL VALVE04
    if(Firebase.RTDB.getBool(&fdbo, "CONTROL/VALVE04"))
    {
      if(fdbo.dataType() = "boolean")
      {
        if(fdbo.boolData() == true)
        {
          digitalWrite(VALVE04, HIGH);
        }
        else
        {
          digitalWrite(VALVE04, LOW);
        }
      }
    }
    else
    {
      Serial.println("FAILED: " + fdbo.errorReason());
    }
  }
}
