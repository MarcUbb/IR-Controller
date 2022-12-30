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
void handle_form();

// declared outside of setup() to make visible to handler functions
ESP8266WebServer server(80);

// global variables (used to hand over information about current state of website or state of function execution)
// programname is used to hand over information about currently selected program
// on /program, handleProgram() will access this variable and send program code and name to the frontend
String PROGRAMNAME = "";
// message is used to hand over error messages (is set by website on /form) and updated in frontend on reload
String MESSAGE = "";
