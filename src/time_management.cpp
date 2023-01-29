#include "base.h"


String weekday_to_num (String weekday){
  /*
  parameters:
    String weekday:
      weekday as a String

  returns:
    String:
      weekday as a number:
        "Monday" - "1"
        "Tuesday" - "2"
        "Wednesday" - "3"
        "Thursday" - "4"
        "Friday" - "5"
        "Saturday" - "6"
        "Sunday" - "0"
        error - "error"

  description:
  converts a weekday String to a weekday number

  calls:
  ---

  called by:
  - handle_day_command (in src/workflows.cpp) used to convert user input to format 
    in which the time is saved.
  */

  // uppercase starting letters are also accepted
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


boolean compare_time (String time, boolean weekday_included) {
  /*
  parameters:
    String time:
      time in format "hh:mm:ss" or "hh:mm:ss weekday"
    boolean weekday_included:
      true if weekday is included in time, false if not

  returns:
    boolean:
      true if time is equal to current time, false if not

  description:
  This elementary function checks if the current time is equal to the time in the program. 
  It is used in timed programs and handles millis() overflow. The function has a delay of 500ms 
  to reduce the number of operations inside the while(true) loop.

  calls:
  - check_and_update_offset (in src/time_management.cpp) to check if millis() overflowed and update time if necessary
  - get_current_time (in src/time_management.cpp) to get the current time from the LittleFS

  called by:
  - handle_time_command (in src/workflows.cpp) used to check if the current time is equal to the time in the program
  - handle_day_command (in src/workflows.cpp) used to check if the current time is equal to the time in the program
  */
  
  // delay to reduce the number of operations inside the while(true) loop
  delay(500);

  // check for millis() overflow
  check_and_update_offset();

  // get current time
  String current_time = get_current_time();

  // compare time for day-command
  if (weekday_included == true) {
    return(time == current_time);
  }
  // compare time for time-command
  else {
    return(time == current_time.substring(0, current_time.indexOf(" ")));
  }
}


void update_time(String time, boolean AP_mode){
  /*
  parameters:
    int time:
      String data from user in format "weekday hh:mm:ss timezone"
    boolean AP_mode:
      true if the device is in AP mode, false if not
  
  returns:
    void

  description:
  This function is called when the user presses the "sync" button on the website. 
  It updates only the timezone to the LittleFS since the time from the NTP request is more precise
  than the time from the user.

  calls:
  - load_json (in src/filesystem.cpp) to load the time from the LittleFS
  - save_json (in src/filesystem.cpp) to save the time to the LittleFS

  called by:
  - handle_time (in src/main.cpp) used to save the timezone to the LittleFS
  */

  // extrace information from String
  String time_timezone = time.substring(time.indexOf(" ") + 1);
  int weekday = time.substring(0, time.indexOf(" ")).toInt();
  String time_only = time_timezone.substring(0, time_timezone.indexOf(" "));
  int timezone = time_timezone.substring(time_timezone.indexOf(" ") + 1).toInt();
  timezone = timezone * (-60);

  // load time from LittleFS
  DynamicJsonDocument time_json = load_json("/time.json");

  // update only timezon when not in AP mode
  if(AP_mode == false) {
    time_json["timezone"] = timezone;
    time_json.shrinkToFit();
  }
  else {
    // update time and timezone when in AP mode
    time_json["hours"] = time_only.substring(0, time_only.indexOf(":")).toInt();
    time_json["minutes"] = time_only.substring(time_only.indexOf(":") + 1, time_only.lastIndexOf(":")).toInt();
    time_json["seconds"] = time_only.substring(time_only.lastIndexOf(":") + 1).toInt();
    time_json["weekday"] = weekday;
    time_json["timezone"] = timezone;
    time_json["init_offset"] = millis();
    time_json["last_offset"] = millis();
    time_json.shrinkToFit();
  }
  

  // save updated time to LittleFS
  save_json("/time.json", time_json);
  return;
}


String get_current_time(){
  /*
  parameters:
    ---

  returns:
    String:
      current time in format "hh:mm:ss weekday"
  
  description:
  This function loads the time from the LittleFS, adds the relative offset between the offset 
  of initialization and the current offset and returns the current time.

  calls:
  - load_json (in src/filesystem.cpp) to load the time from the LittleFS
  - turn_seconds_in_time (in src/time_management.cpp) to convert the millis() offset to a time format
  - add_time (in src/time_management.cpp) to add the offset time to the time from the LittleFS

  called by:
  - compare_time (in src/time_management.cpp) used to check if the current time is equal to the time in the program
  */

  // load time from LittleFS
  DynamicJsonDocument time_json = load_json("/time.json");

  // convert json to string
  String time =  time_json["hours"].as<String>();
  time += ":";
  time += time_json["minutes"].as<String>();
  time += ":";
  time += time_json["seconds"].as<String>();
  time += " ";
  time += time_json["weekday"].as<String>();

  // check initial offset
  unsigned long init_offset = time_json["init_offset"].as<unsigned long>();

  // calculate effective offset in seconds (time since time initialization)
  unsigned long effective_offset = millis() - init_offset;
  unsigned long effective_offset_seconds = effective_offset / 1000;

  // add offset to time
  String offset_time = turn_seconds_in_time(effective_offset_seconds);
  time = add_time(time, offset_time);

  // return current time
  return time;
}


String turn_seconds_in_time(unsigned long input_seconds){
  /*
  parameters:
    unsigned long input_seconds:
      seconds to convert
  
  returns:
    String:
      time in format "hh:mm:ss"
  
  description:
  This function converts seconds to time format. It is used in get_current_time() to prepare 
  millis() offset for comparison with saved time.

  calls:
    ---

  called by:
  - get_current_time (in src/time_management.cpp) used to convert relative offset to a time format
  - check_and_update_offset (in src/time_management.cpp) used to adjust time after millis() overflow
  */

  // variables
  int hours = input_seconds / 3600;
  int minutes = (input_seconds % 3600) / 60;
  int seconds = input_seconds % 60;

  String time = "";

  // add leading zeros and build string
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


String add_time(String time, String offset_time){
  /*
  parameters:
    String time:
      time to add to in format "hh:mm:ss weekday"
    String offset_time:
      time to add in format "hh:mm:ss"

  returns:
    String:
      time in format "hh:mm:ss weekday"

  description:
  This function adds two times together. The order of the parameters is important. 
  The first parameter contains the weekday, the second parameter does not.

  calls:
    ---

  called by:
  - get_current_time (in src/time_management.cpp) used to add the offset to the saved time
  - check_and_update_offset (in src/time_management.cpp) used to adjust time after millis() overflow
  */

  // split time in ints
  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();
  int weekday = time.substring(time.indexOf(" ") + 1).toInt();

  int offset_hours = offset_time.substring(0, offset_time.indexOf(":")).toInt();
  int offset_minutes = offset_time.substring(offset_time.indexOf(":") + 1, offset_time.indexOf(":") + 3).toInt();
  int offset_seconds = offset_time.substring(offset_time.indexOf(":") + 4, offset_time.indexOf(":") + 6).toInt();

  // sum up seconds and add overflow to minutes
  seconds += offset_seconds;
  if (seconds >= 60){
    seconds -= 60;
    minutes += 1;
  }

  // sum up minutes and add overflow to hours
  minutes += offset_minutes;
  if (minutes >= 60){
    minutes -= 60;
    hours += 1;
  }

  // sum up hours and add overflow to weekday (multiple overflows are possible)
  hours += offset_hours;
  while (hours >= 24){
    hours -= 24;
    weekday += 1;
  }

  // adjust weekday
  while (weekday >= 7){
    weekday -= 7;
  }

  // build string with leading zeros
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

  // return sum
  return (time_sum);
}


DynamicJsonDocument get_NTP_time(int timezone){
  /*
  parameters:
    int timezone:
      timezone offset in seconds
  
  returns:
    DynamicJsonDocument:
      time in format: 
        {
          "hours": hh,
          "minutes": mm, 
          "seconds": ss, 
          "weekday": w, 
          "init_offset": millis(),
          "timezone": ssss,
          "last_offset": millis()
        }

  description:
  This function gets the time from the web and returns it as a DynamicJsonDocument.
  It is only used in the initial setup to get the time without user interaction.

  calls:
    ---

  called by:
  - init_time (in src/time_management.cpp) to implement time retrieval in the initial setup
  */

  // initialize NTP client
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone);
  timeClient.begin();

  //check if server is available
  if (!timeClient.update()){
    timeClient.end();
    //return empthy json
    DynamicJsonDocument time_json(1024);
    time_json.shrinkToFit();
    return time_json;
  }

  // get time and weekday
  String time = timeClient.getFormattedTime();
  int weekday = timeClient.getDay();
  timeClient.end();

  // build json
  DynamicJsonDocument time_json(1024);

  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();

  time_json["hours"] = hours;
  time_json["minutes"] = minutes;
  time_json["seconds"] = seconds;
  time_json["weekday"] = weekday;
  time_json["init_offset"] = millis();
  time_json["timezone"] = timezone;

  time_json["last_offset"] = millis();
  time_json.shrinkToFit();

  // return json
  return(time_json);
}


void init_time(){
  /*
  parameters:
    ---
  
  returns:
    boolean:
      true: time was saved before
      false: time was not saved before

  description:
  Initializes the time after connecting to the wifi.

  calls:
  - load_json (in src/filesystem.cpp) to load the time from time.json
  - get_NTP_time (in src/time_management.cpp) to get the time from the web 
    (serves timezone if given in time.json)
  - save_json (in src/filesystem.cpp) to save the time to time.json

  called by:
  - setup (in src/main.cpp) to initialize the time after WifiManager setup
  */

  // load time from time.json
  DynamicJsonDocument current_values = load_json("/time.json");

  // no time was saved yet (timezone is set to 0)
  if (current_values.isNull()){
    save_json("/time.json", get_NTP_time(0));
    return;
  }

  // if there is a time saved: update it according to the timezone and overwrite offsets with current offset
  else {
    Serial.println("init timezone: " + current_values["timezone"].as<String>());
    save_json("/time.json", get_NTP_time(current_values["timezone"]));
    return;
  }
}


void check_and_update_offset() {
  /*
  parameters:
    ---

  returns:
    void

  description:
  Since millis() overflows after 49.7 days, this function checks if an overflow occured and updates 
  the time saved in "time.json" every time it happens. This function is used whenever long waiting times
  are expected (e.g. in timed programs or wait/skip command). It is important to note that this function
  has to be able to work offline (no NTP call) since it should be possible to run programs without internet
  connection.

  calls:
  - load_json (in src/filesystem.cpp) to load the time from time.json
  - save_json (in src/filesystem.cpp) to either only update the last offset value if no overflow occured
    or to additionally reinitialize the time with its updated init_offset if an overflow occured
  - turn_seconds_in_time (in src/time_management.cpp) to convert the overflow time to time format
  - add_time (in src/time_management.cpp) to first add the maximum time to the current time and then
    add the overflow time to the result
  
  called by:
  - compare_time (in src/time_management.cpp) and therefore indirectly called by timed programs
  - handle_wait_command (in src/workflows.cpp) to check for overflow in wait and skip commands
  */

  // load time from time.json
  DynamicJsonDocument current_values = load_json("/time.json");

  // get offsets
  unsigned long last_offset = current_values["last_offset"];
  unsigned long current_offset = millis();

  // if no overflow occured update last_offset
  if (current_offset > last_offset){
    current_values["last_offset"] = current_offset;
    save_json("/time.json", current_values);
    return;
  }

  // if overflow occured update time
  else if (current_offset < last_offset){
    
    // calculate time until overflow
    unsigned long overflow_seconds = 4294967;
    unsigned long init_offset = current_values["init_offset"];
    unsigned long init_offset_seconds = init_offset/1000;
    overflow_seconds = overflow_seconds - init_offset_seconds;
    String overflow_time = turn_seconds_in_time(overflow_seconds);

    // turn initial time into string
    DynamicJsonDocument init_values = load_json("/time.json");

    String init_time = init_values["hours"].as<String>();
    init_time += ":";
    init_time += init_values["minutes"].as<String>();
    init_time += ":";
    init_time += init_values["seconds"].as<String>();
    init_time += " ";
    init_time += init_values["weekday"].as<String>();

    // add overflow time to initial time (result is time at moment of overflow)
    String current_time = add_time(init_time, overflow_time);

    // calculate time since overflow
    unsigned long current_offset = millis();
    unsigned long current_offset_seconds = current_offset/1000;
    String overflowed_time = turn_seconds_in_time(current_offset_seconds);

    // add overflowed time to time at moment of overflow (result is current time)
    current_time = add_time(current_time, overflowed_time);

    // write current time to json
    current_values["hours"] = current_time.substring(0, current_time.indexOf(":")).toInt();
    current_values["minutes"] = current_time.substring(current_time.indexOf(":") + 1, current_time.indexOf(":") + 3).toInt();
    current_values["seconds"] = current_time.substring(current_time.indexOf(":") + 4, current_time.indexOf(":") + 6).toInt();
    current_values["weekday"] = current_time.substring(current_time.indexOf(" ") + 1).toInt();
    current_values["last_offset"] = current_offset;
    current_values["init_offset"] = current_offset;
    current_values["timezone"] = current_values["timezone"];

    // save json
    save_json("/time.json", current_values);
    return;
  }
  
  // no change in time occured (called too quickly after last call)
  else {
    return;
  }
}