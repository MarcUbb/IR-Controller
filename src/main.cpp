#include <Arduino.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "functions.h"
/*
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>IR Remote</title>
  <style>
    html {font-family: Arial; display: inline-block;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;} 
  </style>
</head>
<body>
  <h2>IR-Remote</h2>
  <form action="/get">
    <select id="sequences" name="sequences"></select>
    <input type="submit" value="send">
  </form>

  <form action="/get">
    <label for="seqName">Sequence name:</label><br>
    <input type="text" id="seqName" name="seqName" placeholder="sequence name"><br>
    <input type="submit" value="+">
  </form>

  <form action="/get">
    <select name="programs" id="programs"></select>
    <input type="submit" value="activate">
  </form>

  <form action="/get">
    <label for="progCode">Program code:</label><br>
    <input type="text" id="progCode" name="progCode" placeholder="write your code here"><br>
    <label for="progName">Program name:</label><br>
    <input type="text" id="progName" name="progName" placeholder="program name"><br>
    <input type="submit" value="+">
  </form>
</body>
</html>)rawliteral";


ESP8266WebServer server(80);

void handleRoot() {
 server.send(200, "text/html", index_html); //Send web page
}

void handleForm() {
  String seqName = server.arg("seqName"); 
  String progCode = server.arg("progCode");
  String progName = server.arg("progName"); 

  Serial.println("seqName: " + seqName);
  Serial.println("progCode: " + progCode);
  Serial.println("progName: " + progName);

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //ESP.eraseConfig();
  //wifiManager.resetSettings();
  
  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("IR-Remote");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  
  if (MDNS.begin("IRR")) { 
    Serial.println("MDNS responder started"); 
  }

  server.on("/", handleRoot);
  server.on("/get", handleForm);

  server.begin();
}

void loop() {
  MDNS.update();
  server.handleClient();
}
*/



void setup() {
  Serial.begin(115200);
  delay(3000);
  checkFiles("/data");

  boolean error_recording = false;
  boolean error_sending = false;
  boolean error_deleting = false;

  error_recording = recording_workflow("Sequenz 2");
  Serial.println(error_recording);
  delay(1000);
  checkFiles("/data");

  if(error_recording) {
    error_sending = sending_workflow("Sequenz 2");
    Serial.println(error_sending);
    delay(1000);
    checkFiles("/data");
  }

  if(error_sending) {
    error_deleting = deleting_workflow("Sequenz 2");
    Serial.println(error_deleting);
    delay(1000);
    checkFiles("/data");
  }
}

void loop() {
  yield();
}

