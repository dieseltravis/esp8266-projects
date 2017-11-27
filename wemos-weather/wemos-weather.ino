#include <FS.h> //this needs to be first
// ESP8266WiFi - Version: Latest 
#include <ESP8266WiFi.h>
// DNSServer - Version: Latest 
#include <DNSServer.h>
// ESP8266WebServer - Version: Latest 
#include <ESP8266WebServer.h>
// WiFiManager - Version: Latest
#include <WiFiManager.h>
// JSON parsing for Arduino
#include <ArduinoJson.h>

// WeMos SHT30 & OLED libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WEMOS_SHT3X.h>


// web server
//std::unique_ptr<ESP8266WebServer> wwwServer;
//#define HTML_FILE "/index.html"

#define CONFIG  "/config.json"

// wunderground constants * params
#define WU_BASE_API "http://api.wunderground.com/api/"
// Returns the API response in the specified language.
#define WU_LANG "lang:"
// Use personal weather stations for conditions.
#define WU_PWS "pws:"
// Use Weather Underground Best Forecast for forecast.
#define WU_BESTFCT "bestfct:"
// features & format of this code
#define WU_FEATURES "conditions/forecast"
#define WU_FORMAT "json"

// wunderground config params will be overwritten from CONFIG json file above
char wu_key[17] = "1234567890abcdef";
char wu_lang[3] = "EN";
char wu_pws[2] = "1";
char wu_bestfct[2] = "1";
// using ZIP code, but increase size if other query is needed
char wu_query[6] = "00000";


// WeMos
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

SHT3X sht30(0x45);


void setup() {
  Serial.begin(115200);

  // init values from file system
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    // parse json config file
    File jsonFile = GetFile(CONFIG);
    if (jsonFile) {
      size_t size = jsonFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> jsonBuf(new char[size]);
      jsonFile.readBytes(jsonBuf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(jsonBuf.get());
      if (json.success()) {
        strcpy(wu_key, json["wu_key"]);
        strcpy(wu_lang, json["wu_lang"]);
        strcpy(wu_pws, json["wu_pws"]);
        strcpy(wu_bestfct, json["wu_bestfct"]);
        strcpy(wu_query, json["wu_query"]);
      } else {
        Serial.println("failed to load json config");
      }
      jsonFile.close();
    }
  }

  // wifi setup
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Wifi connected ok. :)");
  Serial.println(WiFi.localIP());
 
  // web server
  //wwwServer.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  //wwwServer->on("/", Www_Root);
  //wwwServer->on("/feed", Www_Feed);
  //wwwServer->onNotFound(Www_NotFound);
  //wwwServer->begin();
  //Serial.println("HTTP Server Started");
  
  // OLED setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {
  // handle web requests
  //wwwServer->handleClient();
  //TODO: 
}


// file io
File GetFile(String fileName) {
  File textFile;
  if (SPIFFS.exists(fileName)) {
    textFile = SPIFFS.open(fileName, "r");
  }
  return textFile;
}

String GetWuUrl() {
  return WU_BASE_API + wu_key + "/" + 
    WU_LANG + wu_lang + "/" +
    WU_PWS + wu_pws + "/" +
    WU_BESTFCT + wu_bestfct + "/" +
    WU_FEATURES + "/" +
    "q/" + wu_query + 
    "." + WU_FORMAT;
}


// OLED
void Oled_ShowTemps() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  if (sht30.get() == 0) {
    display.println("T: ");
    display.setTextSize(2);
    display.print(sht30.fTemp);
		display.println("°F");

    display.setTextSize(1);
    display.println("H: ");
    display.setTextSize(2);
    display.print(sht30.humidity);
		display.println("%");
  } else {
    display.println("Error!");
  }
  
  display.display();
}


// WU data values to get:
// data.current_observation.temp_f (float)
// data.forecast.simpleforecast.forecastday[0].high.fahrenheit (string)
// data.forecast.simpleforecast.forecastday[0].low.fahrenheit (string)

//String GetTempFormat(String temp) {
//	return temp + "°F";
//}
