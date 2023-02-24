/**
 * @file workflows.cpp
 * @author Marc Ubbelohde
 * @brief This file contains high level functions.
 * 
 * @details The functions in this file implement workflows which define the main 
 * functionalities of the device such as recording signals and sending and deleting 
 * signals and programs. Some functions are written here and not in the filesystem.cpp
 * file because they are called directly from the webpage or use other functions 
 * from this file.
 */

#include "workflows.h"

/**
 * @brief This function deletes a file from the LittleFS filesystem.
 * 
 * @param directory - "signals" or "programs"
 * @param name - name of the sequence or program to be deleted
 * @return String - message that will be displayed on the webpage:\n 
 * "success message" - if file was deleted\n
 * "error message" - if file could not be found
 * 
 * @details This function is used to delete signals and programs.
 * 
 * @callgraph This function does not call other functions.
 * 
 * @callergraph 
 */
String deleting_workflow(String directory, String name) {
  
  // generate filename
  String filename = "/" + directory + "/" + name + ".json";

  // start filesystem
  LittleFS.begin();

  // check if file exists and delet if found
  if(LittleFS.exists(filename)){
    LittleFS.remove(filename);
    LittleFS.end();
    return("successfully deleted " + directory + ": " + name);
  }

  // return error message if file was not found
  LittleFS.end();
  return("could not find " + directory + ": " + name);
}

/**
 * @brief This function records and saves a signal.
 * 
 * @param signal_name - name of the sequence to be recorded
 * 
 * @return String - message that will be displayed on the webpage:\n
 * "success message" - if signal was saved\n
 * "error message" - no signal was captured
 * 
 * @details This function records a signal and saves it to a file in the LitteFS with
 * the signals name. (spaces at the end of the signal name will be removed)
 * 
 * @callgraph
 * 
 * @callergraph
 */
String recording_workflow(String signal_name) {
  
  // receive signal sequence
  String raw_sequence = capture_signal();

  // return error message if no signal was captured
  if (raw_sequence == "no_signal"){
    return("failed to record signal");
  }
  Serial.println(raw_sequence);

  // remove spaces at the end of the signal name
  while(signal_name.lastIndexOf(" ") == signal_name.length() - 1){
    signal_name.remove(signal_name.length() - 1);
  }

  // save signal to file
  String message = save_signal(raw_sequence, signal_name);

  // return success message if signal was saved
  if (message == "success"){
    return("successfully recorded signal: " + signal_name);
  }

  // return error message if signal could not be saved
  else {
    return(message);
  }
}

/**
 * @brief This function loads a signal from a file and sends it.
 * 
 * @param signal_name - name of the sequence to be sent
 * 
 * @return String - message that will be displayed on the webpage:\n
 * "success message" - if file was found and command was sent\n
 * "error message" - if file could not be found
 * 
 * @details
 * 
 * @callgraph
 * 
 * @callergraph
 */
String sending_workflow(String signal_name) {
  // generate filename
  String filename = "/signals/" + signal_name + ".json";

  // check if file exists
  if (check_if_file_exists(filename) == false) {
    return("could not find signal: " + signal_name);
  }

  // load signal from file
  DynamicJsonDocument doc = load_json(filename);

  // send signal

  String message = send_signal(doc);

  if (message.indexOf("success") != -1) {
    return("successfully sent signal: " + signal_name);
  }
  else {
    return(message);
  }
}

/**
 * @brief This function creates a file with the programs name and writes the code to it.
 * 
 * @param program_name - name of the program to be added
 * 
 * @param program_code - code of the program to be added
 * 
 * @return String - message that will be displayed on the webpage:\n
 * "success message" - if file was created and code was written\n
 * "error message" - if file could not be created
 * 
 * @details
 * 
 * @callgraph
 * 
 * @callergraph
 */
String adding_workflow(String program_name, String program_code) {

  // generate filename
  String filename = "/programs/" + program_name + ".txt";

  // start filesystem
  LittleFS.begin();

  // check if file exists and delete if found (to overwrite old file)
  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  // create or recreate file
  File myfile = LittleFS.open(filename, "w");
  
  // return error message if file could not be created
  if (!myfile) {
    myfile.close();
    LittleFS.end();
    return("failed to create file");
  }

  // write code to file
  myfile.write(program_code.c_str());
  myfile.close();
  LittleFS.end();

  return("successfully saved program: " + program_name);
}

