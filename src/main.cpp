#include <Arduino.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


//#include <Arduino.h>
#include "functions.h"



void setup() {
  Serial.begin(115200);
  //deleteFile("/data.txt");
  //createFile("/data.txt", false);
  //initFile("/data.txt");
  String result_string = captureSignal();
  Serial.println("As raw Sequence:");
  Serial.println(result_string);
  if (result_string != "No Signal"){
    DynamicJsonDocument testJSON = convertToJSON(result_string, "PLACEHOLDER");
    saveCommand("/data.txt", testJSON);
    delay(1000);
    printFile("/data.txt");
  }
}

void loop() {

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