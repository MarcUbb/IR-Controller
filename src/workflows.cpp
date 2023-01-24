#include "workflows.h"

/*
In this file you find high level functions which are called directly by the website handlers.
*/

String deleting_workflow(String directory, String command_name) {
  /*
  parameters:
    String directory:
      "signals" - for deleting a sequence
      "programs" - for deleting a program
    String command_name:
      name of the sequence or program to be deleted
  
  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if file was deleted
        "error message" - if file could not be found
  
  description:
  This function deletes a file from the LittleFS filesystem. 
  It is used to delete signals and programs.

  calls:
   ---

  called by:
  - handle_form (in src/main.cpp) to delete a sequence or program
  */
  
  // generate filename
  String filename = "/" + directory + "/" + command_name + ".json";

  // start filesystem
  LittleFS.begin();

  // check if file exists and delet if found
  if(LittleFS.exists(filename)){
    LittleFS.remove(filename);
    LittleFS.end();
    return("successfully deleted " + directory + ": " + command_name);
  }

  // return error message if file was not found
  LittleFS.end();
  return("could not find " + directory + ": " + command_name);
}


String recording_workflow(String command_name) {
  /*
  parameters:
    String command_name:
      name of the sequence to be recorded
  
  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if signal was saved
        "error message" - no signal was captured
  
  description:
  This function records a signal and saves it to a file with the signals name 
  in the LittleFS filesystem.

  calls:
  - capture_signal (in src/filesystem.cpp) to capture a signal
  - save_signal (in src/filesystem.cpp) to convert the signal to JSON and add the name
  - save_json (in src/filesystem.cpp) to save the JSON to a file

  called by:
  - handle_form (in src/main.cpp) to record a sequence
  */
  
  // receive signal sequence
  String raw_sequence = capture_signal();

  // return error message if no signal was captured
  if (raw_sequence == "no_signal"){
    return("failed to record signal");
  }
  Serial.println(raw_sequence);

  // save signal to file
  String message = save_signal(raw_sequence, command_name);

  // return success message if signal was saved
  if (message == "success"){
    return("successfully recorded signal: " + command_name);
  }

  // return error message if signal could not be saved
  else {
    return(message);
  }
}


String sending_workflow(String command_name) {
  /*
  parameters:
    String command_name:
      name of the sequence to be sent
  
  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if file was found and command was sent
        "error message" - if file could not be found

  description:
  This function loads a signal from a file and sends it.

  calls:
  - check_if_file_exists (in src/filesystem.cpp) to check if file exists
  - load_json (in src/filesystem.cpp) to load the signal from the file
  - send_signal (in src/filesystem.cpp) to send the signal

  called by:
  - handle_form (in src/main.cpp) to send a signal
  */
 
  // generate filename
  String filename = "/signals/" + command_name + ".json";

  // check if file exists
  if (check_if_file_exists(filename) == false) {
    return("could not find command: " + command_name);
  }

  // load signal from file
  DynamicJsonDocument doc = load_json(filename);

  // send signal

  String message = send_signal(doc);

  if (message.indexOf("success") != -1) {
    return("successfully sent command: " + command_name);
  }
  else {
    return(message);
  }
}


