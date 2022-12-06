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


// TODO: Umschreiben, sodass alle Sequenzen in data/Sequenzname, also in eigener Datei gespeichert werden


// TODO: lädt JSON aus Datei und liest Eintrag
/*
void loadCommand(String filename, int sequence[], String filter) {
  // Open file for reading
  File file = SPIFFS.open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<3096> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  config.port = doc["port"] | 2731;
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "example.com",  // <- source
          sizeof(config.hostname));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();

  // https://www.geeksforgeeks.org/convert-a-string-to-integer-array-in-c-c/
  // get length of string str
  int str_length = output.length();

  // create an array with size as string
  // length and initialize with 0
  int arr[str_length] = { 0 };
  int j = 0, i, sum = 0;
  // Traverse the string
  for (i = 0; output[i] != '&#092;&#048;'; i++) {

    // if str[i] is ', ' then split
    if (output[i] == ',')
      continue;
    if (output[i] == ' '){
      // Increment j to point to next
      // array location
      j++;
    }
    else {

    // subtract str[i] by 48 to convert it to int
    // Generate number by multiplying 10 and adding
    // (int)(str[i])
    arr[j] = arr[j] * 10 + (output[i] - 48);
    }
  }
  return arr;
}
*/


// TODO: testen
void sendCommand(String result_string) {
  
  // extracts length and sequence from String
  unsigned first = result_string.indexOf("[");
  unsigned last = result_string.indexOf("]");
  int length = result_string.substring(first +1,last).toInt();

  first = result_string.indexOf("{");
  last = result_string.indexOf("}");
  String sequence = result_string.substring (first +1,last);
  
  int kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

  IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

  // Example of data captured by IRrecvDumpV2.ino
  uint16_t command[length] = {0};
  int j = 0, i;

  for (i = 0; sequence[i] != '&#092;&#048;'; i++) {

    // https://www.geeksforgeeks.org/convert-a-string-to-integer-array-in-c-c/
    // if str[i] is ', ' then split
    if (sequence[i] == ',')
      continue;
    if (sequence[i] == ' '){
      // Increment j to point to next
      // array location
      j++;
    }
    else {

      // subtract str[i] by 48 to convert it to int
      // Generate number by multiplying 10 and adding
      // (int)(str[i])
      command[j] = command[j] * 10 + (sequence[i] - 48);
    }
  }

  irsend.begin();

  Serial.println("a rawData capture from IRrecvDumpV2");
  irsend.sendRaw(command, length, 38);  
  
}

// TODO: lädt JSON aus Datei und entfernt Eintrag mit Komma
void deleteCommand(String name) {
  return;
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
  Serial.println("In JSON format:");
  serializeJson(doc, Serial);
  return doc;
}


void deleteFile(String filename) {
  LittleFS.begin();
  LittleFS.remove(filename);
  LittleFS.end();
  return;
}


void createFile(String filename, boolean replace) {
  LittleFS.begin();

  // File exists already:
  if (LittleFS.exists(filename)) {
    if (replace == false) {
      Serial.println("File exists already!");
      LittleFS.end();
      return;
    }
    else {
      LittleFS.remove(filename);
      File myfile = LittleFS.open(filename, "w");
      myfile.close();
      LittleFS.end();
      return;
    }
  }

  // File doesn't exist yet:
  else{
    File myfile = LittleFS.open(filename, "w");
    myfile.close();
    LittleFS.end();
    return;
  }
}


void initFile (String filename) {
  LittleFS.begin();
  File myfile = LittleFS.open(filename, "w");
  myfile.print("{\"data\":[]}");
  myfile.close();
  LittleFS.end();
}


void saveCommand(String filename, DynamicJsonDocument doc) {

  LittleFS.begin();

  File myfile = LittleFS.open(filename, "r+"); // See https://github.com/lorol/LITTLEFS/issues/33
  myfile.seek((myfile.size()-2), SeekSet);

  if (!myfile) {
    Serial.println(F("Failed to create file"));
    return;
  }

  if (myfile.size() > 11) {
    myfile.print(",");
  }

  // Serialize JSON to file
  if (serializeJson(doc, myfile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  myfile.print("]}");
  // Close the file
  myfile.close();
  LittleFS.end();
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

// TODO: handle case that no Signal was received
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
    }
    // Check if the IR code has been received.
    if (irrecv.decode(&results)){
      if (results.overflow == false){
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