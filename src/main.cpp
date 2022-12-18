#include <Arduino.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "workflows.h"
#include "website.h"


ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", index_html); //Send web page
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleForm() {
  String sequence = server.arg("sequence");
  String send_sequence_button = server.arg("send_sequence_button"); 
  String delete_sequence_button = server.arg("delete_sequence_button"); 
  String seqName = server.arg("seqName"); 
  String add_sequence_button = server.arg("add_sequence_button");
  String program = server.arg("program");
  String play_program_button = server.arg("play_program_button"); 
  String progCode = server.arg("progCode");
  String progName = server.arg("progName"); 
  String add_program_button = server.arg("add_program_button");
  String delete_programs_butto = server.arg("delete_program_button");


  // sequence logic:
  if (seqName != "" && add_sequence_button != "") {
    boolean recording_error = false;
    recording_error = recording_workflow(seqName);
    if (!recording_error) {
      Serial.println("failed to record sequence: " + seqName);
    }
    else {
      Serial.println("successfully added sequence: " + seqName);
    }
  }

  else if (sequence != "") {
    if (send_sequence_button != "") {
      boolean sending_error = false;
      sending_error = sending_workflow(sequence);
      if (!sending_error) {
        Serial.println("failed to send sequence: " + sequence);
      }
      else {
        Serial.println("successfully sent sequence: " + sequence);
      }
    }
    else if (delete_sequence_button != "") {
      boolean deleting_error = false;
      deleting_error = deleting_workflow("sequences", sequence);
      if (!deleting_error) {
        Serial.println("failed to delete sequence: " + sequence);
      }
      else {
        Serial.println("successfully deleted sequence: " + sequence);
      }
    }
  }

  // program logic:
  else if (progName != "" && add_program_button != "" && progCode != "") {
    boolean adding_error = false;
    adding_error = adding_workflow(progName, progCode);
    if (!adding_error) {
      Serial.println("failed to add program: " + progName);
    }
    else {
      Serial.println("successfully added program: " + progName);
    }
  }

  else if (program != "") {
    if (play_program_button != "") {
      boolean playing_error = false;
      playing_error = playing_workflow(program);
      if (!playing_error) {
        Serial.println("failed to play program: " + program);
      }
      else {
        Serial.println("successfully played program: " + program);
      }
    }
    else if (delete_programs_butto != "") {
      boolean deleting_error = false;
      deleting_error = deleting_workflow("programs", program);
      if (!deleting_error) {
        Serial.println("failed to delete program: " + program);
      }
      else {
        Serial.println("successfully deleted program: " + program);
      }
    }
  }
  
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}


// TODO: send Error message to frontend (when signal file in Program cannot be found)


void send_files() 
{
  String files = get_files("/sequences", "/programs");
  Serial.println(files);
  server.send(200, "text/plane", files);
}


void setup() {
  Serial.begin(115200);

  //reset saved settings
  //ESP.eraseConfig();
  WiFiManager wifiManager;
  wifiManager.autoConnect("IR-Remote");
  
  if (!MDNS.begin("irr")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");

  server.on("/", handleRoot);
  server.on("/get", handleForm);
  server.on("/files", send_files);
  server.onNotFound(handleNotFound);

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  MDNS.update();
  server.handleClient();
}

