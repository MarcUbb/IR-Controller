#include "base.h"
#include "helper_functions.h"

String recording_workflow(String command_name);
String deleting_workflow(String command_name);
String sending_workflow(String command_name);

String adding_workflow(String progName, String progCode);
String playing_workflow(String program);

String program_parser(String code);

String handle_wait(unsigned long waiting_time);
String handle_time(String time_command);
String handle_day(String day_command);


// recording workflow for sequences includes recording, converting to JSON and saving to file
String recording_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";
  // 2. signal is received:
  String raw_sequence = captureSignal();
  // 3. if sinal was received successfully:
  if (raw_sequence == "No Signal"){
    return("failed to record signal");
  }
  // 3.1 signal String is serialized to JSON:
  DynamicJsonDocument sequence_JSON = convertToJSON(raw_sequence, command_name);
  // 3.2 JSON is written to file:
  save_json(filename, sequence_JSON);
  return("sucessfully saved signal: " + command_name);
}


// deleting workflow for sequences and programs deletthe corresponding file
String deleting_workflow(String directory, String command_name) {
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


// sending workflow for sequences includes loading the file and sending the command
String sending_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";

  if (check_if_file_exists(filename) == false) {
    return("could not find command: " + command_name);
  }
  // 2. loads Command and saves it in JSON Object
  DynamicJsonDocument doc = load_json(filename);
  // 3. (send command)
  sendCommand(doc);
  return("successfully sent command: " + command_name);
}


// adding workflow for programs writes the code to file
String adding_workflow(String program_name, String program_code) {
  // 1. filename is generated:
  String filename = "/programs/" + program_name + ".json";
  // 2. if file does not exist:
  LittleFS.begin();

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  File myfile = LittleFS.open(filename, "w");
  

  if (!myfile) {
    Serial.println("Failed to create file");
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


// playing workflow for programs includes loading the file, analyizing the codes structure and sending the commands
String playing_workflow(String program) {
  // 1. filename is generated:
  String filename = "/programs/" + program + ".json";
  
  if (check_if_file_exists(filename) == false) {
    return("could not find program: " + program);
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
    return("successfully played program: " + program);
  }
}


// this part is split into a function to be able to call it recursively (for loops)
String program_parser(String code){
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
      error_message = handle_wait(delay_time_long);
    }

    else if (line.indexOf("0") == 0 || line.indexOf("1") == 0 || line.indexOf("2") == 0) {
      String time_command = line;
      error_message = handle_time(time_command);
    }

    else if (line.indexOf("monday") == 0 || line.indexOf("tuesday") == 0 || line.indexOf("wednesday") == 0 || line.indexOf("thursday") == 0 || line.indexOf("friday") == 0 || line.indexOf("saturday") == 0 || line.indexOf("sunday") == 0 || line.indexOf("Monday") == 0 || line.indexOf("Tuesday") == 0 || line.indexOf("Wednesday") == 0 || line.indexOf("Thursday") == 0 || line.indexOf("Friday") == 0 || line.indexOf("Saturday") == 0 || line.indexOf("Sunday") == 0) {
      String day_command = line;
      error_message = handle_day(day_command);
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
      error_message = handle_wait(days);
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

// handels both the wait command and the skip command
String handle_wait(unsigned long waiting_time) {
  unsigned long time_now = millis();
  const int Interrupt_Button = 12;
  pinMode(Interrupt_Button, INPUT_PULLUP);

  // overflow will occur:
  if (time_now + waiting_time < time_now) {
    Serial.println("overflow will occur in wait command");
    unsigned long extra_wait = time_now + waiting_time;

    // wait for overflow to occur:
    while(millis() != 0) {
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
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
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        return("program was canceled by the user.");
      }
    }
  }
  return("success");
}


// handles the timed workflow which means it waits for a certain time to be reached. 
// The time is given in the format hh:mm:ss sequence_name
String handle_time(String time_command) {
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

// handles the daytimed workflow which means it waits for a certain day and time to be reached.
// The time is given in the format day(String) hh:mm:ss sequence_name
String handle_day(String day_command) {
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
