#include "base.h"
#include "helper_functions.h"

boolean recording_workflow(String command_name);
boolean deleting_workflow(String command_name);
boolean sending_workflow(String command_name);

boolean adding_workflow(String progName, String progCode);
boolean playing_workflow(String program);

boolean program_parser(String code);

boolean handle_wait(unsigned long waiting_time);
boolean handle_time(String time_command);
boolean handle_day(String day_command);


// recording workflow for sequences includes recording, converting to JSON and saving to file
boolean recording_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";
  // 2. signal is received:
  String raw_sequence = captureSignal();
  // 3. if sinal was received successfully:
  if (raw_sequence != "No Signal"){
    // 3.1 signal String is serialized to JSON:
    DynamicJsonDocument sequence_JSON = convertToJSON(raw_sequence, command_name);
    // 3.2 JSON is written to file:
    save_json(filename, sequence_JSON);
    return(true);
  }
  return(false);
}


// deleting workflow for sequences and programs deletthe corresponding file
boolean deleting_workflow(String directory, String command_name) {
  // 1. filename is generated:
  String filename = "/" + directory + "/" + command_name + ".json";
  LittleFS.begin();
  // 2. check if file exists:
  if(LittleFS.exists(filename)){
    // 2.1 delete file:
    LittleFS.remove(filename);
    LittleFS.end();
    return(true);
  }
  LittleFS.end();
  return(false);
}


// sending workflow for sequences includes loading the file and sending the command
boolean sending_workflow(String command_name) {
  // 1. filename is generated:
  String filename = "/signals/" + command_name + ".json";

  if (check_if_file_exists(filename) == false) {
    Serial.println("file does not exist");
    return(false);
  }
  // 2. loads Command and saves it in JSON Object
  DynamicJsonDocument doc = load_json(filename);
  // 3. (send command)
  sendCommand(doc);
  return(true);
}


// adding workflow for programs writes the code to file
boolean adding_workflow(String progName, String progCode) {
  // 1. filename is generated:
  String filename = "/programs/" + progName + ".json";
  // 2. if file does not exist:
  LittleFS.begin();

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
  }

  File myfile = LittleFS.open(filename, "w");
  

  if (!myfile) {
    Serial.println(F("Failed to create file"));
    myfile.close();
    LittleFS.end();
    return(false);
  }
  // 3. write code to file:
  myfile.write(progCode.c_str());
  // Close the file
  myfile.close();
  LittleFS.end();
  return(true);
}


// playing workflow for programs includes loading the file, analyizing the codes structure and sending the commands
boolean playing_workflow(String program) {
  // 1. filename is generated:
  String filename = "/programs/" + program + ".json";
  
  LittleFS.begin();
  File myfile = LittleFS.open(filename, "r");

  if (!myfile) {
    Serial.println("Failed to open file for reading");
    return(false);
  }

  // adds " \n" to the end of the file to make sure the last line is read
  String filecontent = myfile.readString() + " \n";
  myfile.close();
  LittleFS.end();
  boolean success = program_parser(filecontent);
  if (success == false) {
    Serial.println("error in program");
    return(false);
  }
  else {
    return(true);
  }
}


