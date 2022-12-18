#include <Arduino.h>
#include <ArduinoJson.h>
#include "base.h"

boolean recording_workflow(String command_name);
boolean deleting_workflow(String command_name);
boolean sending_workflow(String command_name);

boolean adding_workflow(String progName, String progCode);
boolean playing_workflow(String program);


boolean recording_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/sequences/" + command_name + ".txt";
  // 2. signal is received:
  String raw_sequence = captureSignal();
  // 3. if sinal was received successfully:
  if (raw_sequence != "No Signal"){
    // 3.1 signal String is serialized to JSON:
    DynamicJsonDocument sequence_JSON = convertToJSON(raw_sequence, command_name);
    // 3.2 JSON is written to file:
    saveCommand(filename, sequence_JSON);
    return(true);
  }
  return(false);
}


boolean deleting_workflow(String directory, String command_name) {
  // 1. filename is generated:
  String filename = "/" + directory + "/" + command_name + ".txt";
  LittleFS.begin();
  // 2. check if file exists:
  if(LittleFS.exists(filename)){
    // 2.1 delete file:
    LittleFS.remove(filename);
    LittleFS.end();
    return(true);
  }
  LittleFS.end();
  return(false);
}


boolean sending_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/sequences/" + command_name + ".txt";

  if (check_if_file_exists(filename) == false) {
    Serial.println("file does not exist");
    return(false);
  }
  // 2. loads Command and saves it in JSON Object
  DynamicJsonDocument doc = loadCommand(filename);
  // 3. (send command)
  sendCommand(doc);
  return(true);
}

boolean adding_workflow(String progName, String progCode) {
  // 1. filename is generated:
  String filename = "/programs/" + progName + ".txt";
  // 2. if file does not exist:
  LittleFS.begin();

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  File myfile = LittleFS.open(filename, "w");
  

  if (!myfile) {
    Serial.println(F("Failed to create file"));
    myfile.close();
    LittleFS.end();
    return(false);
  }
  // 3. write code to file:
  myfile.write(progCode.c_str());
  // Close the file
  myfile.close();
  LittleFS.end();
  return(true);
}

boolean playing_workflow(String program) {
  // 1. filename is generated:
  String filename = "/programs/" + program + ".txt";
  
  LittleFS.begin();
  File myfile = LittleFS.open(filename, "r");

  if (!myfile) {
    Serial.println("Failed to open file for reading");
    return(false);
  }

  String filecontent = myfile.readString();
  myfile.close();
  LittleFS.end();

  while (filecontent.indexOf("\n") != -1){
    String line = filecontent.substring(0, filecontent.indexOf("\n") - 1);
    filecontent = filecontent.substring(filecontent.indexOf("\n") + 1);

    // TODO: send error message if file is not found
    if (line.indexOf("play") == 0){
      String command_name = line.substring(5);
      boolean success = sending_workflow(command_name);
      if (success == false) {
        Serial.println("sending workflow failed");
        return(false);
      }
      Serial.println("successfully sent sequence: " + command_name);
    }
    else if (line.indexOf("wait") == 0){
      String delay_time = line.substring(5);
      try {
        delay(delay_time.toInt());
      }
      catch (const std::exception& e) {
        Serial.println("delay failed");
        return(false);
      }
    }
  }

  // an extra time because the last line does not have a \n
  String line = filecontent;

  if (line.indexOf("play") == 0){
    String command_name = line.substring(5);
    boolean success = sending_workflow(command_name);
    if (success == false) {
      Serial.println("sending workflow failed");
      return(false);
    }
    Serial.println("successfully sent sequence: " + command_name);
  }
  else if (line.indexOf("wait") == 0){
    String delay_time = line.substring(5);
    try {
        delay(delay_time.toInt());
      }
      catch (const std::exception& e) {
        Serial.println("delay failed");
        return(false);
      } 
  }

  return(true);
}