#include <FS.h> //this needs to be first
// ESP8266WiFi - Version: Latest
#include <ESP8266WiFi.h>
// DNSServer - Version: Latest
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
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

#define DEGREES_F  "°F"

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

String wu_url = ""; 

float wu_current_temp = 0.0;
String wu_hi_temp = "";
String wu_lo_temp = "";


// WeMos OLED
// SSD1306_SWITCHCAPVCC 0x2
// SSD1306_I2C_ADDRESS:
// 011110+SA0+RW - 0x3C or 0x3D
// Address for 128x32 is 0x3C
// Address for 128x64 is 0x3D (default) or 0x3C (if SA0 is grounded)
#define OLED_ADDRESS 0x3C
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define OLED_DELAY_MS 30000

// WeMos SHT
#define SHT3X_ADDRESS 0x45
SHT3X sht30(SHT3X_ADDRESS);


// HC-SR501 Motion Detector
//TODO: test HC-SR501 pin after shields mounted
#define IR_PIN D1
int ir_value = 0;


void setup() {
  Serial.begin(115200);

  // wifi setup
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Wifi connected ok. :)");
  Serial.println(WiFi.localIP());


  // init values from file system.
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

      wu_url = Wu_GetUrl();
    }
  }

  
  // TODO: web server
  //wwwServer.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  //wwwServer->on("/", Www_Root);
  //wwwServer->onNotFound(Www_NotFound);
  //wwwServer->begin();
  //Serial.println("HTTP Server Started");


  // OLED setup
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  // IR sensor setup
  pinMode(IR_PIN, INPUT);
}

void loop() {
  // handle web requests
  //wwwServer->handleClient();

  ir_value = digitalRead(IR_PIN);
  Serial.println("IR value: ");
  Serial.println(ir_value);
  if (ir_value == 1) {
    // get temps from wu
    // show temps
  }

  delay(1000);
}


// file io
File GetFile(String fileName) {
  File textFile;
  if (SPIFFS.exists(fileName)) {
    textFile = SPIFFS.open(fileName, "r");
  }
  return textFile;
}


// WU
String Wu_GetUrl() {
  char * str;
  sprintf(str, "%a%b/%c%d/%e%f/%g%h/%i/q/%j.%k", WU_BASE_API, wu_key, WU_LANG, wu_lang, WU_PWS, wu_pws, WU_BESTFCT, wu_bestfct, WU_FEATURES, wu_query, WU_FORMAT);
  return str;
}

void Wu_Refresh() {
  // GET wu_url
  HTTPClient http;
 
  http.begin(wu_url);
  Serial.println(wu_url);
  
  int httpCode = http.GET();
  Serial.println(httpCode);

  if (httpCode > 0) {
    int len = http.getSize();
    // I bet I'm gonna have to stream that json...
    DynamicJsonBuffer jsonBuffer;
    //http.writeToStream(&Serial);
    JsonObject& json = jsonBuffer.parseObject(http.getString());
    // TODO: maybe use openweathermap instead?
    if (json.success()) {
      // WU data values to get:
      // data.current_observation.temp_f (float)
      // data.forecast.simpleforecast.forecastday[0].high.fahrenheit (string)
      // data.forecast.simpleforecast.forecastday[0].low.fahrenheit (string)
      wu_current_temp = json["data"]["current_observation"]["temp_f"];
      wu_hi_temp = json["forecast"]["simpleforecast"]["forecastday"][0]["high"]["fahrenheit"].as<char*>();
      wu_lo_temp = json["forecast"]["simpleforecast"]["forecastday"][0]["low"]["fahrenheit"].as<char*>();
    }
  }

  http.end();
}


// OLED
void Oled_ShowTemps() {
  // Clear the buffer.
  display.clearDisplay();

  if (sht30.get() == 0) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("T: ");
    display.setTextSize(2);
    display.print(sht30.fTemp);
    display.println(DEGREES_F);

    display.setTextSize(1);
    display.println("H: ");
    display.setTextSize(2);
    display.print(sht30.humidity);
    display.println("%");
  } else {
    display.println(":(");
  }

  // show WU temps
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print(wu_hi_temp);
  display.println(DEGREES_F);
  
  display.setTextSize(2);
  display.print(wu_current_temp);
  display.println(DEGREES_F);

  display.setTextSize(1);
  display.print(wu_lo_temp);
  display.println(DEGREES_F);

  display.display();

  delay(OLED_DELAY_MS);
  display.clearDisplay();
}


