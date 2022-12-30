# include "base.h"

/*
In this file, all functions related to the filesystem are defined.
*/


// TODO: call by reference woimmer es Sinn macht und geht


String capture_signal(){
  /*
  parameters:
    ---
  
  returns:
    String:
      String containing the captured signal in the format:
        "uint16_t rawData[67] = {1234, 5678, ...};" - if signal was receiver, 
          format defined by resultToSourceCode() in IRremoteESP8266/src/IRutils.cpp
        "no_signal" - if no signal was captured

  description:
  This function captures a signal and returns it as a String. It uses the IRremoteESP8266 library
  and is based on the IRrecvDumpV2 example from the library. 
  
  calls:
  - control_led_output (in src/filesystem.cpp) to signalize the state of the capture process

  called by:
  - recording_workflow (in src/workflows.cpp) to capture a signal
  */
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
  
  while(timestamp < 10000){ // works also if overflow occured
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
  return("no_signal");
}


DynamicJsonDocument convert_to_JSON(String result_string, String name){
  /*
  parameters:
    String result_string:
      String containing the captured signal in the format:
        "uint16_t rawData[67] = {1234, 5678, ...};" 
        (format defined by resultToSourceCode() in IRremoteESP8266/src/IRutils.cpp)
    String name:
      name of the signal
  
  returns:
    DynamicJsonDocument:
      JSON document containing the captured signal in the format:
        {
          "name": "name",
          "length": 67,
          "sequence": "1234, 5678, ..."
        }

  description:
  This function converts the captured signal to a JSON format. It uses the ArduinoJson library.
  
  calls:
    ---

  called by:
  - recording_workflow (in src/workflows.cpp) to convert the captured signal to JSON format
  */


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


void save_json(String filename, DynamicJsonDocument doc) {
  /*
  parameters:
    String filename:
      name of the file
    DynamicJsonDocument doc:
      JSON document containing unspecified data:
        {
          "...": "...",
          "..."
        }

  returns:
    void
  
  description:
  This function saves a JSON document to a specified file. It uses LittleFS and the ArduinoJson library.
  
  calls:
    ---

  called by:
  - recording_workflow (in src/workflows.cpp) to save the JSON document containing the captured signal to its file
  - update_timezone (in src/time_management.cpp) to update timezone in /time.json
  - init_time (in src/time_management.cpp) to save NTP time to /time.json
  - check_and_update_offset (in src/time_management.cpp) to update millis() offset to /time.json

  */


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


DynamicJsonDocument load_json(String filename) {
  /*
  parameters:
    String filename:
      name of the file
  
  returns:
    DynamicJsonDocument:
      JSON document containing unspecified data:
        {
          "...": "...",
          "..."
        }
  
  description:
  This function loads a JSON document from a specified file. It uses LittleFS and the ArduinoJson library.

  calls:
    ---

  called by:
  - sending_workflow (in src/workflows.cpp) to load the JSON document containing the signal to be sent
  - update_timezone (in src/time_management.cpp) to load timezone from /time.json
  - get_current_time (in src/time_management.cpp) to load time and offset from /time.json
  - init_time (in src/time_management.cpp) to check if time in /time.json is already set
  - check_and_update_offset (in src/time_management.cpp) to load time from /time.json for overflow checking

  */

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

void send_signal(DynamicJsonDocument doc) {
  /*
  parameters:
    DynamicJsonDocument doc:
      JSON document containing the signal to be sent:
        {
          "name": "name",
          "length": 67,
          "sequence": "1234, 5678, ..."
        }
  
  returns:
    void
  
  description:
  This function sends a signal provided in JSON format. It uses the IRremoteESP8266 library and is based
  on the example code provided by the library.

  calls:
    ---

  called by:
  - sending_workflow (in src/workflows.cpp) to send the signal
  */

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


String get_files(String folder_signals, String folder_programs){
  /*
  parameters:
    String folder_signals:
      name of the folder containing the signals
    String folder_programs:
      name of the folder containing the programs

  returns:
    String:
      String containing all files in the specified folders, separated by a semicolon:
        "signal1, signal2, ...;program1, program2, ..."
  
  description:
  This function scans the signal and program folders for files and returns a String containing all files. It uses LittleFS.
  
  calls:
    ---

  called by:
  - handle_files (in src/main.cpp) to keep the displayed signals and programs updated
  */


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


boolean check_if_file_exists(String filename) {
  /*
  parameters:
    String filename:
      name of the file to be checked
  
  returns:
    boolean:
      true if the file exists, false if not

  description:
  This function checks if a file exists in the LittleFS.

  calls:
    ---

  called by:
  - called by multiple functions to check if a file exists
  */
  LittleFS.begin();
  boolean exists = LittleFS.exists(filename);
  LittleFS.end();
  return exists;
}


String read_program(String program_name){
  /*
  parameters:
    String program_name:
      name of the program to be read
    
  returns:
    String:
      String containing the program code

  description:
  This function has its reason of existence next to load_json because in the frontend, 
  after pressing the "edit" button, the program code of the specified program is displayed 
  in the textarea as a string.

  calls:
    ---

  called by:
  - handle_program (in src/main.cpp) to update displayed program code
  */
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
  /*
  parameters:
    String signal:
      String serves as code for the signal received:
        "no_signal" - LED blinks 3 times
        "no_mDNS" - LED blinks 3 times
        "no_init_time" - LED blinks 3 times
        "signal_received" - LED blinks once

  returns:
    void
  
  description:
  This function controls the LED output via codewords to specify the kind of signal to be send.
  The LED is connected to GPIO 5 (D1) on the ESP8266 and is ment as a way to communicate errors
  to the user and singal when the ESP is ready to receive a signal.

  calls:
    ---

  called by:
  - capture_signal (in src/filesystem.cpp) to signal when the ESP is ready for receiving a signal,
    when a signal was sucessfully received or when no signal was received
  - setup (in src/main.cpp) to signal when the mDNS setup failed and to signal when the time could not be set
  */


  int led_pin = 5;
  pinMode(led_pin, OUTPUT);

  if (signal == "no_signal" || signal == "no_mDNS" || signal == "no_init_time") {
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