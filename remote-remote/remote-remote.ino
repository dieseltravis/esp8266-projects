// d1_mini - Version: Latest 
//#include <pins_arduino.h>

//#include <stdlib.h>

// ESP8266WiFi - Version: Latest 
#include <ESP8266WiFi.h>
// DNSServer - Version: Latest 
#include <DNSServer.h>
// ESP8266WebServer - Version: Latest 
#include <ESP8266WebServer.h>

// WiFiManager - Version: 0.12.0
#include <WiFiManager.h>

// IRremoteESP8266-1.2.0 - Version: Latest 
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRutils.h>
#include <ir_Daikin.h>
#include <ir_Kelvinator.h>
#include <ir_LG.h>
#include <ir_Mitsubishi.h>

// IR setup
const int DELAY_BETWEEN_COMMANDS = 1000;
//TODO: use correct IR pin(s):
const int IR_SEND_PIN1 = D1;
const int IR_SEND_PIN2 = D2;
const int IR_SEND_PIN3 = D8;
IRsend irsend1(IR_SEND_PIN1);
IRsend irsend2(IR_SEND_PIN2);
IRsend irsend3(IR_SEND_PIN3);
const int led = BUILTIN_LED;

// web server
std::unique_ptr<ESP8266WebServer> wwwserver;

const char START_HTML[] PROGMEM = "<!doctype html>\n"
  "<html class=\"no-js\" lang=\"en-us\">\n"
  "<head>\n"
  "<meta charset=\"utf-8\" />\n"
  "<meta http-equiv=\"x-ua-compatible\" content=\"ie=edge\" />\n"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
  "<link rel=\"icon\" href=\"data:image/png;base64,"
  "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAS0lEQVR42s2SMQ4AIAjE+P+ncSYdasgNXMJgcyIIlVKPIKdvioAXyWBeJmVpqRZKWtj9QWAKZyWll50b8IcL9JUeQF50n28ckyb0ADG8RLwp05YBAAAAAElFTkSuQmCC"
  "\" type=\"image/x-png\" />\n"
  "<title>Remote²</title>\n";
const char CSS[] PROGMEM = "<style>\n"
  "body {\n"
	"	background: #EEE;\n"
	"	color: #333;\n"
	"	font-family: \"Segoe UI\", Candara, \"Bitstream Vera Sans\", \"DejaVu Sans\", \"Bitstream Vera Sans\", \"Trebuchet MS\", Verdana, \"Verdana Ref\", sans-serif;\n"
	"	font-size: 24px;\n"
	"}\n"
	"\n"
	"fieldset {\n"
	"	background: #DDD;\n"
	"	border: solid 2px #666;\n"
	"	border-radius: 0.5em;\n"
	"}\n"
	"\n"
	"legend { \n"
	"	color: blue; \n"
	"	font-weight: bold;\n"
	"	transition: color 0.2s ease;\n"
	"	font-size: 1.0em;\n"
	"}\n"
	"\n"
	"button {\n"
	"	display: inline-block;\n"
	"	border-width: 1px;\n"
	"	border-radius: 1em;\n"
	"	min-width: 3.5em;\n"
	"	margin: 0.2em 0;\n"
	"	background: #FFF;\n"
	"	font-size: 1.0em;\n"
	"}\n"
	"button:hover {\n"
	"	background-color: #EFE;\n"
	"}\n"
	"button:focus {\n"
	"	outline: none;\n"
	"	box-shadow: 0 0 2pt 2pt #CDC;\n"
	"}\n"
	"\n"
	".indent { margin-left: 3.5em; }\n"
	".y { border-color: yellow; }\n"
	".b { border-color: blue; }\n"
	".r { border-color: red; }\n"
	".g { border-color: green; }\n"
	".done {\n"
	"	color: black;\n"
	"}\n"
	".loading {\n"
	"	color: yellow;\n"
	"}\n"
	".error {\n"
	"	color: red;\n"
	"}\n"
	".ok {\n"
	"	color: green;\n"
	"}\n"
  "</style>\n";
const char START_BODY[] PROGMEM = "</head>\n<body>\n";
const char START_FORM[] PROGMEM = "<form action=\"/remote\">\n<fieldset>\n";
const char BR[] PROGMEM = "<br />\n";
const char HR[] PROGMEM = "<hr />\n";
const char END_FORM[] PROGMEM = "</fieldset>\n</form>\n";
const char JS[] PROGMEM = "<script>\n"
  "(function iife(w) {\n"
	"	\"use strict\";\n"
	"\n"
	"	var d = w.document,\n"
	"			form = d.querySelector(\"form\"),\n"
	"			legend = d.querySelector(\"legend\"),\n"
	"			led = d.querySelector(\"input[name='led']\"),\n"
	"			type = d.querySelector(\"input[name='type']\"),\n"
	"			buttons = d.querySelectorAll(\"button\");\n"
	"\n"
	"	buttons.forEach(function eachButton(button) {\n"
	"		button.addEventListener(\"click\", function formSubmit(ev) {\n"
	"			ev.preventDefault();\n"
	"			legend.className = \"loading\";\n"
	"\n"
	"			w.fetch(form.action + \"?led=\" + led.value + \"&type=\" + type.value + \"&cmd=\" + button.value, {\n"
	"				method: \"GET\"\n"
	"			}).then(function fetchForm(response) {\n"
	"				if (response.ok) {\n"
	"					w.console.info(\"ok\", response.text());\n"
	"					legend.className = \"ok\";\n"
	"				} else {\n"
	"					w.console.info(\"not ok\", response);\n"
	"					legend.className = \"error\";\n"
	"				}\n"
	"			}).then(function fetchThen() {\n"
	"				legend.className += \" done\";\n"
	"			}).catch(function fetchError(err) {\n"
	"				w.console.info(\"error\", err);\n"
	"				legend.className = \"error\";\n"
	"			});\n"
	"		});\n"
	"	});\n"
	"\n"
	"}(this));\n"
  "</script>\n";
