# include "base.h"


// TODO: call by reference woimmer es Sinn macht und geht


// implementation of the signal capturing routing
// receive_pin = 14; capture_buffer_size = 1024; timeout_sequence = 50; min_unknown_size = 12; timeout_recording = 10000;
String capture_signal(){
  int receive_pin = 14;
  int led_pin = 5;
  pinMode(led_pin, OUTPUT);
  int capture_buffer_size = 1024;
  int timeout_sequence = 50;
  int min_unknown_size = 12;
  int tolerance_percentage = kTolerance;  // kTolerance is normally 25%

  // Use turn on the save buffer feature for more complete capture coverage.
  IRrecv irrecv(receive_pin, capture_buffer_size, timeout_sequence, true);
  decode_results results;  // Somewhere to store the results

  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(min_unknown_size);
  irrecv.setTolerance(tolerance_percentage);  // Override the default tolerance.
  irrecv.enableIRIn();  // Start the receiver

  unsigned long start_time = millis();
  unsigned long timestamp = millis() - start_time;
  digitalWrite(led_pin, HIGH);
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
        control_led_output("signal_received");
        return(resultToSourceCode(&results));
      }
    }
  }

  control_led_output("no_signal");
  return("No Signal");
}

// converts the captured signal to a JSON format
DynamicJsonDocument convert_to_JSON(String result_string, String name){

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
void send_signal(DynamicJsonDocument doc) {

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
// this function is used to update the website list of signals and programs
String get_files(String folder_signals, String folder_programs){
  String files = "";
  boolean signals = false;
  boolean programs = false;

  LittleFS.begin();
  Dir dir = LittleFS.openDir(folder_signals);
  while (dir.next()) {
    signals = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 5);
    files += filename;
    files += ",";
  }

  // only remove last comma if there is one
  if (signals == true){
    files = files.substring(0, files.length() - 1);
  }

  files += ";";

  dir = LittleFS.openDir(folder_programs);
  while (dir.next()) {
    programs = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 5);
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
  String filename = "/programs/" + program_name + ".json";
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


void control_led_output(String signal) {
  
  int led_pin = 5;
  pinMode(led_pin, OUTPUT);

  if (signal == "no_signal" || signal == "no_mDNS") {
    digitalWrite(led_pin, LOW);
    delay(100);
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
    delay(100);
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
    delay(100);
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
  }

  else if (signal == "signal_received") {
    digitalWrite(led_pin, LOW);
    delay(100);
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
  }

  return;
}