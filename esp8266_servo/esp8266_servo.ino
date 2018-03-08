#include <FS.h>
// for parsing config: https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>

#include <Servo.h>
#include <ESP8266WiFi.h>

const int LED_PIN = D4;

// config file:
#define CONFIG_JSON  "/config.json"

// Servo Params:
const int SERVO_PIN = D2;
const int SERVO_MINPULSE = 500;
const int SERVO_MAXPULSE = 2400;
Servo servo_feeder;

// Feeder params
const int SERVO_FEEDSTARTANGLE = 105;
const int SERVO_FEEDENDANGLE = 0;
const int SERVO_FEEDDELAYMS = 1000;

const int SERVO_MOVE_DELAY = 5;  // ms
const int SERVO_OFF_DELAY = 500;  // ms


int servo_last_angle = SERVO_FEEDSTARTANGLE;
//  persist last angle into file
#define SERVO_LAST_ANGLE_FILE  "/servo_last_angle.txt"

// Wifi Params (overridden with values in data/config.json):
String wifi_ssid = "*****";
String wifi_password = "*****";

// Web server on port 80
WiFiServer www_server(80);

void setup() {
  // turn off built-in LED? off by default on WeMos D1 mini
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);

  Config_Init();
  Wifi_Init();
  Www_Init();
  Servo_Init();
}

void loop() {
  Www_Listen();
}

void App_Feed() {
  pinMode(SERVO_PIN, OUTPUT);
  Servo_Rotate(SERVO_FEEDENDANGLE);
  delay(SERVO_FEEDDELAYMS);
  Servo_Rotate(SERVO_FEEDSTARTANGLE);
  pinMode(SERVO_PIN, INPUT);
}

// config file io
File Config_GetFile(String fileName) {
  File textFile;
  if (SPIFFS.exists(fileName)) {
    Serial.println("Opening file");
    textFile = SPIFFS.open(fileName, "r");
  }
  return textFile;
}

String Config_ReadFile(String path) {
  Serial.print("Reading file: ");
  Serial.println(path);

  File file = SPIFFS.open(path, "r");
  if (!file) {  // || file.isDirectory()
    Serial.println("Failed to open file for reading");
  }

  Serial.print("Read from file: ");
  String text = "";
  text = file.readStringUntil('\n');
  file.close();
  Serial.println(text);
  return text;
}

void Config_WriteFile(const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void Config_Init() {
  Serial.println("starting SPIFFS");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    // parse json config file
    File jsonFile = Config_GetFile(CONFIG_JSON);
    if (jsonFile) {
      Serial.println("JSON open");
      size_t size = jsonFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> jsonBuf(new char[size]);
      jsonFile.readBytes(jsonBuf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(jsonBuf.get());
      if (json.success()) {
        // init wifi settings
        wifi_ssid = json.get<String>("wifi_ssid");
        Serial.println(wifi_ssid);
        wifi_password = json.get<String>("wifi_password");
        Serial.println(wifi_password);
      } else {
        Serial.println("failed to load json config");
      }
      jsonFile.close();
    }

    // read value from last angle
    String content = Config_ReadFile(SERVO_LAST_ANGLE_FILE);
    servo_last_angle = content.toInt();
    Serial.println(servo_last_angle);
  }
}

// Web server functionality:
void Www_Init() {
  Serial.println("starting web server");
  www_server.begin();
}

void Www_Listen() {
  WiFiClient www_client = www_server.available();
  if (www_client) {
    Serial.println("WWW client");
    while (www_client.connected()) {
      if (www_client.available()) {
        Serial.println("WWW client available");

        String line = www_client.readStringUntil('\n');
        Serial.println(line);

        if (line.startsWith("GET /feed")) {
          Serial.println("Feed!");
          App_Feed();

          www_client.println("HTTP/1.1 200 OK");
          www_client.println("Content-Type: text/html");
          www_client.println("Connection: close");
          www_client.println();
          www_client.println("<!DOCTYPE HTML><html><head><title>ok</title></head><body><h1>OK</h1></body></html>");
          www_client.println();
          delay(200);
        } else {
          // ignore everything else, usually /favicon.ico
          Serial.println("Ignoring request.");
        }
        // exit loop after first line.
        break;
      }
    }
    delay(10);
    // close the connection:
    www_client.stop();
  }
}


// WiFi Functionality:
void Wifi_Init() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifi_ssid.c_str(), wifi_password.c_str());
  delay(500);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}


// Servo functionality:
void Servo_Init() {
  Serial.println("initializing servo");
  servo_feeder.attach(SERVO_PIN, SERVO_MINPULSE, SERVO_MAXPULSE);
}

void Servo_Rotate(int angle) {
  // set angle direcly
  pinMode(SERVO_PIN, OUTPUT);

  //servo_feeder.write(angle);
  int pos = 0;
  if (angle > servo_last_angle) {
    for (pos = servo_last_angle; pos <= angle; pos++) {
      servo_feeder.write(pos);
      delay(SERVO_MOVE_DELAY);
    }
  } else if (angle < servo_last_angle) {
    for (pos = servo_last_angle; pos >= angle; pos--) {
      servo_feeder.write(pos);
      delay(SERVO_MOVE_DELAY);
    }
  }
  
  delay(SERVO_OFF_DELAY);
  pinMode(SERVO_PIN, INPUT);

  servo_last_angle = angle;

  // store last angle in firmware:
  if (SPIFFS.begin()) {
    char buf[4];
    itoa(servo_last_angle, buf, 10);
    Config_WriteFile(SERVO_LAST_ANGLE_FILE, buf);
  }
}

