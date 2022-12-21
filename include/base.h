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
void save_json(String filename, DynamicJsonDocument doc);
DynamicJsonDocument load_json(String filename);
void sendCommand(DynamicJsonDocument doc);

String get_files(String folderSequences, String folderPrograms);
boolean check_if_file_exists(String filename);
String read_program(String program_name);
String weekday_to_num (String weekday);

boolean compare_time (String time, boolean weekday_included);
void save_time(String time, unsigned long offset);
String get_time();
String turn_seconds_in_time(int seconds);
String add_time(String time, String offset_time);

DynamicJsonDocument get_NTP_time(int timezone);
boolean init_time();
void check_and_update_offset()

// TODO: call by reference woimmer es Sinn macht und geht

// TODO: 65535 (max int value) appears regulary (fix)
// receivePin = 14; captureBufferSize = 1024; timeoutSequence = 90; minUnknownSize = 12; timeoutRecording = 10000;

// implementation of the signal capturing routing
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

  unsigned long start_time = millis();
  unsigned long timestamp = millis() - start_time;
  digitalWrite(ledPin, HIGH);
  //Serial.begin(115200);
  while(timestamp < 10000){
    timestamp = millis() - start_time;
    if (timestamp % 1000 == 0){
      Serial.println((unsigned long)timestamp/1000);
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

// converts the captured signal to a JSON format
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

// saves the converted JSON to a file
void save_json(String filename, DynamicJsonDocument doc) {

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


// loads the JSON of a file
DynamicJsonDocument load_json(String filename) {
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

// Implementation of the IR sending routine
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


// scans the LittleFS for files and returns a String with all files
// this function is used to update the website list of sequences and programs
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

// checks if a file exists (used in multiple functions)
boolean check_if_file_exists(String filename) {
  LittleFS.begin();
  boolean exists = LittleFS.exists(filename);
  LittleFS.end();
  return exists;
}


// reads a program from the LittleFS and returns it as a String
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


// converts a weekday String to a number
// function is used to compare the weekday of the program with the current weekday and the conversion is necessary because the user
// enters the weekday as a String
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


// elementary function of timed programs as it compares the current time with the time in the program
boolean compare_time (String time, boolean weekday_included) {

  // checks if millis() overflowed and updates time if necessary
  check_and_update_offset();

  String current_time = get_time();
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


// Saves time from website to LittleFS. Offset is not generated because it would create greater error
void save_time(String time, unsigned long offset){
  if (time == ""){
    Serial.println("No time available");
    return;
  }

  DynamicJsonDocument time_json(1024);

  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();
  int weekday = time.substring(time.indexOf(" ") + 1, time.indexOf(" ") + 2).toInt();
  int timezone = time.substring(time.indexOf(" ") + 3).toInt();

  time_json["hours"] = hours;
  time_json["minutes"] = minutes;
  time_json["seconds"] = seconds;
  time_json["weekday"] = weekday;
  time_json["init_offset"] = offset;
  time_json["timezone"] = timezone;

  // check if this can cause problems with overflow
  time_json["last_offset"] = millis();
  time_json.shrinkToFit();

  save_json("/time.txt", time_json);
  return;
}


// loads the time from the LittleFS, adds the current offset and returns the current time
String get_time(){
  DynamicJsonDocument time_json = load_json("/time.txt");

  String time =  time_json["hours"].as<String>();
  time += ":";
  time += time_json["minutes"].as<String>();
  time += ":";
  time += time_json["seconds"].as<String>();
  time += " ";
  time += time_json["weekday"].as<String>();

  
  unsigned long init_offset = time_json["init_offset"].as<unsigned long>();


  unsigned long effective_offset = millis() - init_offset;
  unsigned long effective_offset_seconds = effective_offset / 1000;

  String offset_time = turn_seconds_in_time(effective_offset_seconds);
  time = add_time(time, offset_time);

  return time;
}

// converts seconds to time format. Is used in get_time() to prepare millis() offset for comparison with saved time
String turn_seconds_in_time(int input_seconds){
  int hours = input_seconds / 3600;
  int minutes = (input_seconds % 3600) / 60;
  int seconds = input_seconds % 60;

  String time = "";
  if (hours < 10){
    time += "0";
  }
  time += hours;
  time += ":";
  if (minutes < 10){
    time += "0";
  }
  time += minutes;
  time += ":";
  if (seconds < 10){
    time += "0";
  }
  time += seconds;

  return time;
}


// adds two times together. Is used in get_time() to add the offset to the saved time
String add_time(String time, String offset_time){
  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();
  int weekday = time.substring(time.indexOf(" ") + 1).toInt();

  int offset_hours = offset_time.substring(0, offset_time.indexOf(":")).toInt();
  int offset_minutes = offset_time.substring(offset_time.indexOf(":") + 1, offset_time.indexOf(":") + 3).toInt();
  int offset_seconds = offset_time.substring(offset_time.indexOf(":") + 4, offset_time.indexOf(":") + 6).toInt();

  seconds += offset_seconds;
  if (seconds >= 60){
    seconds -= 60;
    minutes += 1;
  }
  minutes += offset_minutes;
  if (minutes >= 60){
    minutes -= 60;
    hours += 1;
  }
  hours += offset_hours;

  while (hours >= 24){
    hours -= 24;
    weekday += 1;
  }
  while (weekday >= 7){
    weekday -= 7;
  }

  String time_sum = "";

  if (hours < 10){
    time_sum += "0";
  }
  time_sum += hours;
  time_sum += ":";
  if (minutes < 10){
    time_sum += "0";
  }
  time_sum += minutes;
  time_sum += ":";
  if (seconds < 10){
    time_sum += "0";
  }
  time_sum += seconds;
  time_sum += " ";
  time_sum += weekday;
  return (time_sum);
}


// only used in intital setup to get the time from the web without user interaction
DynamicJsonDocument get_NTP_time(int timezone){
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone);
  timeClient.begin();
  timeClient.update();
  String time = timeClient.getFormattedTime();
  int weekday = timeClient.getDay();
  time = time + " " + weekday;

  DynamicJsonDocument time_json(1024);

  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();
  int weekday = time.substring(time.indexOf(" ") + 1).toInt();

  time_json["hours"] = hours;
  time_json["minutes"] = minutes;
  time_json["seconds"] = seconds;
  time_json["weekday"] = weekday;
  time_json["init_offset"] = millis();
  time_json["timezone"] = timezone;

  time_json["last_offset"] = millis();
  time_json.shrinkToFit();

  return(time_json);
}

boolean init_time(){

  // loads the time from time.txt
  DynamicJsonDocument current_values = load_json("/time.txt");

  // no time was saved yet (first boot)
  if (current_values.isNull()){
    save_json("/time.txt", get_NTP_time(0));
    return false;
  }

  // if there is a time saved: update it according to the timezone and overwrite offsets with current offset
  else {
    save_json("/time.txt", get_NTP_time(current_values["timezone"]));
    return true;
  }
}


// checks if overflow occured and updates time if necessary
// generates current_offset and compares to last offset
// if current_offset < last_offset: overflow occured and last_offset is resetted
void check_and_update_offset() {
  DynamicJsonDocument current_values = load_json("/time.txt");
  unsigned long last_offset = current_values["last_offset"];
  unsigned long current_offset = millis();

  // no overflow occured:
  if (current_offset > last_offset){
    current_values["last_offset"] = current_offset;
    save_json("/time.txt", current_values);
    Serial.println("no overflow occured");
    return;
  }
  // overflow occured:
  else if (current_offset < last_offset){
    unsigned long overflow_seconds = 4294967;
    String overflow_time = turn_seconds_in_time(overflow_seconds);

    DynamicJsonDocument init_values = load_json("/time.txt");

    String init_time = init_values["hours"].as<String>();
    init_time += ":";
    init_time += init_values["minutes"].as<String>();
    init_time += ":";
    init_time += init_values["seconds"].as<String>();
    init_time += " ";
    init_time += init_values["weekday"].as<String>();

    String current_time = add_time(init_time, overflow_time);

    unsigned long current_offset = millis();
    unsigned long current_offset_seconds = current_offset/1000;
    String overflowed_time = turn_seconds_in_time(current_offset_seconds);
    current_time = add_time(current_time, overflowed_time);

    current_values["hours"] = current_time.substring(0, current_time.indexOf(":")).toInt();
    current_values["minutes"] = current_time.substring(current_time.indexOf(":") + 1, current_time.indexOf(":") + 3).toInt();
    current_values["seconds"] = current_time.substring(current_time.indexOf(":") + 4, current_time.indexOf(":") + 6).toInt();
    current_values["weekday"] = current_time.substring(current_time.indexOf(" ") + 1).toInt();
    current_values["last_offset"] = current_offset;
    current_values["init_offset"] = current_offset;
    current_values["timezone"] = current_values["timezone"];

    save_json("/time.txt", current_values);
    Serial.println("overflow occured! Time was updated.");
    return;
  }
  // no change occured:
  else {
    return;
  }
}