/**
 * @brief This function loads a program from a file and hands it to the program_parser.
 * 
 * @param program_name - name of the program to be played
 * 
 * @return String - message that will be displayed on the webpage:\n
 * "success message" - if file was found and program was played successfully\n
 * "error message" - if file could not be found or if in one of the commands an error occured (error message gets passed by program_parser)
 * 
 * @details This function loads a program from a file and hands it to the program_parser.
 * The program_parser then sends the commands and returns a message when the execution
 * of the program finished.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String playing_workflow(String program_name) {

  // generate filename
  String filename = "/programs/" + program_name + ".txt";

  // check if file exists 
  if (check_if_file_exists(filename) == false) {
    return("could not find program: " + program_name);
  }

  // read file and add " \n" to the end of the file to be able to read lines more easily in loop
  String programcode = read_program(program_name);
  programcode += " \n";

  // hand program to parser and catch error message
  String message = program_parser(programcode);
  
  // return error message if error occured
  if (message.indexOf("success") == -1){
    return(message);
  }

  else {
    return("successfully played program: " + program_name);
  }
}

/**
 * @brief This function parses the code of a program line by line and executes the commands.
 * 
 * @param code - code of the program to be parsed
 * 
 * @return String - message that will be displayed on the webpage:\n
 * "success message" - if program was played successfully\n
 * "error message" - if in one of the commands an error occured an command specific error message is returned
 * 
 * @details This function parses the code of a program line by line and executes the commands.
 * It was necessary to split the parser from the playing_workflow function to be able
 * to call it recursively (for loops). Each line is searched for command specific keywords
 * and the corresponding command handler is called.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String program_parser(String code){

  // initialize variables
  String line = "";
  String error_message = "success";
  String command = "invalid";
  
  // declare matchstate for regex
  MatchState REGEX;

  // loop through code line by line (until no more newlines are found)
  while (code.indexOf("\n") != -1){
    // get current line and remaining code
    line = code.substring(0, code.indexOf("\n") - 1);
    code = code.substring(code.indexOf("\n") + 1);    

    // check with regex which command was found
    char * line_as_char_array = (char *)line.c_str();
    REGEX.Target(line_as_char_array);

    // check which command was found
    if (REGEX.Match("play [a-zA-Z0-9\\s\\-\\_]+") == REGEXP_MATCHED){
      command = "play";
    }
    else if (REGEX.Match("wait [0-9]+") == REGEXP_MATCHED){
      command = "wait";
    }
    else if (REGEX.Match("^(0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9] [a-zA-Z0-9\\s\\-\\_]+") == REGEXP_MATCHED){
      command = "time";
    }
    else if (REGEX.Match("^(monday|tuesday|wednesday|thursday|friday|saturday|sunday|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday) (0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9] [a-zA-Z0-9\\s\\-\\_]+") == REGEXP_MATCHED){
      command = "day";
    }
    else if (REGEX.Match("skip [0-9]+") == REGEXP_MATCHED){
      command = "skip";
    }
    else if (REGEX.Match("loop [0-9]+") == REGEXP_MATCHED || REGEX.Match("loop inf") == REGEXP_MATCHED){
      command = "loop";
    }
    else if (line == ""){ // TODO: check why regex didnt work
      command = "empty";
    }
    else {
      error_message = "invalid command: " + line;
      command = "invalid";
    }

    if (command != "invalid"){

      // play command was found
      if (command == "play"){
        // slice command name from line
        String command_name = line.substring(5);

        // send signal
        error_message = sending_workflow(command_name);

        Serial.println("-");
      }
      
      // wait command was found
      else if (command == "wait"){
        // slice delay time from line
        String delay_time = line.substring(5);
        // typecast
        unsigned long delay_time_long = atol(delay_time.c_str());
        // check if number is not too high
        if (delay_time_long == 0 && delay_time != "0"){
          error_message = "invalid delay time";
        }
        else {
          // wait for delay time
          error_message = handle_wait_command(delay_time_long);
        }
      }

      // time command was found
      else if (command == "time") {
        // whole line is passed to handler
        String time_command = line;
        error_message = handle_times_commands(time_command, false);
      }

      // day command was found
      else if (command == "day") {
        // whole line is passed to handler
        String day_command = line;
        error_message = handle_times_commands(day_command, true);
      }

      // skip command was found
      else if (command == "skip") {
        // slice days from line
        String skip_days = line.substring(5);
        unsigned long days = atoi(skip_days.c_str());

        // check if amount of days is valid (49 days in ms is limit for unsigned long)
        if (days == 0) {
          return("failed to execute skip command: no days given");
        }
        if (days > 49) {
          return("failed to execute skip command: too many days given");
        }

        // convert days to milliseconds
        days = days * 86400000; 

        // pass to handler
        error_message = handle_wait_command(days);
      }

      // code is split in loop part and part that follows (if loop is not repeated infinitely)
      // loop part is executed the given amount of time / or indefinitely
      // after loop is executed, the part after the loop is given back to method

      // loop command was found
      else if (command == "loop") {
        // slice loop time from line
        String loop_time = line.substring(5);
        
        // track loop lines
        int loop_counter = 1;

        // variable for current line
        String loop_line = "";

        // variable for loop code
        String loop_code = "";

        // go through lines and find end of loop by adding 1 to loop_counter for every loop and 
        // subtracting 1 for every end if the counter reches 0, the end of the loop was found
        while(loop_counter != 0) {
          // update loop line and remaining code
          loop_line = code.substring(0, code.indexOf("\n"));
          code = code.substring(code.indexOf("\n") + 1);

          // check if "loop" or "end" was found and adjust counter
          if (loop_line.indexOf("loop") == 0) {
            loop_counter++;
          }
          else if (loop_line.indexOf("end") == 0) {
            loop_counter--;
          }

          // add line to loop code if it is not the last line of the loop
          if (loop_counter != 0) {
            loop_code += loop_line + "\n";
          }
        }

        // loop is repeated indefinitely
        if (loop_time == "inf") {
          while(true) {
            // execute loop code with parser indefinitely
            error_message = program_parser(loop_code);

            // check for error since end of this parser is not reached
            if (error_message.indexOf("success") == -1) {
              return(error_message);
            }
          }
        }

        // loop is repeated a certain amount of times
        else {
          // convert loop time to unsigned long
          unsigned long loop_time_long = atol(loop_time.c_str());

          // loop is repeated the given amount of times
          for (unsigned long i = 0; i < loop_time_long; i++) {
            // code is passed again to parser and checked for errors
            error_message = program_parser(loop_code);
            if (error_message.indexOf("success") == -1) {
              return(error_message);
            }
          }
        }
      }
    }

    // at the end the parser checks if the current command wrote to the error message
    if (error_message.indexOf("success") == -1) {
      return(error_message);
    }
  }

  return("success");
}

/**
 * @brief This function waits a certain amount of time.
 * 
 * @param waiting_time - time to wait in milliseconds
 * 
 * @return String - message that will be displayed on the webpage:\n
 *                 "success message" - if command was executed successfully\n
 *                "error message" - if command was interrupted by the user
 * 
 * @details This function waits a certain amount of time. It is used for the wait and skip command.
 *         It is necessary to check beforehand if a millis() overflow will occur during the waiting time.
 *        If an overflow will occur, the function will first calculate the time it will have to
 *       wait after the overflow occurs, then it will wait until the overflow occurs and waits
 *      the remaining time. The function also checks if the user pressed the interrupt button.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String handle_wait_command(unsigned long waiting_time) {

  // initialize variables:
  unsigned long time_now = millis();
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);

  // check if overflow will occur (if yes:)
  if (time_now + waiting_time < time_now) {
    Serial.println("overflow will occur in wait command");

    // calculate time to wait after overflow
    unsigned long extra_wait = time_now + waiting_time;

    // wait for overflow to occur (buffer of 1s not to miss it)
    while(millis() > 1000) {
      // update time
      check_and_update_offset();

      // check if user pressed interrupt button
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }

    // wait the extra time (after overflow)
    while(millis() < extra_wait){
      // update time
      check_and_update_offset();

      // check if user pressed interrupt button
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
  }

  // no overflow will occur
  else{
    // wait the given time
    while(millis() < time_now + waiting_time){
      check_and_update_offset(); // no trusting my own code haha

      // check if user pressed interrupt button
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
  }
  return("success");
}

/**
 * @brief This function executes timed commands.
 * 
 * @param command - String command:\n
 *                 "weekday hh:mm:ss signal_name" - if day_included is true\n
 *                "hh:mm:ss signal_name" - if day_included is false
 * 
 * @param day_included - true if day is included in command, false if not
 * 
 * @return String - message that will be displayed on the webpage:\n
 *                "success message" - if command was executed successfully\n
 * 
 * @details This function waits until a certain day and/or time is reached and then executes the given signal.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String handle_times_commands(String command, boolean day_included) {

  // variable declaration
  String day = "";
  String command_name = "";
  String timestamp = "";
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);
  
  // input command format: "day hh:mm:ss command_name" (day command)
  if (day_included == true) {
    // extract weekday
    day = command.substring(0, command.indexOf(" "));

    // convert weekday to number
    day = weekday_to_num(day);

    // check if day was converted correctly
    if (day == "error") {
      return("weekday in command " + command + " is not valid");
    }

    // extract time and command name (hh:mm:ss command_name)
    String time_command = command.substring(command.indexOf(" ") + 1);
    
    // extract time (hh:mm:ss)
    String time = time_command.substring(0, time_command.indexOf(" "));

    // extract command name (command_name)
    command_name = time_command.substring(time_command.indexOf(" ") + 1);

    // create timestamp (hh:mm:ss day)
    timestamp = time + " " + day;
  }

  // input command format: "hh:mm:ss command_name" (time command)
  else {
    // extract time (hh:mm:ss)
    timestamp = command.substring(0, command.indexOf(" "));

    // extract command name (command_name)
    String command_name = command.substring(command.indexOf(" ") + 1);
  }
  
  // generate filename
  String filename = "/signals/" + command_name + ".json";

  // checks beforehand if file exists (not to waste time)
  if (check_if_file_exists(filename) == false) {
    return("signal in command " + command + " not found");
  }

  // loop waits for day and/or time to be reached or button to be pressed
  while(true) {
    if (compare_time(timestamp, day_included) == true) {
      break;
    }
    if (digitalRead(Interrupt_Button) == LOW) {
      return("program was canceled by the user.");
    }
  }

  // send signal
  sending_workflow(command_name);
  return("success");
}
