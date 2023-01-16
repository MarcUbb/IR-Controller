#include "main.h"

/*
In this file, you find the main procedures of the program.
*/

void setup() {
  Serial.begin(115200);
  delay(500);
  //test();

  // read config file (if ESP is in AP mode or not)

  // start LittleFS
  LittleFS.begin();

  // read config file
  File configFile = LittleFS.open("/config.txt", "r");

  // get content of file
  String content = configFile.readString();
  configFile.close();

  // end LittleFS
  LittleFS.end();

  // start AP
  if (content == "AP: true") {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("IR-Remote");
    Serial.println("AP started");

    // set session variable
    SESSION_AP = "true";

    // notify user to synchronize time (not automatic since no NTP server is available)
    MESSAGE = "Device in AP-Mode. Please synchronize time before using timed Programs!";
  }
  // start WiFiManager if AP is not set
  else {
    // WiFiManager (is skipped if credentials are saved)
    WiFiManager wifiManager;
    wifiManager.autoConnect("IR-Remote");

    // set session variable
    SESSION_AP = "false";
    
    MESSAGE = "Please synchronize timezone if not done yet!";
  }

  // start mDNS
  if (MDNS.begin("irr") == false) {
    Serial.println("Error setting up MDNS responder!");
    control_led_output("no_mDNS");
    // stalls program execution if mDNS fails
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started!");

  // initiate time via NTP (00:00:00 4 in AP Mode)
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
  server.onNotFound(handle_not_found);

  // start server
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  MDNS.update();
  server.handleClient();
}







//------------------ handlers ------------------//
 
void handle_root() {
  /*
  serves index.html (from include/website_string.h)
  */
  server.send(200, "text/html", index_html);
}

void handle_not_found() {
  /*
  serves 404 error
  */
  server.send(404, "text/plain", "404: Not found");
}

void handle_program() {
  /*
  The website is designed with form elements that trigger forwarding to /form. 
  This is a problem because we can only communicate on that channel and we already 
  have to communicate a back to the website to update the page. 
  So in order to still be able to send data back to the website, on every reload the 
  website will send a get request on /program which triggers this function. 
  The data that is sent back is the name of the currently selected program and the code of that program.
  It is only sent back if the "edit" button was pressed. (if not, the variables "PROGRAMNAME" 
  and "PROGRAMCODE" will be empty)
  */

  // read code of selected program
  String PROGRAMCODE = read_program(PROGRAMNAME);

  // send name and code of selected program
  server.send(200, "text/plain", PROGRAMNAME + ";" + PROGRAMCODE);
}

void handle_error() {
  /*
  Similarly to handle_program(), the website will send a get request on /error to 
  update the error message on reload. The MESSAGE is then written again by handle_form().
  */
  server.send(200, "text/plain", MESSAGE);
}

void handle_files() {
  /*
  Sends a list of all files in /signals and /programs on reload to be displayed on the website.
  */

  // generate list of files in /signals and /programs
  String files = get_files("/signals", "/programs");

  //Serial.println(files);

  // send list of files
  server.send(200, "text/plane", files);
}


// TODO: split server.arg in time and timezone and call different function with each (AP-Mode on/off)
void handle_time() {
  /*
  Gets the time from the client via Date() and saves it together with the millis() offset 
  to the LittleFS.
  The offset is important because only with millis() we can calculate the time that has passed 
  between the synchronisation and the time of program execution.
  This enabled the ESP to execute timed programs even if the wifi connection is lost.
  */

  // get time from client (saved in hidden dummy text input field on website)
  String timezone = server.arg("time_dummy");

  /*
  if(SESSION_AP == "true") {
    // update time and timezone
    update_time(timestamp.toInt());
  }
  else {
    // update only timezone because NTP time is more precise
    update_timezone(timezone.toInt());
  }*/

  // update only timezone because NTP time is more precise
  update_timezone(timezone.toInt());

  // update MESSAGE
  MESSAGE = "Time synchronized!";

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

void handle_credentials() {
  /*
  erases the wifi credentials saved on the ESP
  */

  ESP.eraseConfig();

  MESSAGE = "Credentials erased! You will need to reconnect to the Wifi on reboot (if you are not in AP-Mode).";

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

void handle_apmode() {
  /*
  checks config file and switches between AP mode and normal mode
  */

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
  if(config == "AP: true") {
    // write new config
    File configFile = LittleFS.open("/config.txt", "w");
    if (!configFile) {
      MESSAGE = "Failed to open config file";
    }
    configFile.print("AP: false");
    configFile.close();

    MESSAGE = "AP mode disabled! You will need to reconfigure the ESP the your Wifi on reboot if no credentials are saved already.";
  }
  else {
    // write new config
    File configFile = LittleFS.open("/config.txt", "w");
    if (!configFile) {
      MESSAGE = "Failed to open config file";
    }
    configFile.print("AP: true");
    configFile.close();

    MESSAGE = "AP mode enabled! Dont forget to synchronize your time on next reboot.";
  }

  // redirect to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

void handle_form() {
  /*
  Handles all form elements on the website (signals and programs) also updates 
  the error message and PROGRAMNAME.
  All the user interaction with the website is handled here and the functions 
  from workflows.h are called.
  */

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

  // edit button is not pressed and therefor PROGRAMNAME is reset
  if (edit_program_button == "") {
    PROGRAMNAME = "";
  }
  
  // sends user back to root
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}