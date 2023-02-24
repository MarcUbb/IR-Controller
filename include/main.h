/**
 * @file main.h
 * @author Marc Ubbelohde
 * @brief Header File for main.cpp
 * 
 * @details This file includes dependencies for the wifi setup and webserver.
 * Also the HTML code for the website is included and the variables that are 
 * shared between the handler functions are declared.
 */

#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "workflows.h"
#include "website_string.h"

#include "tests.h"

// forward declarations
void handle_root();
void handle_not_found();
void handle_program();
void handle_error();
void handle_files();
void handle_time();
void handle_credentials();
void handle_apmode();
void handle_apinfo();
void handle_password();
void handle_form();

// global variables
/**
 * @brief Constructor of the webserver that is used to serve the website.
 * 
 */
ESP8266WebServer server(80);

/**
 * @brief Holds the name of the currently selected program if the edit button was pressed.
 * Gets updated on /form and called on /program. 
 * 
 */
String PROGRAMNAME = "";

/**
 * @brief Holds the message that is displayed on the website and updated on reload.
 * 
 */
String MESSAGE = "";

/**
 * @brief Stores the session value (if AP-Mode is activated or not).
 * 
 */
boolean SESSION_AP = true;

/**
 * @brief Tracks which setting to be used on reboot.
 * 
 */
boolean AP_SETTING = true;