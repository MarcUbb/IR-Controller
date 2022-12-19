#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "workflows.h"
#include "website.h"

void handleRoot();
void handleNotFound();
void handleForm();
void send_files();
void handleProgram();
void handleError();
void handleTime();


ESP8266WebServer server(80);

String programname = "";
String message = "";

void setup() {
  Serial.begin(115200);

  //reset saved settings:
  //ESP.eraseConfig();
  WiFiManager wifiManager;
  wifiManager.autoConnect("IR-Remote");
  
  if (!MDNS.begin("irr")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started");

  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.on("/files", send_files);
  server.on("/program", handleProgram);
  server.on("/error", handleError);
  server.on("/time", handleTime);
  server.onNotFound(handleNotFound);

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  MDNS.update();
  server.handleClient();
}








//------------------ handlers ------------------//

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// Get request has to be on a separate page because esp hast to send back command because submit buttons trigger forwarding
void handleProgram() {
  String programcode = read_program(programname);
  server.send(200, "text/plain", programname + ";" + programcode);
}

void handleError() {
  server.send(200, "text/plain", message);
}

void send_files() {
  String files = get_files("/sequences", "/programs");
  Serial.println(files);
  server.send(200, "text/plane", files);
}

void handleTime() {
  String timezone = server.arg("time_input");
  save_timezone(timezone);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
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
  String delete_programs_button = server.arg("delete_program_button");
  String edit_program_button = server.arg("edit_program_button");


  // sequence logic:
  if (seqName != "" && add_sequence_button != "") {
    boolean recording_error = false;
    recording_error = recording_workflow(seqName);
    if (!recording_error) {
      message = "failed to record sequence: " + seqName;
    }
    else {
      message = "successfully added sequence: " + seqName;
    }
  }

  else if (seqName == "" && add_sequence_button != "") {
    message = "no sequence name given";
  }

  else if (sequence != "") {
    if (send_sequence_button != "") {
      boolean sending_error = false;
      sending_error = sending_workflow(sequence);
      if (!sending_error) {
        message = "failed to send sequence: " + sequence;
      }
      else {
        message = "successfully sent sequence: " + sequence;
      }
    }
    else if (delete_sequence_button != "") {
      boolean deleting_error = false;
      deleting_error = deleting_workflow("sequences", sequence);
      if (!deleting_error) {
        message = "failed to delete sequence: " + sequence;
      }
      else {
        message = "successfully deleted sequence: " + sequence;
      }
    }
  }

  else if (sequence == "" && send_sequence_button != "") {
    message = "no sequence selected";
  }

  else if (sequence == "" && delete_sequence_button != "") {
    message = "no sequence selected";
  }

  // program logic:
  else if (progName != "" && add_program_button != "" && progCode != "") {
    boolean adding_error = false;
    adding_error = adding_workflow(progName, progCode);
    if (!adding_error) {
      message = "failed to add program: " + progName;
    }
    else {
      message = "successfully added program: " + progName;
    }
  }

  else if (progName == "" && add_program_button != "") {
    message = "no program name given";
  }

  else if (progCode == "" && add_program_button != "") {
    message = "no program code given";
  }

  else if (program != "") {
    if (play_program_button != "") {
      boolean playing_error = false;
      playing_error = playing_workflow(program);
      if (!playing_error) {
        message = "failed to play program: " + program;
      }
      else {
        message = "successfully played program: " + program;
      }
    }
    else if (delete_programs_button != "") {
      boolean deleting_error = false;
      deleting_error = deleting_workflow("programs", program);
      if (!deleting_error) {
        message = "failed to delete program: " + program;
      }
      else {
        message = "successfully deleted program: " + program;
      }
    }
    // provides data for separate Get request
    else if (edit_program_button != "") {
      programname = program;
      message = "successfully loaded program: " + program;
    }
  }

  else if (program == "" && play_program_button != "") {
    message = "no program selected";
  }

  else if (program == "" && delete_programs_button != "") {
    message = "no program selected";
  }

  else if (program == "" && edit_program_button != "") {
    message = "no program selected";
  }

  if (edit_program_button == "") {
    programname = "";
  }
  
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}