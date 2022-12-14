#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <LittleFS.h>

boolean recording_workflow(String command_name);
boolean deleting_workflow(String command_name);
boolean sending_workflow(String command_name);

String captureSignal();
DynamicJsonDocument convertToJSON(String result_string, String name);
void saveCommand(String filename, DynamicJsonDocument doc);
DynamicJsonDocument loadCommand(String filename);
void sendCommand(DynamicJsonDocument doc);
void checkFiles(String directory);
boolean checkFile(String filename);
void printFile(String filename);


boolean recording_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/data/" + command_name + ".txt";
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


boolean deleting_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/data/" + command_name + ".txt";
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
  String filename = "/data/" + command_name + ".txt";
  // 2. loads Command and saves it in JSON Object
  DynamicJsonDocument doc = loadCommand(filename);
  // 3. (send command)
  sendCommand(doc);
  return(true);
}

// TODO: call by reference woimmer es Sinn macht und geht

// TODO: 65535 (max int value) appears regulary (fix)
// receivePin = 14; captureBufferSize = 1024; timeoutSequence = 90; minUnknownSize = 12; timeoutRecording = 10000;
String captureSignal(){
  int receivePin = 14;
  int ledPin = 5;
  pinMode(ledPin, OUTPUT);
  int captureBufferSize = 1024;
  int timeout = 50;
  int minUnknownSize = 12;
  int tolerancePercentage = kTolerance;  // kTolerance is normally 25%

  // Use turn on the save buffer feature for more complete capture coverage.
  IRrecv irrecv(receivePin, captureBufferSize, timeout, true);
  decode_results results;  // Somewhere to store the results

  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(minUnknownSize);
  irrecv.setTolerance(tolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver

  int start_time = millis();
  int timestamp = millis() - start_time;
  digitalWrite(ledPin, HIGH);
  //Serial.begin(115200);
  while(timestamp < 10000){
    timestamp = millis() - start_time;
    if (timestamp % 1000 == 0){
      Serial.println((int)timestamp/1000);
      delay(1);
    }
    // Check if the IR code has been received.
    if (irrecv.decode(&results)){
      if (results.overflow == false){
        Serial.println("captured raw sequence:");
        Serial.println(resultToSourceCode(&results));
        digitalWrite(ledPin, LOW);
        delay(100);
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        return resultToSourceCode(&results);
      }
    }
  }

  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);

  Serial.println("No signal received");
  return "No Signal";
}


DynamicJsonDocument convertToJSON(String result_string, String name){

  // extracts length and sequence from String
  unsigned first = result_string.indexOf("[");
  unsigned last = result_string.indexOf("]");
  String length = result_string.substring (first +1,last);

  first = result_string.indexOf("{");
  last = result_string.indexOf("}");
  String sequence = result_string.substring (first +1,last);

  DynamicJsonDocument doc(3096);
  doc["name"] = name;
  doc["length"] = length.toInt();
  doc["sequence"] = sequence;
  doc.shrinkToFit();
  delay(1000);
  Serial.println("Converting Sequence to JSON format:");
  serializeJson(doc, Serial);
  return doc;
}


void saveCommand(String filename, DynamicJsonDocument doc) {

  LittleFS.begin();

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  File myfile = LittleFS.open(filename, "w");
  

  if (!myfile) {
    Serial.println(F("Failed to create file"));
    myfile.close();
    LittleFS.end();
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, myfile) == 0) {
    Serial.println(F("Failed to write to file"));
    myfile.close();
    LittleFS.end();
    return;
  }
  // Close the file
  myfile.close();
  LittleFS.end();
  return;
}


DynamicJsonDocument loadCommand(String filename) {
  LittleFS.begin();
  // Open file for reading
  File myfile = LittleFS.open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(3096);

  if (!myfile) {
    Serial.println("Failed to read file!");
    myfile.close();
    LittleFS.end();
    return doc;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, myfile);
  if (error) {
    Serial.println("Failed to read file, using default configuration");
    LittleFS.remove(filename);
    myfile.close();
    LittleFS.end();
    return doc;
  }

  doc.shrinkToFit();
  // Close the file (Curiously, File's destructor doesn't close the file)
  myfile.close();
  LittleFS.end();
  return doc;
}

// TODO: zu call by reference ändern
void sendCommand(DynamicJsonDocument doc) {

  int length = doc["length"];

  String sequence = doc["sequence"];
  
  int kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

  IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

  // https://stackoverflow.com/questions/48409822/convert-a-string-to-an-integer-array
  unsigned int data_num = 0;
  uint16_t command[length];
  // loop as long as a comma is found in the string
  while(sequence.indexOf(",")!=-1){
    // take the substring from the start to the first occurence of a comma, convert it to int and save it in the array
    command[ data_num ] = sequence.substring(0,sequence.indexOf(",")).toInt();
    data_num++; // increment our data counter
    //cut the data string after the first occurence of a comma
    sequence = sequence.substring(sequence.indexOf(",")+1);
  }
  // get the last value out of the string, which as no more commas in it
  command[ data_num ] = sequence.toInt();

  irsend.begin();

  irsend.sendRaw(command, length, 38);
  return;
}


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
  File file2 = LittleFS.open(filename, "r");

  if (!file2) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("");
  Serial.println("JSON in file:");
  while (file2.available()) {
    Serial.write(file2.read());
  }

  file2.close();
  LittleFS.end();
}