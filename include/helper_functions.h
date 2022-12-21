


// unused functions which could be useful in the future or when debugging are saved here

#include <Arduino.h>
#include <LittleFS.h>

// https://arduino.stackexchange.com/questions/76186/how-can-i-list-only-files-that-begin-with-log
void checkFiles(String directory) {
  LittleFS.begin();
  Dir dir = LittleFS.openDir(directory);

  while(dir.next()){
    Serial.println();
    Serial.print(dir.fileName());
    Serial.print(" - ");
    String filepath = "";
    if (directory != "/") {
      filepath += directory + "/" + dir.fileName();
    }
    else {
      filepath += "/" + dir.fileName();
    }
    Serial.print(filepath + ": ");
    boolean corrupted = true;  //checkFile(filepath);
    if(corrupted == true) {
      Serial.print("true");
    }
    else{
      Serial.print("false");
    }
  }
  Serial.println();
  LittleFS.end();
  return;
}

// may be removed
boolean checkFile(String filename) {
  LittleFS.begin();

  File myfile = LittleFS.open(filename, "r");

  if (!myfile) {
    myfile.close();
    LittleFS.end();
    return false;
  }

  DynamicJsonDocument doc(3096);
  DeserializationError error = deserializeJson(doc, myfile);
  doc.shrinkToFit();
  if (error) {
    myfile.close();
    LittleFS.end();
    doc.garbageCollect();
    return false;
  }

  if (doc["length"] < 12 || doc["length"] > 2048) {
    myfile.close();
    LittleFS.end();
    doc.garbageCollect();
    return false;
  }

  String sequence = doc["sequence"];
  int length = sequence.length();
  if (sequence.substring(length - 1) == "," || sequence.substring(0, 1) == ",") {
    myfile.close();
    LittleFS.end();
    doc.garbageCollect();
    return false;
  }

  myfile.close();
  LittleFS.end();
  doc.garbageCollect();
  return true;
}

void printFile(String filename) {
  LittleFS.begin();
  File myfile = LittleFS.open(filename, "r");

  if (!myfile) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("");
  Serial.println("File content:");
  Serial.println(myfile.readString());
  Serial.println("");

  myfile.close();
  LittleFS.end();
}


void delete_all_files_in_littlefs(){
  LittleFS.begin();
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.println(dir.fileName());
    LittleFS.remove(dir.fileName());
  }
  dir = LittleFS.openDir("/programs");
  while (dir.next()) {
    Serial.println(dir.fileName());
    LittleFS.remove("/programs/" + dir.fileName());
  }

  dir = LittleFS.openDir("/signals");
  while (dir.next()) {
    Serial.println(dir.fileName());
    LittleFS.remove("/signals/" + dir.fileName());
  }
  LittleFS.end();
}