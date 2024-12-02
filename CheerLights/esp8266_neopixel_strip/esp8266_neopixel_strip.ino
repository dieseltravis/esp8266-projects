/*
 Set Latest CheerLights Color on a WS2812 NeoPixel Strip via ESP8266 Wi-Fi
 Requirements:

   * ESP8266 Wi-Fi Device
   * Arduino 1.8.8+ IDE
   * Additional Boards URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Library: Adafruit NeoPixel by Adafruit

 Setup Wi-Fi:
  * Enter SECRET_SSID in "secrets.h"
  * Enter SECRET_PASS in "secrets.h"

 CheerLights Project: http://cheerlights.com

 Created: Dec 19, 2018 by Hans Scharler (http://nothans.com)
 Remix with soft animations by Travis Hardiman
 Updated to use Cheerlights library and don't adjust brightness when purple, because before adjusting brightness when purple would cause the LEDs to just turn off
*/

#include <Adafruit_NeoPixel.h>

// Include the correct WiFi library based on the board
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #include <WiFi.h>
#endif

// Include the CheerLights library and instantiate the CheerLights object
#include <CheerLights.h>
CheerLights CheerLights;

#include "secrets.h"

// https://www.wemos.cc/en/latest/d1/d1_mini_pro.html
// D2  IO, SDA   GPIO4
#define RGBPIN         4     // Enter the NeoPixel strip pin number
#define NUMPIXELS      19    // Enter how many NeoPixels are connected on the strip

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RGBPIN, NEO_GRB + NEO_KHZ800); // Set up the NeoPixel library
// max brightness to use
const uint8_t MAX_BRIGHT = 51;

String color;
String lastColor = "black";
uint8_t r, g, b, lr = 0, lg = 0, lb = 0;

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
#if defined(ESP8266)
  uint32_t chipid = ESP.getChipId();
  id[0] = 0;
  id[1] = 0;
  id[2] = 0;
  id[3] = 0;
  id[4] = chipid >> 24;
  id[5] = chipid >> 16;
  id[6] = chipid >> 8;
  id[7] = chipid;
#elif defined(ESP32)
  uint64_t chipid = ESP.getEfuseMac();
  id[0] = 0;
  id[1] = 0;
  id[2] = chipid;
  id[3] = chipid >> 8;
  id[4] = chipid >> 16;
  id[5] = chipid >> 24;
  id[6] = chipid >> 32;
  id[7] = chipid >> 40;
#endif
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

  // Initialize the CheerLights library
  CheerLights.begin(SECRET_SSID, SECRET_PASS);
    
  Serial.println("\nConnected!");

  pixels.setBrightness(MAX_BRIGHT); // set brightness of the strip

  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  CheerLights.getCurrentColor();
  Serial.print("Current CheerLights Color: ");
  color = CheerLights.currentColorName();
  Serial.println(color);
        
  if (lastColor != color) {
    Serial.println("changing from " + lastColor + " to " + color);
    setColors(CheerLights.currentRed(), CheerLights.currentGreen(), CheerLights.currentBlue());
    lastColor = color;
  }

	bool isPurple = (r == 128 && g == 0 && b == 128);
	// TODO: use ColorHSV(h, s, v) and modulate V instead of setBrightness
	// pixels.fill(pixels.Color(r, g, b), 0, NUMPIXELS);
  // breathe brightness a bit:
  int last_bright = MAX_BRIGHT;
  for (int j = 0; j < 5; j++) {
    Serial.print('.');
    for (int i = 0; i < 20; i++) {
      last_bright -= 1;
      // only when not purple
      if (!isPurple) {
        pixels.setBrightness(last_bright);
        pixels.show();
      }
      delay(100);
    }
    Serial.print('.');
    for (int i = 0; i < 20; i++) {
      last_bright += 1;
      // only when not purple
      if (!isPurple) {
        pixels.setBrightness(last_bright);
        pixels.show();
      }
      delay(100);
    }
  }
  Serial.println('.');
}

void setColors(uint8_t red, uint8_t green, uint8_t blue) {
  // Set the color of each NeoPixel on the strip
  r = red;
  g = green;
  b = blue;

  Serial.print("Setting RGB to: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.print(b);
  Serial.println(".");
  
  updateRGB();
}

void updateRGB() {
  // delta values
  uint8_t dr = (r == lr) ? 0 : (r - lr > 0) ? 1 : -1;
  uint8_t dg = (g == lg) ? 0 : (g - lg > 0) ? 1 : -1;
  uint8_t db = (b == lb) ? 0 : (b - lb > 0) ? 1 : -1;

  // current values start at last values
  uint8_t cr = lr;
  uint8_t cg = lg;
  uint8_t cb = lb;

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
