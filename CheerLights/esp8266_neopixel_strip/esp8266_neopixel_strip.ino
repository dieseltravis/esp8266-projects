/*
 Set Latest CheerLights Color on a WS2812 NeoPixel Strip via ESP8266 Wi-Fi

 This sketch periodically checks the CheerLights color that is stored in
 ThingSpeak channel 1417 and sets the color of a WS2812-based NeoPixel strip.

 Requirements:

   * ESP8266 Wi-Fi Device
   * Arduino 1.8.8+ IDE
   * Additional Boards URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Library: ThingSpeak by MathWorks
   * Library: Adafruit NeoPixel by Adafruit

 Setup Wi-Fi:
  * Enter SECRET_SSID in "secrets.h"
  * Enter SECRET_PASS in "secrets.h"

 CheerLights Project: http://cheerlights.com

 Created: Dec 19, 2018 by Hans Scharler (http://nothans.com)
 Remix with soft animations by Travis Hardiman
*/

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ThingSpeak.h>
#include "secrets.h"

unsigned long cheerLightsChannelNumber = 1417;

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  client;

// https://www.wemos.cc/en/latest/d1/d1_mini_pro.html
// D2  IO, SDA   GPIO4
#define RGBPIN         4     // Enter the NeoPixel strip pin number
#define NUMPIXELS      19    // Enter how many NeoPixels are connected on the strip

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RGBPIN, NEO_GRB + NEO_KHZ800); // Set up the NeoPixel library
// max brightness to use
const uint8_t MAX_BRIGHT = 51;

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");

  Serial.print("ESP Reset Reason: ");
  Serial.println(ESP.getResetReason());
  Serial.print("ESP Core Version: ");
  Serial.println(ESP.getCoreVersion());
  Serial.print("ESP SDK Version: ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("ESP Flash ID: ");
  Serial.println(ESP.getFlashChipId());
  uint8_t id[8];
  // esp8266
  uint32_t chipid = ESP.getChipId();
  id[0] = 0;
  id[1] = 0;
  id[2] = 0;
  id[3] = 0;
  id[4] = chipid >> 24;
  id[5] = chipid >> 16;
  id[6] = chipid >> 8;
  id[7] = chipid;
  // esp 32
  /*
  uint64_t chipid = ESP.getEfuseMac();
  id[0] = 0;
  id[1] = 0;
  id[2] = chipid;
  id[3] = chipid >> 8;
  id[4] = chipid >> 16;
  id[5] = chipid >> 24;
  id[6] = chipid >> 32;
  id[7] = chipid >> 40;
  */
  Serial.print("ESP Chip ID: ");
  Serial.println(chipid);
  Serial.print(id[4]);
  Serial.print(":");
  Serial.print(id[5]);
  Serial.print(":");
  Serial.print(id[6]);
  Serial.print(":");
  Serial.print(id[7]);
  Serial.println("");
  char sssid[12];
  snprintf(sssid, 12, "ESP-%06X", chipid);
  Serial.println(sssid);
  Serial.println("\n");
    
  Serial.print("LED_BUILTIN: ");
  Serial.println(LED_BUILTIN);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.print("RGBPIN: ");
  Serial.println(RGBPIN);
  
  pixels.begin();
  pixels.setBrightness(0);
  pixels.clear();

  // Connect or reconnect to WiFi
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SECRET_SSID);
  Serial.print("ESP MAC address: ");
  Serial.println(WiFi.macAddress());
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("\nConnected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client);
  pixels.setBrightness(MAX_BRIGHT); // set brightness of the strip
}

// Define the supported CheerLights colors and their RGB values
String colorName[] = {"red", "pink", "green", "blue", "cyan", "white", "warmwhite", "oldlace", "purple", "magenta", "yellow", "orange"};

int colorRGB[][3] = { 255,   0,   0,  // "red"
                      255, 192, 203,  // "pink"
                        0, 255,   0,  // "green"
                        0,   0, 255,  // "blue"
                        0, 255, 255,  // "cyan"
                      255, 255, 255,  // "white"
                      255, 223, 223,  // "warmwhite"
                      255, 223, 223,  // "oldlace"
                      128,   0, 128,  // "purple"
                      255,   0, 255,  // "magenta"
                      255, 255,   0,  // "yellow"
                      255, 165,   0}; // "orange"
String color;
String lastColor = "black";

void loop() {

  int statusCode = 0;
  
  // Read CheerLights color from ThingSpeak channel
  color = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();

  if (statusCode == 200) {
    Serial.println("CheerLights Color: " + color);
    if (lastColor != color) {
      Serial.println("changing from " + lastColor + " to " + color);
      setColor(color);
      lastColor = color;
    }
  }
  else {
    Serial.println("Problem reading channel. HTTP error code: " + String(statusCode));
    // todo: error blink
  }

  // Wait 20 seconds before checking again
  // regular delay:
  //delay(20000);
  // breathe a bit instead:
  int last_bright = MAX_BRIGHT;
  for (int j = 0; j < 5; j++) {
    Serial.print('.');
    for (int i = 0; i < 20; i++) {
      last_bright -= 1;
      pixels.setBrightness(last_bright);
      pixels.show();
      delay(100);
    }
    Serial.print('.');
    for (int i = 0; i < 20; i++) {
      last_bright += 1;
      pixels.setBrightness(last_bright);
      pixels.show();
      delay(100);
    }
  }
  Serial.println('.');
}

int r, g, b, lr = 0, lg = 0, lb = 0;

void setColor(String color) {
  // Loop through the list of colors to find the matching color
  for (int colorIndex = 0; colorIndex < 12; colorIndex++) {
    if (color == colorName[colorIndex]) {
      // Set the color of each NeoPixel on the strip
      r = colorRGB[colorIndex][0];
      g = colorRGB[colorIndex][1];
      b = colorRGB[colorIndex][2];

      updateRGB();
    }
  }
}

void updateRGB() {
  // delta values
  int dr = (r == lr) ? 0 : (r - lr > 0) ? 1 : -1;
  int dg = (g == lg) ? 0 : (g - lg > 0) ? 1 : -1;
  int db = (b == lb) ? 0 : (b - lb > 0) ? 1 : -1;

  // current values
  int cr = lr;
  int cg = lg;
  int cb = lb;

  // loop until all equal
  while (cr != r || cg != g || cb != b) {
    for (int pixel = 0; pixel < NUMPIXELS; pixel++) {
      pixels.setPixelColor(pixel, pixels.Color(cr, cg, cb));
      pixels.show();
      delay(10);
    }
    if (cr != r) cr += dr;
    if (cg != g) cg += dg;
    if (cb != b) cb += db;
  }
  
  lr = r;
  lg = g;
  lb = b;
}
