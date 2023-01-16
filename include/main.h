#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "workflows.h"
#include "website_string.h"

#include <test.h>

// forward declarations
void handle_root();
void handle_not_found();
void handle_program();
void handle_error();
void handle_files();
void handle_time();
void handle_credentials();
void handle_apmode();
void handle_form();

// server declared in header file to make visible to handler functions
ESP8266WebServer server(80);

// holds the name of the currently selected program if the edit button was pressed
// gets updated on /form and called on /program
String PROGRAMNAME = "";

// message is used to hand over error messages (is set by website on /form) and updated in frontend on reload
String MESSAGE = "";

// stores session value (if AP-Mode is activated or not)
String SESSION_AP = "";
