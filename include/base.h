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
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


String captureSignal();
DynamicJsonDocument convertToJSON(String result_string, String name);
void saveCommand(String filename, DynamicJsonDocument doc);
DynamicJsonDocument loadCommand(String filename);
void sendCommand(DynamicJsonDocument doc);

String get_files(String folderSequences, String folderPrograms);
boolean check_if_file_exists(String filename);
String get_NTP_time(int timezone);
String read_program(String program_name);
String weekday_to_num (String weekday);

boolean compare_time (String time, boolean weekday_included);
void save_timezone(String timezone);
int get_timezone();

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

String get_files(String folderSequences, String folderPrograms){
  String files = "";
  boolean sequences = false;
  boolean programs = false;

  LittleFS.begin();
  Dir dir = LittleFS.openDir(folderSequences);
  while (dir.next()) {
    sequences = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 4);
    files += filename;
    files += ",";
  }

  // only remove last comma if there is one
  if (sequences == true){
    files = files.substring(0, files.length() - 1);
  }

  files += ";";

  dir = LittleFS.openDir(folderPrograms);
  while (dir.next()) {
    programs = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 4);
    files += filename;
    files += ",";
  }

  if (programs == true){
    files = files.substring(0, files.length() - 1);
  }
  

  LittleFS.end();
  return files;
}


boolean check_if_file_exists(String filename) {
  LittleFS.begin();
  boolean exists = LittleFS.exists(filename);
  LittleFS.end();
  return exists;
}

// TODO: fix occasional wrong time
String get_NTP_time(int timezone){
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
  timeClient.begin();
  timeClient.setTimeOffset(timezone);
  timeClient.update();
  String time = timeClient.getFormattedTime();
  int weekday = timeClient.getDay();
  return(time + " " + weekday);
}

String read_program(String program_name){
  String filename = "/programs/" + program_name + ".txt";
  String file_content = "";
  LittleFS.begin();
  File file = LittleFS.open(filename, "r");
  while (file.available()) {
    file_content += (char)file.read();
  }
  file.close();
  LittleFS.end();
  return file_content;
}

String weekday_to_num (String weekday){
  if (weekday == "Monday" || weekday == "monday"){
    return "1";
  }
  else if (weekday == "Tuesday" || weekday == "tuesday"){
    return "2";
  }
  else if (weekday == "Wednesday" || weekday == "wednesday"){
    return "3";
  }
  else if (weekday == "Thursday" || weekday == "thursday"){
    return "4";
  }
  else if (weekday == "Friday" || weekday == "friday"){
    return "5";
  }
  else if (weekday == "Saturday" || weekday == "saturday"){
    return "6";
  }
  else if (weekday == "Sunday" || weekday == "sunday"){
    return "0";
  }
  else {
    return "error";
  }
}

boolean compare_time (String time, boolean weekday_included) {
  String current_time = get_NTP_time(get_timezone());
  delay(500);
  Serial.println("Current time: " + current_time);
  Serial.println("Time to compare: " + time);

  if (weekday_included == true) {
    return(time == current_time);
  }
  else {
    return(time == current_time.substring(0, current_time.indexOf(" ")));
  }
}

void save_timezone(String timezone){
  if (timezone == ""){
    Serial.println("No timezone given");
    return;
  }

  LittleFS.begin();
  File file = LittleFS.open("/timezone.txt", "w");
  if (!file) {
    Serial.println("Failed to create file");
    return;
  }
  file.println(timezone);
  file.close();
  LittleFS.end();
  return;
}

int get_timezone(){
  String timezone = "";
  LittleFS.begin();
  File file = LittleFS.open("/timezone.txt", "r");
  while (file.available()) {
    timezone += (char)file.read();
  }
  file.close();
  LittleFS.end();
  int timezone_int = timezone.toInt();
  timezone_int = timezone_int * 3600;
  return timezone_int;
}