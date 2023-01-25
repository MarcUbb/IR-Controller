# include "base.h"

/*
In this file, all functions related to the filesystem are defined.
*/


// TODO: Inputs jeweils auf Sinnhaftigkeit checken (überall und in Verknüpfung mit Unit Tests)


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

  // set the GPIOs for the IR-Receiver and output LED
  int receive_pin = 14;
  int led_pin = 5;
  pinMode(led_pin, OUTPUT);

  // set the parameters for the IR-Receiver
  int capture_buffer_size = 1024;
  int timeout_sequence = 50;
  int min_unknown_size = 12;
  int tolerance_percentage = kTolerance;  // 25%

  // initialize the IR-Receiver and results
  IRrecv irrecv(receive_pin, capture_buffer_size, timeout_sequence, true);
  decode_results results;

  // Ignore messages with less than minimum on or off pulses
  irrecv.setUnknownThreshold(min_unknown_size);
  irrecv.setTolerance(tolerance_percentage);  
  irrecv.enableIRIn();

  // initilize the 10s timer
  unsigned long start_time = millis();
  unsigned long timestamp = millis() - start_time;

  // turn on the LED to signalize the user that the capture process has started
  digitalWrite(led_pin, HIGH);
  

  // 10s timer (works also if overflow occurs)
  while(timestamp < 10000){
    // signal was captured:
    if (irrecv.decode(&results)){
      if (results.overflow == false){
        // blink the LED to signalize the user that the capture process has finished
        control_led_output("signal_received");
        // return the captured signal as a String
        return(resultToSourceCode(&results));
      }
    }
    timestamp = millis() - start_time;
    yield();
  }

  // no signal was captured in 10s:
  control_led_output("no_signal");
  return("no_signal");
}


String save_signal(String result_string, String name){
  /*
  parameters:
    String result_string:
      String containing the captured signal in the format:
        "uint16_t rawData[67] = {1234, 5678, ...};" 
        (format defined by resultToSourceCode() in IRremoteESP8266/src/IRutils.cpp)
    String name:
      name of the signal
  
  returns:
    String:
      "success" - if signal was saved successfully
      "Error: ..." - if an error occurred

  description:
  This function saves a captured signal in json format to a file. It uses the ArduinoJson library.
  
  calls:
    - save_json (in src/filesystem.cpp) to save the JSON document containing the captured signal to its file

  called by:
  - recording_workflow (in src/workflows.cpp) to save the captured signal to its file
  */

  // check if name is specified
  if (name == ""){
    return("Error: name is empthy");
  }

  if (check_if_string_is_alphanumeric(name) == false){
    return("Error: name is not alphanumeric");
  }

  if (name.length() > 32){
    return("Error: name exceeds 32 characters");
  }

  // extract length from String
  int first = result_string.indexOf("[");
  int last = result_string.indexOf("]");
  if (first == -1 || last == -1 || first > last || last - first > 4 || last - first < 2){
    return("Error: could not extract length from String");
  }
  String length = result_string.substring (first +1,last);

  // extract sequence from String
  first = result_string.indexOf("{");
  last = result_string.indexOf("}");
  if (first == -1 || last == -1 || first > last || last - first < 2){
    return("Error: could not extract sequence from String");
  }
  String sequence = result_string.substring (first +1,last);

  // creates JSON document from extracted data
  DynamicJsonDocument doc(3096);
  doc["name"] = name;
  doc["length"] = length.toInt();
  doc["sequence"] = sequence;
  doc.shrinkToFit();

  // save JSON document to file
  save_json("/signals/" + name  + ".json", doc);
  return("success");
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
  - save_signal (in src/filesystem.cpp) to save the JSON document containing the captured signal to its file
  - recording_workflow (in src/workflows.cpp) to save the JSON document containing the captured signal to its file
  - update_time (in src/time_management.cpp) to update time in /time.json
  - init_time (in src/time_management.cpp) to save NTP time to /time.json
  - check_and_update_offset (in src/time_management.cpp) to update millis() offset to /time.json
  */

  // initialize LittleFS
  LittleFS.begin();

  // check if file exists and delete it
  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  // Open file for writing
  File myfile = LittleFS.open(filename, "w");
  
  // Check if the file was opened
  if (!myfile) {
    Serial.println("Failed to create file");
    myfile.close();
    LittleFS.end();
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, myfile) == 0) {
    Serial.println("Failed to write to file");
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
  - update_time (in src/time_management.cpp) to load time from /time.json
  - get_current_time (in src/time_management.cpp) to load time and offset from /time.json
  - init_time (in src/time_management.cpp) to check if time in /time.json is already set
  - check_and_update_offset (in src/time_management.cpp) to load time from /time.json for overflow checking
  */

  // initialize LittleFS
  LittleFS.begin();

  // Open file for reading
  File myfile = LittleFS.open(filename, "r");

  // create JSON document
  DynamicJsonDocument doc(3096);

  // Check if the file was opened
  if (!myfile) {
    Serial.println("Failed to read file!");
    myfile.close();
    LittleFS.end();
    return doc;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, myfile);

  // if deserialization failed, delete the file and return empty JSON document
  if (error) {
    Serial.println("Failed to deserialize JSON");
    LittleFS.remove(filename);
    myfile.close();
    LittleFS.end();
    return doc;
  }

  // Close the file and return JSON document
  doc.shrinkToFit();
  myfile.close();
  LittleFS.end();
  return doc;
}