String adding_workflow(String program_name, String program_code) {
  /*
  parameters:
    String program_name:
      name of the program to be added
    String program_code:
      code of the program to be added

  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if file was created and code was written
        "error message" - if file could not be created
  
  description:
  This function creates a file with the programs name and writes the code to it.

  calls:
  - save_json (in src/filesystem.cpp) to save the JSON to a file

  called by:
  - handle_form (in src/main.cpp) to add a program
  */

  // generate filename
  String filename = "/programs/" + program_name + ".json";

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


String playing_workflow(String program_name) {
  /*
  parameters:
    String program_name:
      name of the program to be played

  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if file was found and program was played successfully
        "error message" - if file could not be found or if in one of the commands 
        an error occured (error message gets passed by program_parser)

  description:
  This function loads a program from a file and hands it to the program_parser.
  The program_parser then sends the commands and returns a message when the execution
  of the program finished.

  calls:
  - check_if_file_exists (in src/filesystem.cpp) to check if file exists
  - program_parser (in src/workflows.cpp) to parse the program and send the commands

  called by:
  - handle_form (in src/main.cpp) to play a program
  */

  // generate filename
  String filename = "/programs/" + program_name + ".json";

  // check if file exists 
  if (check_if_file_exists(filename) == false) {
    return("could not find program: " + program_name);
  }

  // load program from file
  LittleFS.begin();
  File myfile = LittleFS.open(filename, "r");

  // return error message if file could not be opened
  if (!myfile) {
    return("Failed to open file for reading");
  }

  // read file and add " \n" to the end of the file to be able to read lines more easily in loop
  String filecontent = myfile.readString() + " \n";
  myfile.close();
  LittleFS.end();

  // hand program to parser and catch error message
  String message = program_parser(filecontent);
  
  // return error message if error occured
  if (message.indexOf("success") == -1){
    return(message);
  }

  else {
    return("successfully played program: " + program_name);
  }
}


String program_parser(String code){
  /*
  parameters:
    String code:
      code of the program to be parsed

  returns:
    String:
      message that will be displayed on the webpage:
        "success message" - if program was played successfully
        "error message" - if in one of the commands an error occured an command 
          specific error message is returned

  description:
  This function parses the code of a program line by line and executes the commands.
  It was necessary to split the parser from the playing_workflow function to be able
  to call it recursively (for loops). Each line is searched for command specific keywords
  and the corresponding command handler is called.

  calls:
  - sending_workflow (in src/workflows.cpp) to send a signal (for play command)
  - handle_wait_command (in src/workflows.cpp) to wait a certain amount of time (for wait and skip commands)
  - program_parser (in src/workflows.cpp) to execute code that is inside a loop (for loop command)
  - handle_time_command (in src/workflows.cpp) to wait until a certain time (for time command)
  - handle_day_command (in src/workflows.cpp) to wait until a certain time and day (for day command)
  */

  // initialize variables
  String line = "";
  String error_message = "";

  // loop through code line by line (until no more newlines are found)
  while (code.indexOf("\n") != -1){
    // get current line and remaining code
    line = code.substring(0, code.indexOf("\n") - 1);
    code = code.substring(code.indexOf("\n") + 1);

    //Serial.println("current line: " + line);
    //Serial.println("remaining code: " + code);

    // play command was found
    if (line.indexOf("play") == 0){
      // slice command name from line
      String command_name = line.substring(5);

      // send signal
      error_message = sending_workflow(command_name);
    }
    
    // wait command was found
    else if (line.indexOf("wait") == 0){
      // slice delay time from line
      String delay_time = line.substring(5);
      unsigned long delay_time_long = atol(delay_time.c_str());

      // wait for delay time
      error_message = handle_wait_command(delay_time_long);
    }

    // time command was found
    else if (line.indexOf("0") == 0 || line.indexOf("1") == 0 || line.indexOf("2") == 0) {
      // whole line is passed to handler
      String time_command = line;
      error_message = handle_times_commands(time_command, false);
    }

    // day command was found
    else if (line.indexOf("monday") == 0 || line.indexOf("tuesday") == 0 || line.indexOf("wednesday") == 0 || line.indexOf("thursday") == 0 || line.indexOf("friday") == 0 || line.indexOf("saturday") == 0 || line.indexOf("sunday") == 0 || line.indexOf("Monday") == 0 || line.indexOf("Tuesday") == 0 || line.indexOf("Wednesday") == 0 || line.indexOf("Thursday") == 0 || line.indexOf("Friday") == 0 || line.indexOf("Saturday") == 0 || line.indexOf("Sunday") == 0) {
      // whole line is passed to handler
      String day_command = line;
      error_message = handle_times_commands(day_command, true);
    }

    // skip command was found
    else if (line.indexOf("skip") == 0) {
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
    else if (line.indexOf("loop") == 0) {
      // slice loop time from line
      String loop_time = line.substring(5);

      // slice loop code from code
      String loop_code = code.substring(0, code.lastIndexOf("end") - 1) + "\n";

      // slice remaining code from code
      code = code.substring(code.lastIndexOf("end"));
      code = code.substring(code.indexOf("\n") + 1);

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

    // at the end the parser checks if the current command wrote to the error message
    if (error_message.indexOf("success") == -1) {
      return(error_message);
    }
  }

  return("success");
}


String handle_wait_command(unsigned long waiting_time) {
  /*
  parameters:
    unsigned long waiting_time:
      time to wait in milliseconds
  
  return:
    String:
      message that will be displayed on the webpage:
        "success message" - if command was executed successfully
        "error message" - if command was interrupted by the user

  description:
  This function waits a certain amount of time. It is used for the wait and skip command.
  It is necessary to check beforehand if a millis() overflow will occur during the waiting time.
  If an overflow will occur, the function will first calculate the time it will have to 
  wait after the overflow occurs, then it will wait until the overflow occurs and waits 
  the remaining time. The function also checks if the user pressed the interrupt button.

  calls:
  - check_and_update_offset (in src/time.cpp) to check if the offset needs to be updated

  called by:
  - program_parser (in src/workflows.cpp) to execute wait or skip command
  */

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


String handle_times_commands(String command, boolean day_included) {
  /*
  parameters:
    String command:
      "day hh:mm:ss signal_name" - if day_included is true
      "hh:mm:ss signal_name" - if day_included is false

  return:
    String:
      message that will be displayed on the webpage:
        "success message" - if command was executed successfully
        "error message" - if command was interrupted by the user

  description:
  This function waits until a certain day and/or time is reached and then executes the given signal.

  calls:
  - check_if_file_exists (in src/filesystem.cpp) to check if the file exists
  - weekday_to_num (in src/time_management.cpp) to convert the written day to its numerical 
    representation
  - compare_time (in src/time_management.cpp) to compare the given time with the current time
  - sending_workflow (in src/workflows.cpp) to send the given signal

  called by:
  - program_parser (in src/workflows.cpp) to execute day command
  */

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
      return("could not identify day in command");
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
