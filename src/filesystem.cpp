/**
 * @file filesystem.cpp
 * 
 * @author Marc Ubbelohde
 * 
 * @brief In this file, all functions related to the filesystem are defined.
 * 
 * @details The functions in this file are used to save and load data from the filesystem, 
 * provide the frontend with the data it needs to display necessary information,
 * to handle signal capture and sending and to control the LED output. This file is the
 * foundation of the project and the functions are used by almost all other files.
 */

#include "base.h"


/**
 * @brief This function captures a signal and returns it as a String.
 * 
 * @return String - String containing the captured signal in the format:\n
 *                  "uint16_t rawData[67] = {1234, 5678, ...};" - if signal was receiver, format defined by resultToSourceCode() in IRremoteESP8266/src/IRutils.cpp\n 
 *                  "no_signal" - if no signal was captured
 * 
 * @details This function uses the IRremoteESP8266 library and is based on the IRrecvDumpV2 example from the library.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String capture_signal(){

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

/**
 * @brief This function saves a captured signal
 * 
 * @param result_string - String containing the captured signal in the format:\n
 *                       "uint16_t rawData[67] = {1234, 5678, ...};" (format defined by resultToSourceCode() in IRremoteESP8266/src/IRutils.cpp)\n
 * 
 * @param name - name of the signal
 * 
 * @return String - "success" - if signal was saved successfully\n
 *                 "Error: ..." - if an error occurred
 * 
 * @details This function saves a captured signal in json format to a file. It uses the ArduinoJson library.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String save_signal(String result_string, String name){

  // check if name is specified
  if (name == ""){
    return("Error: name is empthy");
  }

  if (check_if_string_is_alphanumeric(name) == false){
    return("Error: name is not alphanumeric");
  }

  if (name.length() > 21){
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

/**
 * @brief This function saves a JSON document to a specified file
 * 
 * @param filename - name of the file
 * 
 * @param doc - JSON document containing unspecified data
 * 
 * @details This function uses LittleFS and the ArduinoJson library.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
void save_json(String filename, DynamicJsonDocument doc) {

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
    Serial.println("/filesystem.cpp/save_json: Failed to create file: " + filename);
    myfile.close();
    LittleFS.end();
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, myfile) == 0) {
    Serial.println("/filesystem.cpp/save_json: Failed to write to file: " + filename);
    myfile.close();
    LittleFS.end();
    return;
  }

  // Close the file
  myfile.close();
  LittleFS.end();
  return;
}

/**
 * @brief This function loads a JSON document from a specified file
 * 
 * @param filename - name of the file
 * 
 * @return DynamicJsonDocument - JSON document containing unspecified data
 * 
 * @details This function uses LittleFS and the ArduinoJson library.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
DynamicJsonDocument load_json(String filename) {

  // initialize LittleFS
  LittleFS.begin();

  // Open file for reading
  File myfile = LittleFS.open(filename, "r");

  // create JSON document
  DynamicJsonDocument doc(3096);

  // Check if the file was opened
  if (!myfile) {
    Serial.println("/filesystem.cpp/load_json: Failed to read file: " + filename);
    myfile.close();
    LittleFS.end();
    return doc;
  }

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, myfile);

  // if deserialization failed, delete the file and return empty JSON document
  if (error) {
    Serial.println("/filesystem.cpp/load_json: Failed to deserialize JSON from file: " + filename);
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

/**
 * @brief This function sends a signal provided in JSON format.
 * 
 * @param doc - JSON document containing the signal to be sent:\n 
 *        {\n
 *        "name": "name",\n
 *        "length": 67,\n
 *        "sequence": "1234, 5678, ..."\n
 *        }
 * 
 * @return String - "success" if sending was successful\n
 *                  "Error: ..." if sending failed
 * 
 * @details This function sends a signal provided in JSON format. It uses the IRremoteESP8266 library and is based
 * on the example code provided by the library.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
String send_signal(DynamicJsonDocument doc) {

  // extract data from JSON document
  int length = doc["length"];
  String sequence = doc["sequence"];

  // check if length and sequence are valid (non null)
  if (length == 0 || sequence == "" || sequence == "null") {
    return("Error: invalid signal");
  }
  
  // set GPIO to be used for sending the signal
  int kIrLed = 4;

  // initialize IRsend object with GPIO
  IRsend irsend(kIrLed);

  // convert string to array of integers (thanks to https://stackoverflow.com/questions/48409822/convert-a-string-to-an-integer-array)
  int data_num = 0;
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

/**
 * @brief Returns List of saved signals and programs.
 * 
 * @param folder_signals - name of the folder containing the signals
 * 
 * @param folder_programs - name of the folder containing the programs
 * 
 * @return String - String containing all files in the specified folders, separated by a semicolon:\n
 *                 "signal1, signal2, ...;program1, program2, ..."
 * 
 * @details This function scans the signal and program folders for files and returns a String containing all files. It uses LittleFS.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
String get_files(String folder_signals, String folder_programs){

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

/**
 * @brief Checks if a file exists.
 * 
 * @param filename - name of the file to be checked
 * 
 * @return boolean - true if the file exists, false if not
 * 
 * @details This function checks if a file exists in the LittleFS.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
boolean check_if_file_exists(String filename) {
  LittleFS.begin();
  boolean exists = LittleFS.exists(filename);
  LittleFS.end();
  return exists;
}

/**
 * @brief Reads a program file and returns its content as a String.
 * 
 * @param program_name - name of the program to be read
 * 
 * @return String - String containing the program code
 * 
 * @details This function has its reason of existence next to load_json because in the 
 * frontend, after pressing the "edit" button, the program code of the specified 
 * program is displayed in the textarea as a string.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
String read_program(String program_name){

  // declare variables
  String filename = "/programs/" + program_name + ".json";
  String file_content = "";

  // check if file exists
  if (check_if_file_exists(filename) == false){
    return("");
  }

  // read file and save content to String
  LittleFS.begin();
  File file = LittleFS.open(filename, "r");
  while (file.available()) {
    file_content += (char)file.read();
  }

  // cut last 2 characters (newline and EOF)
  file_content = file_content.substring(0, file_content.length() - 2);

  file.close();
  LittleFS.end();

  // return String
  return file_content;
}

/**
 * @brief Controls the LED output.
 * 
 * @param signal - String containing the signal to be send:\n 
 *                "no_signal" - LED blinks 3 times\n
 *                "no_mDNS" - LED blinks 3 times\n
 *                "signal_received" - LED blinks once
 * 
 * 
 * @details This function controls the LED output via codewords to specify the kind 
 * of signal to be send. The LED is connected to GPIO 5 (D1) on the ESP8266 and is 
 * ment as a way to communicate errors to the user and singal when the ESP is ready to 
 * receive a signal.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
void control_led_output(String signal) {

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

/**
 * @brief Checks if a String is alphanumeric.
 * 
 * @param word - String to be checked
 * 
 * @return boolean - true if the String is alphanumeric, false if not
 * 
 * @details This function checks if a String is alphanumeric. Spaces, dashes and
 * underscores are also allowed.
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
boolean check_if_string_is_alphanumeric (String word) {

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