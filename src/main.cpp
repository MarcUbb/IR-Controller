#include <Arduino.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


//#include <Arduino.h>
#include "functions.h"



void setup() {
  Serial.begin(115200);
  delay(1000);
  listFiles("/data");
  // User choses to create a new sequence
  // 1. filename is generated:
  String filename = "/data/Sequenz1.txt";
  // 2. signal is received
  String result_string = captureSignal();
  // 3. if sinal was received successfully
  if (result_string != "No Signal"){
    // 3.1 signal String is serialized to JSON
    DynamicJsonDocument testJSON = convertToJSON(result_string, "PLACEHOLDER");
    // 3.2 JSON is written to file
    saveCommand(filename, testJSON);
    delay(1000);
    listFiles("/data");
    // 3.3 loads Command and saves it in JSON Object
    DynamicJsonDocument doc = loadCommand(filename);
    // 3.4 (send command)
    sendCommand(doc);
  }
}

void loop() {
  delay(1000);
}
/*
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


ESP8266WebServer server(80);


void setup() {
  Serial.begin(115200);
  Serial.println("ESP Gestartet");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Verbunden! IP-Adresse: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("ESP8266")) {
    Serial.println("DNS gestartet!");
  }

  server.begin();
}

void loop() {
  server.handleClient();
}*/