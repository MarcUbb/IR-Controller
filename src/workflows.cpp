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
  // 1. filename is generated:
  String filename = "/" + directory + "/" + command_name + ".json";
  LittleFS.begin();
  // 2. check if file exists:
  if(LittleFS.exists(filename)){
    // 2.1 delete file:
    LittleFS.remove(filename);
    LittleFS.end();
    return("successfully deleted " + directory + ": " + command_name);
  }
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
  - convert_to_JSON (in src/filesystem.cpp) to convert the signal to JSON and add the name
  - save_json (in src/filesystem.cpp) to save the JSON to a file

  called by:
  - handle_form (in src/main.cpp) to record a sequence
  */
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";
  // 2. signal is received:
  String raw_sequence = capture_signal();
  // 3. if sinal was received successfully:
  if (raw_sequence == "no_signal"){
    return("failed to record signal");
  }
  // 3.1 signal String is serialized to JSON:
  DynamicJsonDocument sequence_JSON = convert_to_JSON(raw_sequence, command_name);
  // 3.2 JSON is written to file:
  save_json(filename, sequence_JSON);
  return("sucessfully saved signal: " + command_name);
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
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";

  if (check_if_file_exists(filename) == false) {
    return("could not find command: " + command_name);
  }
  // 2. loads Command and saves it in JSON Object
  DynamicJsonDocument doc = load_json(filename);
  // 3. (send command)
  send_signal(doc);
  return("successfully sent command: " + command_name);
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
  // 1. filename is generated:
  String filename = "/programs/" + program_name + ".json";
  // 2. if file does not exist:
  LittleFS.begin();

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  File myfile = LittleFS.open(filename, "w");
  

  if (!myfile) {
    myfile.close();
    LittleFS.end();
    return("failed to create file");
  }
  // 3. write code to file:
  myfile.write(program_code.c_str());
  // Close the file
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
  // 1. filename is generated:
  String filename = "/programs/" + program_name + ".json";
  
  if (check_if_file_exists(filename) == false) {
    return("could not find program: " + program_name);
  }

  LittleFS.begin();
  File myfile = LittleFS.open(filename, "r");

  if (!myfile) {
    return("Failed to open file for reading");
  }

  // adds " \n" to the end of the file to make sure the last line is read
  String filecontent = myfile.readString() + " \n";
  myfile.close();
  LittleFS.end();
  String message = program_parser(filecontent);
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

  String line = "";
  String error_message = "";

  while (code.indexOf("\n") != -1){
    line = code.substring(0, code.indexOf("\n") - 1);
    code = code.substring(code.indexOf("\n") + 1);

    Serial.println("current line: " + line);
    Serial.println("remaining code: " + code);

    if (line.indexOf("play") == 0){
      String command_name = line.substring(5);
      error_message = sending_workflow(command_name);
    }
    
    else if (line.indexOf("wait") == 0){
      String delay_time = line.substring(5);
      unsigned long delay_time_long = atol(delay_time.c_str());
      error_message = handle_wait_command(delay_time_long);
    }

    else if (line.indexOf("0") == 0 || line.indexOf("1") == 0 || line.indexOf("2") == 0) {
      String time_command = line;
      error_message = handle_time_command(time_command);
    }

    else if (line.indexOf("monday") == 0 || line.indexOf("tuesday") == 0 || line.indexOf("wednesday") == 0 || line.indexOf("thursday") == 0 || line.indexOf("friday") == 0 || line.indexOf("saturday") == 0 || line.indexOf("sunday") == 0 || line.indexOf("Monday") == 0 || line.indexOf("Tuesday") == 0 || line.indexOf("Wednesday") == 0 || line.indexOf("Thursday") == 0 || line.indexOf("Friday") == 0 || line.indexOf("Saturday") == 0 || line.indexOf("Sunday") == 0) {
      String day_command = line;
      error_message = handle_day_command(day_command);
    }

    else if (line.indexOf("skip") == 0) {
      String skip_days = line.substring(5);
      unsigned long days = atoi(skip_days.c_str());
      if (days == 0) {
        return("failed to execute skip command: no days given");
      }
      if (days > 49) {
        return("failed to execute skip command: too many days given");
      }
      days = days * 86400000; // days in milliseconds
      error_message = handle_wait_command(days);
    }

    // code is split in loop part and part that follows (if loop is not repeated infinitely)
    // loop part is executed the given amount of time / or indefinitely
    // after loop is executed, the part after the loop is given back to method
    else if (line.indexOf("loop") == 0) {
      String loop_time = line.substring(5);
      String loop_code = code.substring(0, code.lastIndexOf("end") - 1) + "\n";

      // skip line with "end":
      code = code.substring(code.lastIndexOf("end"));
      code = code.substring(code.indexOf("\n") + 1);

      // if loop is repeated indefinitely, the code after the loop is not executed
      if (loop_time == "inf") {
        while(true) {
          error_message = program_parser(loop_code);
          if (error_message.indexOf("success") == -1) {
            return(error_message);
          }
        }
      }
      else {
        unsigned long loop_time_long = atol(loop_time.c_str());
        for (unsigned long i = 0; i < loop_time_long; i++) {
          error_message = program_parser(loop_code);
          if (error_message.indexOf("success") == -1) {
            return(error_message);
          }
        }
      }
    }

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
  unsigned long time_now = millis();
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);

  // overflow will occur:
  if (time_now + waiting_time < time_now) {
    Serial.println("overflow will occur in wait command");
    unsigned long extra_wait = time_now + waiting_time;

    // wait for overflow to occur (buffer of 1s not to miss it):
    while(millis() > 1000) {
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
    // wait the extra time:
    while(millis() < extra_wait){
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
  }

  // no overflow will occur:
  else{
    while(millis() < time_now + waiting_time){
      check_and_update_offset(); // no trusting my own code haha
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
  }
  return("success");
}

// TODO: merge handle_time_command and handle_day_command into one function
String handle_time_command(String time_command) {
  /*
  parameters:
    String time_command:
      time to wait for in format "hh:mm:ss signal_name"

  return:
    String:
      message that will be displayed on the webpage:
        "success message" - if command was executed successfully
        "error message" - if command was interrupted by the user

  description:
  This function waits until a certain time is reached and then sends the given signal.

  calls:
  - check_if_file_exists (in src/filesystem.cpp) to check if the file exists
  - compare_time (in src/time_management.cpp) to compare the given time with the current time
  - sending_workflow (in src/workflows.cpp) to send the given signal

  called by:
  - program_parser (in src/workflows.cpp) to execute time command
  */
  // 12:00:13
  String time = time_command.substring(0, time_command.indexOf(" "));
  // Sequence1
  String command_name = time_command.substring(time_command.indexOf(" ") + 1);
  // /signals/Sequence1.json
  String filename = "/signals/" + command_name + ".json";
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);

  // checks beforehand if file exists (not to waste time)
  if (check_if_file_exists(filename) == false) {
    return("command in command " + command_name + " not found");
  }

  // loop waits for time to be reached or button to be pressed
  while(true) {
    if (compare_time(time, false) == true) {
      break;
    }
    if (digitalRead(Interrupt_Button) == LOW) {
      return("program was canceled by the user.");
    }
  }

  sending_workflow(command_name);
  return("success");
}


String handle_day_command(String day_command) {
  /*
  parameters:
    String day_command:
      day and time to wait for in format "day hh:mm:ss signal_name"

  return:
    String:
      message that will be displayed on the webpage:
        "success message" - if command was executed successfully
        "error message" - if command was interrupted by the user

  description:
  This function waits until a certain day and time is reached and then executes the given signal.

  calls:
  - check_if_file_exists (in src/filesystem.cpp) to check if the file exists
  - weekday_to_num (in src/time_management.cpp) to convert the written day to its numerical 
    representation
  - compare_time (in src/time_management.cpp) to compare the given time with the current time
  - sending_workflow (in src/workflows.cpp) to send the given signal

  called by:
  - program_parser (in src/workflows.cpp) to execute day command
  */
  // Wednesday
  String day = day_command.substring(0, day_command.indexOf(" "));
  // 3
  day = weekday_to_num(day);
  if (day == "error") {
    return("could not identify day in command");
  }
  // 12:00:13 Sequence1
  String time_command = day_command.substring(day_command.indexOf(" ") + 1);
  // 12:00:13
  String time = time_command.substring(0, time_command.indexOf(" "));
  // Sequence1
  String command_name = time_command.substring(time_command.indexOf(" ") + 1);
  // 12:00:13 3
  String day_time = time + " " + day;;
  // /signals/Sequence1.json
  String filename = "/signals/" + command_name + ".json";
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);

  // checks beforehand if file exists (not to waste time)
  if (check_if_file_exists(filename) == false) {
    return("command in command " + day_command + " not found");
  }

  // loop waits for day to be reached or button to be pressed
  while(true) {
    if (compare_time(day_time, true) == true) {
      break;
    }
    if (digitalRead(Interrupt_Button) == LOW) {
      return("program was canceled by the user.");
    }
  }

  sending_workflow(command_name);
  return("success");
}
