#include <FS.h> //this needs to be first
// Arduino Servo
#include <Servo.h>
// ESP8266WiFi - Version: Latest 
#include <ESP8266WiFi.h>
// DNSServer - Version: Latest 
#include <DNSServer.h>
// ESP8266WebServer - Version: Latest 
#include <ESP8266WebServer.h>
// WiFiManager - Version: Latest
#include <WiFiManager.h>
// MQTT
#include <PubSubClient.h>
// JSON parsing for Arduino
#include <ArduinoJson.h>


// web server
std::unique_ptr<ESP8266WebServer> wwwServer;
#define HTML_FILE "/index.html"


// MQTT pub/sub for the servo functions
#define CLOUDMQTT_CONFIG  "/config.json"
// these values are loaded from a config.json file in the data directory
char cloudmqtt_server[18] = "***.cloudmqtt.com";
int  cloudmqtt_port       = 0;
char cloudmqtt_user[9]    = "********";
char cloudmqtt_pass[13]   = "************";

// note: if you have multiple devices, assign them different ID's
#define MQTT_CLIENT_ID   "WeMosEsp8266"
// set to true if you want last msg for all topics retained on server, so you get it automatically on client connect
#define MQTT_USE_RETAIN   false

#define TOPIC_OUT_CONN      "esp/feeder/conn"
#define TOPIC_IN_SYNC       "esp/feeder/sync"
#define TOPIC_OUT_FEEDING   "esp/feeder/feeding"
#define TOPIC_IN_FEED       "esp/feeder/feed"

// publish data periodically every 10 minutes
#define PUB_PERIODIC_MS     1000 * 60 * 10
// don't publish more often that this, even on status/feeding change
#define PUB_MIN_MS          1000

char mqtt_msg[50];
uint16_t connect_cnt = 0;
uint16_t sync_cnt = 0;

uint8_t last_read_status = 255;
uint8_t last_published_status = 255;
long last_status_unstable_ms = 0;

long last_published_ms = 0;
bool isSyncing = false;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// servo
const int SERVO_CONTROL = D2;
const int SERVO_MOVE_DELAY = 15;  // ms
const int SERVO_OFF_DELAY = 500;  // ms
bool isFeeding = false;
Servo feederServo;


