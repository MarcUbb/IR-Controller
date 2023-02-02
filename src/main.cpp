/**
 * @file main.cpp
 * 
 * @author Marc Ubbelohde
 * 
 * @brief Main file of the program.
 * 
 * @details In this file, you find the main procedures of the program and
 * the handler functions for the webserver.
 */


#include "main.h"

/**
 * @brief Arduino Setup function
 * 
 * @details This function is called once at the start of the program. It
 * checks the configuration file and start either the Access Point or
 * the WFiManager. It also starts the webserver and initializes the
 * time file. Optionally, unit tests can be run.
 * 
 * @callgraph
 * 
 * @callergraph This function is called by the Arduino framework.
 */
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(500);

  // optional: run tests (uncomment "include tests.h" in main.h before production)
  //run_all_tests(false);
  //run_all_empirical_tests(false);

  // start LittleFS
  LittleFS.begin();

  // read config file (if ESP is in AP mode or not)
  File configFile = LittleFS.open("/config.txt", "r");

  // get content of file
  String content = configFile.readString();
  configFile.close();

  // end LittleFS
  LittleFS.end();

  // AP is true
  if (content == "AP: true") {
    // read password file
    File passwordFile = LittleFS.open("/password.txt", "r");
    String password = passwordFile.readString();
    passwordFile.close();

    // set password to default if empty
    if(password == "") {
      password = "12345678";
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP("IR-Remote", password);
    Serial.println("AP started");

    // set session variable
    SESSION_AP = true;
    AP_SETTING = true;

    // notify user to synchronize time (not automatic since no NTP server is available)
    MESSAGE = "Device in AP-Mode. Please synchronize time before using timed Programs!";
    control_led_output("AP_on");
  }

  // AP is false (default)
  else {
    // try to connect to saved credentials
    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());

    unsigned long start = millis();
    while((WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) && millis() - start < 5000){
      delay(100);
    }

    // use WifiManager if connection fails
    if (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
      control_led_output("no_wifi");
      // WiFiManager (is skipped if credentials are saved)
      WiFiManager wifiManager;
      wifiManager.autoConnect("IR-Remote");
    }

    // set session variable
    SESSION_AP = false;
    AP_SETTING = false;
    
    MESSAGE = "Please synchronize timezone if not done yet!";
    control_led_output("AP_off");
  }

  // start mDNS
  if (MDNS.begin("irr") == false) {
	Serial.println("Error setting up MDNS responder!");
	control_led_output("no_mDNS");
	// stalls program execution if mDNS fails
	while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started!");

  // initiate time via NTP (00:00:20 4 if no internet connection)
  init_time();
  

  // declare handler functions
  server.on("/", handle_root);
  server.on("/form", handle_form);
  server.on("/files", handle_files);
  server.on("/program", handle_program);
  server.on("/error", handle_error);
  server.on("/time", handle_time);
  server.on("/credentials", handle_credentials);
  server.on("/apmode", handle_apmode);
  server.on("/apinfo", handle_apinfo);
	server.on("/password", handle_password);
  server.onNotFound(handle_not_found);

  // start server
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

/**
 * @brief Arduino Loop function
 * 
 * @details This function is called repeatedly. It updates the mDNS, 
 * handles clients and checks for a millis() overflow every 5 minutes.
 * 
 * @callgraph
 * 
 * @callergraph This function is called by the Arduino framework. 
 */
void loop() {
  MDNS.update();
  server.handleClient();
  if (millis() % 300000 == 0) {
    // update time every second
    check_and_update_offset();
    Serial.println("updating time");
  }
}







//------------------ handlers ------------------//

/**
 * @brief Handler function for the root page
 * 
 * @details This function is called when the root page is requested. It
 * serves the content of the index_html string from the website_string.h file to the client.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to the root page.
 */
void handle_root() {
  server.send(200, "text/html", index_html);
}

/**
 * @brief Handler function for any page that is not found.
 * 
 * @details This function is called when a page is requested that is not found. It
 * serves a 404 error to the client.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to a page that is not found. 
 */
void handle_not_found() {
  server.send(404, "text/plain", "404: Not found");
}

/**
 * @brief Handler function to display the program the user wants to edit.
 * 
 * @details This function is called when the user wants to edit a program. It
 * reads the program from the file system and sends it back to the website.
 * The website is designed with form elements that trigger forwarding to /form.
 * This is a problem because we can only communicate on that channel and we already
 * have to communicate a back to the website to update the page.
 * So in order to still be able to send data back to the website, on every reload the
 * website will send a get request on /program which triggers this function.
 * The data that is sent back is the name of the currently selected program and the code of that program.
 * It is only sent back if the "edit" button was pressed. (if not, the variables "PROGRAMNAME"
 * and "PROGRAMCODE" will be empty)
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /program.
 */
void handle_program() {

  // read code of selected program
  String PROGRAMCODE = read_program(PROGRAMNAME);

  if (PROGRAMCODE.indexOf("Error") != -1) {
    MESSAGE = PROGRAMCODE;
  }

  // send name and code of selected program
  server.send(200, "text/plain", PROGRAMNAME + ";" + PROGRAMCODE);

  PROGRAMNAME = "";
}

/**
 * @brief Handler function to display the error message.
 * 
 * @details Similarly to handle_program(), the website will send a get request on /error to
 * update the error message on reload. The MESSAGE is then written again by handle_form().
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /error.
 */
void handle_error() {
  server.send(200, "text/plain", MESSAGE);
}

/**
 * @brief Handler function to send a list of all signals and programs to the forntend.
 * 
 * @details Sends a list of all files in /signals and /programs on reload to be displayed on the website.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /files.
 * 
 */
void handle_files() {

  // generate list of files in /signals and /programs
  String files = get_files("/signals", "/programs");

  //Serial.println(files);

  // send list of files
  server.send(200, "text/plane", files);
}

/**
 * @brief Handler funciton to synchronize the time and/or timezone provided by the user.
 * 
 * @details Gets the time from the client via Date() and saves it together with the millis() offset
 * to the LittleFS. The offset is important because only with millis() we can calculate the time that has passed
 * between the synchronisation and the time of program execution. This enabled the ESP to execute timed programs
 * even if the wifi connection is lost.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /time.
 * 
 */
void handle_time() {

  // get time from client (saved in hidden dummy text input field on website)
  // time in format: "weekday hh:mm:ss timezone"
  String time = server.arg("weekday_time");

  Serial.println(time);

  // update time (update only timezone when not in AP-mode because NTP time is more precise)
  update_time(time, SESSION_AP);  

  // update MESSAGE
  MESSAGE = "Time synchronized!";

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

/**
 * @brief Handler function to erase the wifi credentials saved on the ESP.
 * 
 * @details Erases the wifi credentials saved on the ESP. This is useful if the user wants to connect to a different
 * wifi network.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /credentials.
 *  
 */
void handle_credentials() {

  ESP.eraseConfig();

  MESSAGE = "Credentials erased! You will need to reconnect to the Wifi on reboot (if you are not in AP-Mode).";

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

/**
 * @brief Handler function to switch between AP mode and normal mode.
 * 
 * @details Checks the config file and switches between AP mode and normal mode.
 * The possibel reconfiguration of the wifi credentails is done after restart.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /apmode.
 */
void handle_apmode() {

  // start LittleFS
  LittleFS.begin();

  // open config file
  File configFile = LittleFS.open("/config.txt", "r");
  if (!configFile) {
	MESSAGE = "Failed to open config file";
  }

  // read config file
  String config = configFile.readString();

  // close config file
  configFile.close();

  // switch value:

  if (config == "AP: false"){
    // write new config
    File configFile = LittleFS.open("/config.txt", "w");
    if (!configFile) {
      MESSAGE = "Failed to open config file";
    }
    configFile.print("AP: true");
    configFile.close();

    // set variable for frontend
    AP_SETTING = true;
    MESSAGE = "AP mode enabled! Dont forget to synchronize your time on next reboot.";
  }

  // includes case when no config exists
  else {
    // update config
    File configFile = LittleFS.open("/config.txt", "w");
    if (!configFile) {
      MESSAGE = "Failed to open config file";
    }
    configFile.print("AP: false");
    configFile.close();

    // set variable for frontend
    AP_SETTING = false;
    MESSAGE = "AP mode disabled! You will need to reconfigure the ESP the your Wifi on reboot if no credentials are saved already.";
  }

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}


/**
 * @brief Handler function to send the current AP mode setting to the frontend.
 * 
 * @details Sends the current AP mode setting to the frontend. 
 * This is used to display the correct setting at the top the website.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /apinfo.
 * 
 */
void handle_apinfo() {

  if(AP_SETTING == true) {
		server.send(200, "text/plain", "true");
  }
  else if(AP_SETTING == false) {
		server.send(200, "text/plain", "false");
  }
	else {
		MESSAGE = "undefined AP-mode";
	}
}

/**
 * @brief Handler function that receives new password entries.
 * 
 * @details Receives new password entries from frontend and changes password
 * if entries are the same (to prevent typos).
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /password.
 * 
 */
void handle_password() {

	// get data
	String first_entry = server.arg("first_entry");
	String second_entry = server.arg("second_entry");

  // check if password is long enough
  if(first_entry.length() < 8) {
    MESSAGE = "Password must be at least 8 characters long!";
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Updated– Press Back Button");
    return;
  }

  // check if entries are the same
  if(first_entry == second_entry) {
    // write new password to password file
    LittleFS.begin();

    File passwordFile = LittleFS.open("/password.txt", "w");

    // handle error
    if (!passwordFile) {
      LittleFS.end();
      MESSAGE = "Failed to open password file";
    }

    else {
      // write new password
      passwordFile.print(first_entry);
      passwordFile.close();

      MESSAGE = "Password changed successfully!"; 
      LittleFS.end();
    }
  }

  else {
    MESSAGE = "Passwords do not match!"; 
  }
	
  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

/**
 * @brief Handler function that receives GET requests from all form elements
 * related to signals and programs.
 * 
 * @details Handles all form elements on the website (signals and programs) also
 * updates the error message and PROGRAMNAME. All the user interaction with the
 * website is handled here and the functions from workflows.h are called.
 * 
 * @callgraph
 * 
 * @callergraph This function is called on a GET request to /form.
 * 
 */
void handle_form() {

  String selected_signal = server.arg("selected_signal");
  String send_signal_button = server.arg("send_signal_button"); 
  String delete_signal_button = server.arg("delete_signal_button"); 
  String signal_name = server.arg("signal_name"); 
  String add_signal_button = server.arg("add_signal_button");
  String selected_program = server.arg("selected_program");
  String play_program_button = server.arg("play_program_button"); 
  String program_code = server.arg("program_code");
  String program_name = server.arg("program_name"); 
  String add_program_button = server.arg("add_program_button");
  String delete_programs_button = server.arg("delete_program_button");
  String edit_program_button = server.arg("edit_program_button");

  /*
  Serial.println("signal: " + signal);
  Serial.println("send_signal_button: " + send_signal_button);
  Serial.println("delete_signal_button: " + delete_signal_button);
  Serial.println("signal_name: " + signal_name);
  Serial.println("add_signal_button: " + add_signal_button);
  Serial.println("program: " + program);
  Serial.println("play_program_button: " + play_program_button);
  Serial.println("program_code: " + program_code);
  Serial.println("program_name: " + program_name);
  Serial.println("add_program_button: " + add_program_button);
  Serial.println("delete_programs_button: " + delete_programs_button);
  Serial.println("edit_program_button: " + edit_program_button);
  */

  // SIGNAL LOGIC:
  // signal is added
  if (signal_name != "" && add_signal_button != "") {
	// check if signal name is alphanumeric
	if (check_if_string_is_alphanumeric (signal_name) == true) {
	  MESSAGE = recording_workflow(signal_name);
	}
	else {
	  MESSAGE = "signal name must be alphanumeric";
	}
  }

  // signal name is missing
  else if (signal_name == "" && add_signal_button != "") {
	MESSAGE = "no signal name given";
  }

  // signal is selcted
  else if (selected_signal != "") {

	// send button is pressed
	if (send_signal_button != "") {
	  MESSAGE = sending_workflow(selected_signal);
	}

	// delete button is pressed
	else if (delete_signal_button != "") {
	  MESSAGE = deleting_workflow("signals", selected_signal);
	}
  }

  // no signal is selected
  else if (selected_signal == "" && send_signal_button != "") {
	MESSAGE = "no signal selected";
  }

  // no signal is selected
  else if (selected_signal == "" && delete_signal_button != "") {
	MESSAGE = "no signal selected";
  }

  // PROGRAM LOGIC:
  // program is added
  else if (program_name != "" && add_program_button != "" && program_code != "") {
	// check if program name is alphanumeric
	if (check_if_string_is_alphanumeric (program_name) == true) {
	  MESSAGE = adding_workflow(program_name, program_code);
	}
	else {
	  MESSAGE = "program name must be alphanumeric";
	}
  }

  // program name is missing
  else if (program_name == "" && add_program_button != "") {
	MESSAGE = "no program name given";
  }

  // program code is missing
  else if (program_code == "" && add_program_button != "") {
	MESSAGE = "no program code given";
  }

  // program is selected
  else if (selected_program != "") {

	// play button is pressed
	if (play_program_button != "") {
	  MESSAGE = playing_workflow(selected_program);
	}

	// delete button is pressed
	else if (delete_programs_button != "") {
	  MESSAGE = deleting_workflow("programs", selected_program);
	}

	// edit button is pressed
	else if (edit_program_button != "") {

	  // data is saved in global variable PROGRAMNAME and is used in handle_program() to display on website
	  PROGRAMNAME = selected_program;
	  MESSAGE = "successfully loaded program: " + selected_program;
	}
  }

  // no program is selected
  else if (selected_program == "" && play_program_button != "") {
	MESSAGE = "no program selected";
  }

  // no program is selected
  else if (selected_program == "" && delete_programs_button != "") {
	MESSAGE = "no program selected";
  }

  // no program is selected
  else if (selected_program == "" && edit_program_button != "") {
	MESSAGE = "no program selected";
  }
  
  // sends user back to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}