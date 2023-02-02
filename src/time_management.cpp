/**
 * @file time_management.cpp
 * @author Marc Ubbdelohde
 * @brief This file contains the functions to manage the time.
 * 
 * @details The time functions are exlusively used in the timed programs. The complexity of some of the functions
 * is due to the fact that the device does not use an external RTC and that the millis() function overflows after about 49 days.
 * The different functions utilize functions from the filesystem.cpp file to load and save time information to the LittleFS.
 * They provide functionality to each other and the higher level functions in workflows.cpp and main.cpp where fronend functionalities
 * are implemented.
 */

#include "base.h"



/**
 * @brief Converts a weekday String to a weekday number
 * 
 * @param weekday - weekday as a String
 * @return String - weekday as a number:\n
 *                  "Monday" - "1"\n
 *                  "Tuesday" - "2"\n
 *                  "Wednesday" - "3"\n
 *                  "Thursday" - "4"\n
 *                  "Friday" - "5"\n
 *                  "Saturday" - "6"\n
 *                  "Sunday" - "0"\n
 *                  error - "error"
 * 
 * @details This function converts a weekday String to a weekday number.
 * 
 * @callgraph This function does not call any function.
 * 
 * @callergraph
 * 
 */
String weekday_to_num (String weekday){

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

/**
 * @brief Compare specified time with current time
 * 
 * @param time - String in format "weekday hh:mm:ss timezone"
 * 
 * @param weekday_included - true if weekday is included in time, false if not
 * 
 * @return boolean - true if time is equal to current time, false if not
 * 
 * @details This elementary function checks if the current time is equal to the time in the program.
 * It is used in timed programs and handles millis() overflow. The function has a delay of 500ms
 * to reduce the number of operations inside the while(true) loop.
 * 
 * @callgraph
 * 
 * @callergraph
 */
boolean compare_time (String time, boolean weekday_included) {
  
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

/**
 * @brief Updates the time in the LittleFS
 * 
 * @param time - String in format "weekday hh:mm:ss timezone"
 * 
 * @param AP_mode - true if the device is in AP mode, false if not
 * 
 * @details This function is called when the user presses the "sync" button on the website.
 * It updates only the timezone to the LittleFS since the time from the NTP request is more precise
 * than the time from the user.
 * 
 * @callgraph
 * 
 * @callergraph
 */
void update_time(String time, boolean AP_mode){
  
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

/**
 * @brief Loads the current time from LittleFS
 * 
 * @return String  - current time in format "hh:mm:ss weekday"
 * 
 * @details This function loads the time from the LittleFS, adds the relative offset between the offset
 * of initialization and the current offset and returns the current time.
 * 
 * @callgraph
 * 
 * @callergraph
 */
String get_current_time(){

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

/**
 * @brief This function converts seconds to time format.
 * 
 * @param input_seconds - seconds to convert
 * 
 * @return String - time in format "hh:mm:ss"
 * 
 * @details This function converts seconds to time format. It is used in get_current_time() to prepare
 * millis() offset for comparison with saved time.
 * 
 * @callgraph This function does not call other functions.
 * 
 * @callergraph
 */
String turn_seconds_in_time(unsigned long input_seconds){

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

/**
 * @brief adds two times together
 * 
 * @param time - time to add to in format "hh:mm:ss weekday"
 * 
 * @param offset_time - time to add in format "hh:mm:ss"
 * 
 * @return String - time in format "hh:mm:ss weekday"
 * 
 * @details This function adds two times together. The order of the parameters is important.
 * The first parameter contains the weekday, the second parameter does not.
 * 
 * @callgraph This function does not call other functions.
 * 
 * @callergraph
 */
String add_time(String time, String offset_time){
  
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

/**
 * @brief Gets time from NTP server
 * 
 * @details This function initiates the time by getting the time from the NTP server. 
 * It then saves it ot the LittlfeFS. It also passes the saved timezone to the NTP server
 * or 0 if no timezone is saved.
 * 
 * @callgraph
 * 
 * @callergraph
 */
void init_time(){

  // read timezone from LittleFS
  DynamicJsonDocument saved_time = load_json("/time.json");
  int timezone = saved_time["timezone"];

  Serial.println("Timezone: " + String(timezone));

  // initialize NTP client
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone);
  timeClient.begin();

  // try to retrieve time from NTP server for 5s
  unsigned long startTime = millis();

  while (!timeClient.update() && (millis() - startTime) < 5000) {
    delay(10);
  }

  // get time and weekday
  String time = timeClient.getFormattedTime();
  int weekday = timeClient.getDay();
  timeClient.end();

  Serial.println("Time: " + time + " " + weekday);

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
  time_json["last_offset"] = millis();
  time_json["timezone"] = timezone;
  time_json.shrinkToFit();

  // return json
  save_json("/time.json", time_json);
  return;
}



/**
 * @brief Checks if millis() overflowed and updates time if necessary
 * 
 * @details Since millis() overflows after 49.7 days, this function checks if an overflow occured and updates 
 * the time saved in "time.json" every time it happens to still be able to calculate teh current time. 
 * This function is used whenever long waiting times are expected (e.g. in timed programs, wait/skip 
 * command or user inactivity). It is important to note that this function has to be able to work offline 
 * (no NTP call) since it should be possible to run programs without internet connection.
 * 
 * @callgraph
 * 
 * @callergraph
 */
void check_and_update_offset() {

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