String send_signal(DynamicJsonDocument doc) {
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
    String:
      "success" if sending was successful
      "Error: ..." if sending failed
  
  description:
  This function sends a signal provided in JSON format. It uses the IRremoteESP8266 library and is based
  on the example code provided by the library.

  calls:
    ---

  called by:
  - sending_workflow (in src/workflows.cpp) to send the signal
  */

  // extract data from JSON document
  int length = doc["length"];
  String sequence = doc["sequence"];

  // check if length and sequence are valid
  if (length == 0 || sequence == "") {
    return("Error: invalid signal");
  }
  
  // set GPIO to be used for sending the signal
  int kIrLed = 4;

  // initialize IRsend object with GPIO
  IRsend irsend(kIrLed);

  // convert string to array of integers (thanks to https://stackoverflow.com/questions/48409822/convert-a-string-to-an-integer-array)
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
  // get the last value out of the string, which has no more commas in it
  command[ data_num ] = sequence.toInt();

  if (data_num != length-1) {
    return("Error: length of sequence does not match length in JSON document! Please save signal again.");
  }

  // send signal with IRsend object
  irsend.begin();
  irsend.sendRaw(command, length, 38);
  return("success");
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

  // declare variables
  String files = "";
  boolean signals = false;
  boolean programs = false;

  // initialize LittleFS
  LittleFS.begin();

  // scan signals folder for files and add them to the String
  Dir dir = LittleFS.openDir(folder_signals);
  while (dir.next()) {
    // sets signals to true if there is at least one signal
    signals = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 5);
    files += filename;
    files += ",";
  }

  // only remove last comma if there is at least one signal in the list
  if (signals == true){
    files = files.substring(0, files.length() - 1);
  }

  // add semicolon to separate signals and programs
  files += ";";

  // scan programs folder for files and add them to the String
  dir = LittleFS.openDir(folder_programs);
  while (dir.next()) {
    // sets programs to true if there is at least one program
    programs = true;
    String filename = dir.fileName();
    filename = filename.substring(0, filename.length() - 5);
    files += filename;
    files += ",";
  }

  // only remove last comma if there is at least one program in the list
  if (programs == true){
    files = files.substring(0, files.length() - 1);
  }
  
  // close LittleFS and return String
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

  // declare variables
  String filename = "/programs/" + program_name + ".json";
  String file_content = "";

  // check if file exists
  if (check_if_file_exists(filename) == false){
    return("Error: program does not exist");
  }

  // read file and save content to String
  LittleFS.begin();
  File file = LittleFS.open(filename, "r");
  while (file.available()) {
    file_content += (char)file.read();
  }

  file.close();
  LittleFS.end();

  // return String
  return file_content;
}


void control_led_output(String signal) {
  /*
  parameters:
    String signal:
      String serves as code for the signal received:
        "no_signal" - LED blinks 3 times
        "no_mDNS" - LED blinks 3 times
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
  - setup (in src/main.cpp) to signal when the mDNS setup failed
  */

  // declare variables
  int led_pin = 5;
  pinMode(led_pin, OUTPUT);

  // 3 blinks:
  if (signal == "no_signal" || signal == "no_mDNS" || signal == "no_wifi") {
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

  // 2 blinks:
  else if (signal == "AP_on") {
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

  // 1 blink:
  else if (signal == "signal_received" || signal == "AP_off") {
    digitalWrite(led_pin, LOW);
    delay(100);
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
  }

  return;
}


boolean check_if_string_is_alphanumeric (String word) {
  /*
  parameters:
    String word:
      String to be checked

  returns:
    boolean:
      true if the String is alphanumeric, false if not

  description:
  This function checks if a String is alphanumeric.

  calls:
    ---

  called by:
  - handle_form (in src/main.cpp) to check if the name of a new signal or program is alphanumeric
  - save_signal (in src/filesystem.cpp) to check if the name of a new signal is alphanumeric
  */

  // check if String is alphanumeric
  for (unsigned int i = 0; i < word.length(); i++) {
    if (isalnum(word[i]) == false) {
      // spaces, dashes or underscores are also allowed
      if (word[i] != ' ' && word[i] != '-' && word[i] != '_') {
        return false;
      }
    }
  }

  return true;
}