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
#include <IRremoteESP8266.h>

// IR setup
const int DELAY_BETWEEN_COMMANDS = 1000;
//TODO: use correct IR pin(s):
const int IR_SEND_PIN1 = D1;
const int IR_SEND_PIN2 = D2;
const int IR_SEND_PIN3 = D3;
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
  "  font-family: \"Segoe UI\", Candara, \"Bitstream Vera Sans\", \"DejaVu Sans\", \"Bitstream Vera Sans\", \"Trebuchet MS\", Verdana, \"Verdana Ref\", sans-serif;\n"
  "}\n"
  "\n"
  "button {\n"
  " display: inline-block;\n"
  " border-width: 1px;\n"
  " border-radius: 1em;\n"
  " min-width: 3.5em;\n"
  " margin: 0.1em 0;\n"
  "}\n"
  ".indent { margin-left: 3.5em; }\n"
  ".y { border-color: yellow; }\n"
  ".b { border-color: blue; }\n"
  ".r { border-color: red; }\n"
  ".g { border-color: green; }\n"
  "\n"
  "legend { \n"
  " color: blue; \n"
  " font-weight: bold;\n"
  " transition: color 0.2s ease;\n"
  "}\n"
  ".done {\n"
  " color: black;\n"
  "}\n"
  ".loading {\n"
  " color: yellow;\n"
  "}\n"
  ".error {\n"
  " color: red;\n"
  "}\n"
  ".ok {\n"
  " color: green;\n"
  "}\n"
  "</style>\n";
const char START_FORM[] PROGMEM = "</head>\n"
  "<body>\n"
  "<form action=\"/remote\">\n"
  "<fieldset>\n";
const char BR[] PROGMEM = "<br />\n";
const char END_FORM[] PROGMEM = "</fieldset>\n</form>\n";
const char JS[] PROGMEM = "<script src=\"//code.jquery.com/jquery-3.1.0.js\"></script>\n"
  "<script>\n"
  "$(function jq () {\n"
  " var $form = $(\"form\"),\n"
  "     $legend = $form.find(\"legend\"),\n"
  "     $led = $form.find(\"input[name='led']\"),\n"
  "     $type = $form.find(\"input[name='type']\");\n"
  " \n"
  " $form.on(\"click\", \"button\", function buttonClick (ev) {\n"
  "   $legend.removeClass(\"done\").removeClass(\"error\").removeClass(\"ok\").addClass(\"loading\");\n"
  "   ev.preventDefault();\n"
  "   var $cmd = $(this);\n"
  "   \n"
  "   $.ajax({\n"
  "     url: $form.attr(\"action\"),\n"
  "     data: {\n"
  "       \"led\": $led.val(),\n"
  "       \"type\": $type.val(),\n"
  "       \"cmd\": $cmd.val()\n"
  "     }\n"
  "   }).done(function ajaxDone (/*data, textStatus, jqXHR*/) {\n"
  "     $legend.removeClass(\"loading\").addClass(\"ok\");\n"
  "   }).fail(function(/*jqXHR, textStatus, errorThrown*/) {\n"
  "     $legend.removeClass(\"loading\").addClass(\"error\");\n"
  "   }).always(function(/*data|jqXHR, textStatus, jqXHR|errorThrown*/) { \n"
  "     $legend.addClass(\"done\");\n"
  "   });\n"
  " });\n"
  "});\n"
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
  // is this too large? yes.
  /*
  String page = START_HTML;
  page += CSS;  
  page += START_FORM;
  page += getLegendHtml("TV", "1", "Sony");
  
  // buttons: 
  page += getButtonHtml("0xa90,12,2", "Power", "");
  page += BR;
  
  page += END_FORM;
  page += JS;  
  page += END_HTML;
  
  wwwserver->send(200, "text/html", page);
  */

  // send headers
  wwwserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
  wwwserver->sendHeader("Content-Type", "text/html", true);
  //wwwserver->sendHeader("Cache-Control", "no-cache");
  wwwserver->send(200);

  // send content
  wwwserver->sendContent(START_HTML);
  wwwserver->sendContent(CSS);
  wwwserver->sendContent(START_FORM);
  wwwserver->sendContent(getLegendHtml("TV", "1", "Sony"));

  wwwserver->sendContent(getButtonHtml("0xa90,12,2", "Power", ""));
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