void setup() {
  Serial.begin(115200);

  // init values from file system
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    // parse json config file
    File jsonFile = GetFile(CLOUDMQTT_CONFIG);
    if (jsonFile) {
      size_t size = jsonFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> jsonBuf(new char[size]);
      jsonFile.readBytes(jsonBuf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(jsonBuf.get());
      if (json.success()) {
        strcpy(cloudmqtt_server, json["cloudmqtt_server"]);
        cloudmqtt_port = json["cloudmqtt_port"];
        strcpy(cloudmqtt_user, json["cloudmqtt_user"]);
        strcpy(cloudmqtt_pass, json["cloudmqtt_pass"]);
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
  wwwServer.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  wwwServer->on("/", Www_Root);
  wwwServer->on("/feed", Www_Feed);
  wwwServer->onNotFound(Www_NotFound);
  wwwServer->begin();
  Serial.println("HTTP Server Started");

  feederServo.attach(SERVO_CONTROL);
  pinMode(SERVO_CONTROL, OUTPUT);
  feederServo.write(0);
  delay(SERVO_OFF_DELAY);
  pinMode(SERVO_CONTROL, INPUT);

  // setup MQTT
  mqttClient.setServer(cloudmqtt_server, cloudmqtt_port);
  mqttClient.setCallback(Subscription_Callback);
}

void loop() {
  // handle web requests
  wwwServer->handleClient();

  // handle MQTT connection
  if (!mqttClient.connected()) {
    connect_cnt++;
    Mqtt_Reconnect();  
    last_published_ms = last_status_unstable_ms = millis(); // init time here, it takes a while to connect
  }
  mqttClient.loop();
  long now_ms = millis();

  // limit minimal MQTT sending interval, publish data
  if ((now_ms - last_published_ms) > PUB_MIN_MS) {
    // check if we we're syncing data
    if (isSyncing) {
      char tmp_str[20];

      // increment counter and publish status
      sync_cnt++;
      snprintf (tmp_str, sizeof (tmp_str), "Synced(%d)", sync_cnt);
      Publish_Connection (tmp_str);
      
      // sync complete at this point, everything that needed to be sent is above
      isSyncing = false;
    }
    // send periodically
    else if ((now_ms - last_published_ms) > PUB_PERIODIC_MS) {
      last_published_ms = now_ms;
    }
  }
}


// file io
File GetFile(String fileName) {
  File textFile;
  if (SPIFFS.exists(fileName)) {
    textFile = SPIFFS.open(fileName, "r");
  }
  return textFile;
}

// servo methods
void Servo_Move(int newPos) {
  feederServo.write(newPos);
  delay(SERVO_MOVE_DELAY);
}

void Servo_OpenClose() {
  isFeeding = true;
  Publish_Feeding();
  
  int pos = 0;
  // goes from 0 degrees to 90 degrees to 0 degrees
  Serial.println("Servo opening...");
  pinMode(SERVO_CONTROL, OUTPUT);
  for (pos = 0; pos <= 90; pos++) {
    Servo_Move(pos);
  }
  Serial.println("Servo closing...");
  for (pos = 90; pos >= 0; pos--) {
    Servo_Move(pos);
  }
  delay(SERVO_OFF_DELAY);
  pinMode(SERVO_CONTROL, INPUT);
  Serial.println("Servo closed.");
  
  isFeeding = false;
  Publish_Feeding();
}


// web server methods:
void Www_Root() {
  File htmlFile = GetFile(HTML_FILE);
  wwwServer->streamFile(htmlFile, "text/html");
  htmlFile.close();
} 

void Www_Feed() {
  Servo_OpenClose();
  wwwServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  wwwServer->sendHeader("Pragma", "no-cache");
  wwwServer->sendHeader("Expires", "-1");
  wwwServer->send(200, "text/plain", "OK.\n\n");
}

void Www_NotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wwwServer->uri();
  message += "\nMethod: ";
  message += (wwwServer->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += wwwServer->args();
  message += "\n";
  //for (uint8_t i=0; i<wwwServer->args(); i++){
  //  message += " " + wwwServer->argName(i) + ": " + wwwServer->arg(i) + "\n";
  //}
  wwwServer->send(404, "text/plain", message);
}


// MQTT functions
void Mqtt_Reconnect() {
  uint32_t millis_start = millis();
  
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, cloudmqtt_user, cloudmqtt_pass)) {
      Serial.println("Connected.");
      
      char *OK = new char[3];
      strcpy( OK, "OK." );
      Publish_Connection(OK);
      
      mqttClient.subscribe(TOPIC_IN_FEED);
      mqttClient.subscribe(TOPIC_IN_SYNC);
      break;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    
    // restart ESP if we cannot connect for too long
    if ((millis () - millis_start) > 2 * 60000) {
      Serial.println ("Cannot connect to MQTT, restarting...");  
      ESP.restart ();
    }
  }
}

void Publish_Connection(char * text) {
  // report to terminal for debug
  snprintf(mqtt_msg, sizeof (mqtt_msg), "Publishing %s %s", TOPIC_OUT_CONN, text);
  Serial.println(mqtt_msg);
  
  // publish to MQTT server
  mqttClient.publish(TOPIC_OUT_CONN, text, MQTT_USE_RETAIN);   
}

void Publish_Feeding() {
  mqttClient.publish(TOPIC_OUT_FEEDING, String(isFeeding).c_str(), MQTT_USE_RETAIN); 
}

void Subscription_Callback(char* topic, byte* payload, unsigned int length) {
  // report to terminal for debug
  Serial.print("MQTT msg arrived: ");
  Serial.print(topic);
  Serial.print(" ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // check if we received our IN topic for state change
  if (strcmp (TOPIC_IN_FEED, topic) == 0) {
    // toggle output if we need to change state
    if ((char)payload[0] == '1' && !isFeeding) {
      Serial.print ("Toggling state!");
      Servo_OpenClose();
    }
  }
  // check for sync topic
  else if (strcmp (TOPIC_IN_SYNC, topic) == 0) {
    // we should get '1' as sync request
    if ((char) payload[0] == '1') {
      isSyncing = true;
    }
  }
}