const char END_HTML[] PROGMEM = "</body>\n</html>\n";

String getLegendHtml(String title, String irLed, String irType) {
  return "<legend>" + title + "</legend>\n" +
    "<input name=\"led\" type=\"hidden\" value=\"" + irLed + "\" />\n" +
    "<input name=\"type\" type=\"hidden\" value=\"" + irType + "\" />\n";
}

String getButtonHtml(String value, String text, String cssClasses) {
  return "<button name=\"cmd""\" value=\"" + value + "\" class=\"" + cssClasses + "\">" + text + "</button>";
}

void handleRoot() {
  digitalWrite(led, 0);

  // send headers
  wwwserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
  wwwserver->sendHeader("Content-Type", "text/html", true);
  //wwwserver->sendHeader("Cache-Control", "no-cache");
  wwwserver->send(200);

  // send content
  wwwserver->sendContent(START_HTML);
  wwwserver->sendContent(CSS);
	wwwserver->sendContent(START_BODY);
	
	// Sony TV
  wwwserver->sendContent(START_FORM);
  wwwserver->sendContent(getLegendHtml("TV", "1", "Sony"));

  wwwserver->sendContent(getButtonHtml("0xa90,12,2", "Power", ""));
  wwwserver->sendContent(BR);

  wwwserver->sendContent(END_FORM);
	
	// Test 1
  wwwserver->sendContent(START_FORM);
  wwwserver->sendContent(getLegendHtml("Test", "2", "Sony"));

  wwwserver->sendContent(getButtonHtml("0xa90,12,2", "Test", ""));
  wwwserver->sendContent(BR);

  wwwserver->sendContent(END_FORM);
	
	// Test 2
  wwwserver->sendContent(START_FORM);
  wwwserver->sendContent(getLegendHtml("Test 2", "3", "Sony"));

  wwwserver->sendContent(getButtonHtml("0xa90,12,2", "Test 2", ""));
  wwwserver->sendContent(BR);

  wwwserver->sendContent(END_FORM);
	
  wwwserver->sendContent(JS);
  wwwserver->sendContent(END_HTML);

  digitalWrite(led, 1);
}

void sendRemoteCommands(IRsend &irsend, String typeParam, unsigned long data, int nbits, unsigned int repeat) {
  // send code to IR LED
  if (typeParam == "Sony") {
    irsend.sendSony(data, nbits, repeat);
  } else if (typeParam == "NEC") {
    irsend.sendNEC(data, nbits, repeat);
  }
}

void handleRemote(){
  digitalWrite(led, 1);

  // parse params and send IR codes
  String ledParam = wwwserver->arg("led");
  String typeParam = wwwserver->arg("type");
  String cmdParam = wwwserver->arg("cmd");

  unsigned long data;
  int nbits;
  unsigned int repeat = 0;
  
  // parse cmd param
  int firstComma = cmdParam.indexOf(',');
  int secondComma = cmdParam.indexOf(',', firstComma + 1);
  String firstData = cmdParam.substring(0, firstComma);
  String secondData = "";
  String thirdData = "";
  if (secondComma > -1) {
    secondData = cmdParam.substring(firstComma + 1, secondComma);
    thirdData = cmdParam.substring(secondComma + 1);
  } else {
    secondData = cmdParam.substring(firstComma + 1);
  }
  
  // data param is converted from hex to long
  const char * c = firstData.c_str();
  // or 
  //char c[firstData.length() + 1];
  //firstData.toCharArray(c, sizeof(c));
  data = strtol(c, 0, 16);
  
  // remaining params are simple ints
  nbits = secondData.toInt();
  if (thirdData.length() > 0) {
    repeat = thirdData.toInt();
  }
  
  // select which IR LED to send to
  if (ledParam == "1") {
    sendRemoteCommands(irsend1, typeParam, data, nbits, repeat);
  } else if (ledParam == "2") {
    sendRemoteCommands(irsend2, typeParam, data, nbits, repeat);
  } else if (ledParam == "3") {
    sendRemoteCommands(irsend3, typeParam, data, nbits, repeat);
  }

  wwwserver->send(200, "text/plain", "OK.\n\n");
  digitalWrite(led, 1);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wwwserver->uri();
  message += "\nMethod: ";
  message += (wwwserver->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += wwwserver->args();
  message += "\n";
  for (uint8_t i=0; i<wwwserver->args(); i++){
    message += " " + wwwserver->argName(i) + ": " + wwwserver->arg(i) + "\n";
  }
  wwwserver->send(404, "text/plain", message);
  digitalWrite(led, 1);
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  // wifi setup
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Wifi connected ok. :)");
  Serial.println(WiFi.localIP());
  
  // ir setup
  irsend1.begin();
  irsend2.begin();
  irsend3.begin();
  Serial.println("IR setup.");
 
  // web server
  wwwserver.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  wwwserver->on("/", handleRoot);
  wwwserver->on("/remote", handleRemote);
  wwwserver->onNotFound(handleNotFound);
  wwwserver->begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  wwwserver->handleClient();
}