// this part is split into a function to be able to call it recursively (for loops)
boolean program_parser(String code){
  String line = "";
  boolean success = true;

  while (code.indexOf("\n") != -1){
    line = code.substring(0, code.indexOf("\n") - 1);
    code = code.substring(code.indexOf("\n") + 1);

    Serial.println("current line: " + line);
    Serial.println("remaining code: " + code);

    if (line.indexOf("play") == 0){
      String command_name = line.substring(5);
      success = sending_workflow(command_name);
    }
    
    else if (line.indexOf("wait") == 0){
      String delay_time = line.substring(5);
      unsigned long delay_time_long = atol(delay_time.c_str());
      success = handle_wait(delay_time_long);
    }

    else if (line.indexOf("0") == 0 || line.indexOf("1") == 0 || line.indexOf("2") == 0) {
      String time_command = line;
      success = handle_time(time_command);
    }

    else if (line.indexOf("monday") == 0 || line.indexOf("tuesday") == 0 || line.indexOf("wednesday") == 0 || line.indexOf("thursday") == 0 || line.indexOf("friday") == 0 || line.indexOf("saturday") == 0 || line.indexOf("sunday") == 0 || line.indexOf("Monday") == 0 || line.indexOf("Tuesday") == 0 || line.indexOf("Wednesday") == 0 || line.indexOf("Thursday") == 0 || line.indexOf("Friday") == 0 || line.indexOf("Saturday") == 0 || line.indexOf("Sunday") == 0) {
      String day_command = line;
      success = handle_day(day_command);
    }

    else if (line.indexOf("skip") == 0) {
      String skip_days = line.substring(5);
      unsigned long days = atoi(skip_days.c_str());
      if (days == 0) {
        Serial.println("invalid number of days");
        return(false);
      }
      if (days > 49) {
        Serial.println("too many days");
        return(false);
      }
      days = days * 86400000; // days in milliseconds
      success = handle_wait(days);
    }

    // code is split in loop part and part that follows (if loop is not repeated infinitely)
    // loop part is executed the given amount of time / or indefinitely
    // after loop is executed, the part after the loop is given back to method
    else if (line.indexOf("loop") == 0) {
      String loop_time = line.substring(5);
      String loop_code = code.substring(0, code.indexOf("end") - 1) + "\n";

      // skip line with "end":
      code = code.substring(code.indexOf("end"));
      code = code.substring(code.indexOf("\n") + 1);

      // if loop is repeated indefinitely, the code after the loop is not executed
      if (loop_time == "inf") {
        while(true) {
          success = program_parser(loop_code);
          if (success == false) {
            Serial.println("error in loop code");
          return(false);
          }
        }
      }
      else {
        unsigned long loop_time_long = atol(loop_time.c_str());
        for (unsigned long i = 0; i < loop_time_long; i++) {
          success = program_parser(loop_code);
          if (success == false) {
            Serial.println("error in loop code");
            return(false);
          }
        }
      }
    }

    if (success == false) {
      Serial.println("playing workflow failed");
      return(false);
    }
  }

  return(true);
}

// handels both the wait command and the skip command
boolean handle_wait(unsigned long waiting_time) {
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
        Serial.println("program was canceled by the user.");
        return(false);
      }
    }
    while(millis() < extra_wait){
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        Serial.println("program was canceled by the user.");
        return(false);
      }
    }
  }

  // no overflow will occur:
  else{
    while(millis() < time_now + waiting_time){
      check_and_update_offset();
      if (digitalRead(Interrupt_Button) == LOW) {
        return(false);
      }
    }
  }
  return(true);
}


// handles the timed workflow which means it waits for a certain time to be reached. 
// The time is given in the format hh:mm:ss sequence_name
boolean handle_time(String time_command) {
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
    Serial.println("file does not exist");
    return(false);
  }

  // loop waits for time to be reached or button to be pressed
  while(true) {
    if (compare_time(time, false) == true) {
      break;
    }
    if (digitalRead(Interrupt_Button) == LOW) {
      Serial.println("program was canceled by the user.");
      return(false);
    }
  }

  sending_workflow(command_name);

  return(true);
}

// handles the daytimed workflow which means it waits for a certain day and time to be reached.
// The time is given in the format day(String) hh:mm:ss sequence_name
boolean handle_day(String day_command) {
  // Wednesday
  String day = day_command.substring(0, day_command.indexOf(" "));
  // 3
  day = weekday_to_num(day);
  if (day == "error") {
    Serial.println("day not found");
    return(false);
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
    Serial.println("file does not exist");
    return(false);
  }

  // loop waits for day to be reached or button to be pressed
  while(true) {
    if (compare_time(day_time, true) == true) {
      break;
    }
    if (digitalRead(Interrupt_Button) == LOW) {
      Serial.println("program was canceled by the user.");
      return(false);
    }
  }

  sending_workflow(command_name);

  return(true);
}
