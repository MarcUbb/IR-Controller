#include "base.h"



// converts a weekday String to a number
// function is used to compare the weekday of the program with the current weekday and the conversion is necessary because the user
// enters the weekday as a String
String weekday_to_num (String weekday){
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


// elementary function of timed programs as it compares the current time with the time in the program
boolean compare_time (String time, boolean weekday_included) {

  // checks if millis() overflowed and updates time if necessary
  check_and_update_offset();

  String current_time = get_current_time();
  delay(500);
  Serial.println("Current time: " + current_time);

  if (weekday_included == true) {
    return(time == current_time);
  }
  else {
    return(time == current_time.substring(0, current_time.indexOf(" ")));
  }
}


// Saves time from website to LittleFS. Offset is not generated because it would create greater error
void update_timezone(int timezone){

  timezone = timezone * (-60);

  DynamicJsonDocument time_json = load_json("/time.json");

  time_json["timezone"] = timezone;
  time_json.shrinkToFit();
  save_json("/time.json", time_json);
  return;
}


// loads the time from the LittleFS, adds the current offset and returns the current time
String get_current_time(){
  DynamicJsonDocument time_json = load_json("/time.json");

  String time =  time_json["hours"].as<String>();
  time += ":";
  time += time_json["minutes"].as<String>();
  time += ":";
  time += time_json["seconds"].as<String>();
  time += " ";
  time += time_json["weekday"].as<String>();

  
  unsigned long init_offset = time_json["init_offset"].as<unsigned long>();


  unsigned long effective_offset = millis() - init_offset;
  unsigned long effective_offset_seconds = effective_offset / 1000;

  String offset_time = turn_seconds_in_time(effective_offset_seconds);
  time = add_time(time, offset_time);

  return time;
}

// converts seconds to time format. Is used in get_current_time() to prepare millis() offset for comparison with saved time
String turn_seconds_in_time(unsigned long input_seconds){
  int hours = input_seconds / 3600;
  int minutes = (input_seconds % 3600) / 60;
  int seconds = input_seconds % 60;

  String time = "";
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


// adds two times together. Is used in get_current_time() to add the offset to the saved time
String add_time(String time, String offset_time){
  int hours = time.substring(0, time.indexOf(":")).toInt();
  int minutes = time.substring(time.indexOf(":") + 1, time.indexOf(":") + 3).toInt();
  int seconds = time.substring(time.indexOf(":") + 4, time.indexOf(":") + 6).toInt();
  int weekday = time.substring(time.indexOf(" ") + 1).toInt();

  int offset_hours = offset_time.substring(0, offset_time.indexOf(":")).toInt();
  int offset_minutes = offset_time.substring(offset_time.indexOf(":") + 1, offset_time.indexOf(":") + 3).toInt();
  int offset_seconds = offset_time.substring(offset_time.indexOf(":") + 4, offset_time.indexOf(":") + 6).toInt();

  seconds += offset_seconds;
  if (seconds >= 60){
    seconds -= 60;
    minutes += 1;
  }
  minutes += offset_minutes;
  if (minutes >= 60){
    minutes -= 60;
    hours += 1;
  }
  hours += offset_hours;

  while (hours >= 24){
    hours -= 24;
    weekday += 1;
  }
  while (weekday >= 7){
    weekday -= 7;
  }

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
  return (time_sum);
}


// only used in intital setup to get the time from the web without user interaction
DynamicJsonDocument get_NTP_time(int timezone){
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", timezone);
  timeClient.begin();
  timeClient.update();
  String time = timeClient.getFormattedTime();
  int weekday = timeClient.getDay();
  timeClient.end();

  Serial.println("Init time: " + time + " " + weekday);

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

  return(time_json);
}


// initializes the time after connecting to the wifi
boolean init_time(){

  // loads the time from time.json
  DynamicJsonDocument current_values = load_json("/time.json");

  // no time was saved yet (first boot)
  if (current_values.isNull()){
    save_json("/time.json", get_NTP_time(0));
    return false;
  }

  // if there is a time saved: update it according to the timezone and overwrite offsets with current offset
  else {
    Serial.println("init timezone: " + current_values["timezone"].as<String>());
    save_json("/time.json", get_NTP_time(current_values["timezone"]));
    return true;
  }
}


// checks if overflow occured and updates time if necessary
// generates current_offset and compares to last offset
// if current_offset < last_offset: overflow occured and last_offset is resetted
void check_and_update_offset() {
  DynamicJsonDocument current_values = load_json("/time.json");
  unsigned long last_offset = current_values["last_offset"];
  unsigned long current_offset = millis();

  // no overflow occured:
  if (current_offset > last_offset){
    current_values["last_offset"] = current_offset;
    save_json("/time.json", current_values);
    return;
  }
  // overflow occured:
  else if (current_offset < last_offset){
    unsigned long overflow_seconds = 4294967;
    String overflow_time = turn_seconds_in_time(overflow_seconds);

    DynamicJsonDocument init_values = load_json("/time.json");

    String init_time = init_values["hours"].as<String>();
    init_time += ":";
    init_time += init_values["minutes"].as<String>();
    init_time += ":";
    init_time += init_values["seconds"].as<String>();
    init_time += " ";
    init_time += init_values["weekday"].as<String>();

    String current_time = add_time(init_time, overflow_time);

    unsigned long current_offset = millis();
    unsigned long current_offset_seconds = current_offset/1000;
    String overflowed_time = turn_seconds_in_time(current_offset_seconds);
    current_time = add_time(current_time, overflowed_time);

    current_values["hours"] = current_time.substring(0, current_time.indexOf(":")).toInt();
    current_values["minutes"] = current_time.substring(current_time.indexOf(":") + 1, current_time.indexOf(":") + 3).toInt();
    current_values["seconds"] = current_time.substring(current_time.indexOf(":") + 4, current_time.indexOf(":") + 6).toInt();
    current_values["weekday"] = current_time.substring(current_time.indexOf(" ") + 1).toInt();
    current_values["last_offset"] = current_offset;
    current_values["init_offset"] = current_offset;
    current_values["timezone"] = current_values["timezone"];

    save_json("/time.json", current_values);
    return;
  }
  // no change occured:
  else {
    return;
  }
}