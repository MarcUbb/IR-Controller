#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// forward declarations
// filesystem
String capture_signal();
String save_signal(String result_string, String name);
void save_json(String filename, DynamicJsonDocument doc);
DynamicJsonDocument load_json(String filename);
String send_signal(DynamicJsonDocument doc);
String get_files(String folder_signals, String folder_programs);
boolean check_if_file_exists(String filename);
String read_program(String program_name);
void control_led_output(String signal);
boolean check_if_string_is_alphanumeric (String word);

// time_management
String weekday_to_num (String weekday);
boolean compare_time (String time, boolean weekday_included);
void update_time(String time, boolean AP_mode);
String get_current_time();
String turn_seconds_in_time(unsigned long input_seconds);
String add_time(String time, String offset_time);
DynamicJsonDocument get_NTP_time(int timezone);
void init_time();
void check_and_update_offset();