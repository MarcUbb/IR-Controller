#include <Arduino.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include "functions.h"


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void loop() {
    // put your main code here, to run repeatedly:
    
}



/*
void setup() {
  Serial.begin(115200);
  delay(3000);
  checkFiles("/data");

  boolean error_recording = false;
  boolean error_sending = false;
  boolean error_deleting = false;

  error_recording = recording_workflow("Sequenz2");
  Serial.println(error_recording);
  delay(1000);
  checkFiles("/data");

  if(error_recording) {
    error_sending = sending_workflow("Sequenz2");
    Serial.println(error_sending);
    delay(1000);
    checkFiles("/data");
  }

  if(error_sending) {
    error_deleting = deleting_workflow("Sequenz2");
    Serial.println(error_deleting);
    delay(1000);
    checkFiles("/data");
  }
}

void loop() {
  yield();
}


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